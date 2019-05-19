#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "office.h"
#include "../sope.h"
#include "operations.h"



void *office_main(void *args) {
    office_args_t *actual_args = args;

    /* The thread will keep looking for new requests, as long as the
       server is live or the request queue is not empty */
    while(true) {

        pthread_mutex_lock(actual_args->queue_lock);
        if (logSyncMech(actual_args->log_fd, actual_args->id, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_CONSUMER, 0) < 0) {
            printf("Error writing to logfile! \n");
        }

        while(is_empty(actual_args->queue) && !*actual_args->shutdown) {
            pthread_cond_wait(actual_args->empty_cond, actual_args->queue_lock);
            if (logSyncMech(actual_args->log_fd, actual_args->id, SYNC_OP_COND_WAIT, SYNC_ROLE_CONSUMER, 0) < 0) {
                printf("Error writing to logfile! \n");
            }
        }

        /* Exit condition for the thread */
        if (*actual_args->shutdown && is_empty(actual_args->queue)) {
            pthread_mutex_unlock(actual_args->queue_lock);   
            if (logSyncMech(actual_args->log_fd, actual_args->id, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_CONSUMER, 0) < 0) {
                printf("Error writing to logfile! \n");
            }
            break;
        }

        tlv_request_t request;

        /*
        printf("O %d - Locked the queue to get a request\n", actual_args->id);
        pthread_mutex_lock(actual_args->queue_lock);
        if (logSyncMech(actual_args->log_fd, actual_args->id, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_CONSUMER, request.value.header.pid) < 0) {
                printf("Error writing to logfile! \n");
        }
        */

        request = pop(actual_args->queue);
        pthread_cond_signal(actual_args->full_cond);
        if (logSyncMech(actual_args->log_fd, actual_args->id, SYNC_OP_COND_SIGNAL, SYNC_ROLE_CONSUMER, 0) < 0) {
            printf("Error writing to logfile! \n");
        }

        pthread_mutex_unlock(actual_args->queue_lock);
        if (logSyncMech(actual_args->log_fd, actual_args->id, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_CONSUMER, 0) < 0) {
            printf("Error writing to logfile! \n");
        }

        *actual_args->active_threads = *actual_args->active_threads + 1;

        logRequest(actual_args->log_fd, actual_args->id, &request);     // TODO: deal with errors

        tlv_reply_t reply;

        switch (request.type) {

            case OP_CREATE_ACCOUNT:

                reply.type = request.type;
                reply.length = 0;
                reply.value.header.account_id = request.value.header.account_id;


                /* check if the admin was the one to request the creation */
                if (request.value.header.account_id) {
                    reply.value.header.ret_code = RC_OP_NALLOW;
                }
                /* check if the password for the admin account is correct */
                else if (!authenticate(request.value.header.password, &actual_args->accounts[request.value.header.account_id])) {
                    reply.value.header.ret_code = RC_LOGIN_FAIL;
                }
                /* check if the account id is not used */
                else if (actual_args->accounts[request.value.create.account_id].account_id != ERROR_ACCOUNT_ID) {
                    reply.value.header.ret_code = RC_ID_IN_USE;
                }
                /* create the account */
                else {
                    pthread_mutex_init(&actual_args->account_mutex[request.value.create.account_id], NULL); // TODO: deal with errors

                    pthread_mutex_lock(&actual_args->account_mutex[request.value.create.account_id]);
                    if (logSyncMech(actual_args->log_fd, actual_args->id, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_ACCOUNT, request.value.header.pid) < 0) {
                        printf("Error writing to logfile! \n");
                    }

                    usleep(request.value.header.op_delay_ms * 1000);           // TODO: deal with errors
                    if (logSyncDelay(actual_args->log_fd, actual_args->id, request.value.header.account_id, request.value.header.op_delay_ms * 1000) < 0) {
                        printf("Error writing to logfile! \n");
                    }

                    bank_account_t new_account;
                    new_account.account_id = request.value.create.account_id;
                    new_account.balance =  request.value.create.balance;
                    generateSalt(&new_account);
                    char hash[HASH_LEN + 1];
                    if (generateHash(request.value.create.password, &new_account, hash)) {
                        printf("Error generating account hash! \n");
                    }
                    strcpy(new_account.hash, hash);

                    actual_args->accounts[new_account.account_id] = new_account;

                    reply.value.header.ret_code = RC_OK;

                    logAccountCreation(actual_args->log_fd, actual_args->id, &new_account);

                    pthread_mutex_unlock(&actual_args->account_mutex[request.value.create.account_id]);
                    if (logSyncMech(actual_args->log_fd, actual_args->id, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_ACCOUNT, request.value.header.pid) < 0) {
                        printf("Error writing to logfile! \n");
                    }
                }

                break;
            
            case OP_BALANCE:

                reply.type = request.type;
                reply.length = sizeof(rep_balance_t);
                reply.value.header.account_id = request.value.header.account_id;
                reply.value.balance.balance = 0;
                
                /* check if the account id is valid */
                if (actual_args->accounts[request.value.header.account_id].account_id == ERROR_ACCOUNT_ID) {
                    reply.value.header.ret_code = RC_ID_NOT_FOUND;
                }
                /* check if it is not the admin account */
                else if (request.value.header.account_id == ADMIN_ACCOUNT_ID) {
                    reply.value.header.ret_code = RC_OP_NALLOW;
                }
                /* check if the password is correct */
                else if (!authenticate(request.value.header.password, &actual_args->accounts[request.value.header.account_id])) {
                    reply.value.header.ret_code = RC_LOGIN_FAIL;
                }
                /* get account balance */
                else {
                    pthread_mutex_lock(&actual_args->account_mutex[request.value.header.account_id]);
                    if (logSyncMech(actual_args->log_fd, actual_args->id, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_ACCOUNT, request.value.header.pid) < 0) {
                        printf("Error writing to logfile! \n");
                    }

                    usleep(request.value.header.op_delay_ms * 1000);           // TODO: deal with errors
                    if (logSyncDelay(actual_args->log_fd, actual_args->id, request.value.header.account_id, request.value.header.op_delay_ms * 1000) < 0) {
                        printf("Error writing to logfile\n");
                    }

                    reply.value.balance.balance = actual_args->accounts[request.value.header.account_id].balance;
                    reply.value.header.ret_code = RC_OK;

                    pthread_mutex_unlock(&actual_args->account_mutex[request.value.header.account_id]);
                    if (logSyncMech(actual_args->log_fd, actual_args->id, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_ACCOUNT, request.value.header.pid) < 0) {
                        printf("Error writing to logfile! \n");
                    }
                }

                break;

            case OP_TRANSFER:

                reply.type = request.type;
                reply.length = sizeof(rep_transfer_t);
                reply.value.header.account_id = request.value.header.account_id;
                reply.value.transfer.balance = request.value.transfer.amount;

                /* check if neither of the accounts is the admin account */
                if (request.value.header.account_id == ADMIN_ACCOUNT_ID || request.value.transfer.account_id == ADMIN_ACCOUNT_ID) {
                    reply.value.header.ret_code = RC_SAME_ID;
                }
                /* check if the origin account is valid */
                else if (actual_args->accounts[request.value.header.account_id].account_id == ERROR_ACCOUNT_ID) {
                    reply.value.header.ret_code = RC_ID_NOT_FOUND;
                }
                /* check if the destination account is valid */
                else if (actual_args->accounts[request.value.transfer.account_id].account_id == ERROR_ACCOUNT_ID) {
                    reply.value.header.ret_code = RC_ID_NOT_FOUND;
                }
                /* check if the accounts are different */
                else if (request.value.header.account_id == request.value.transfer.account_id) {
                    reply.value.header.ret_code = RC_SAME_ID;
                }
                /* check if the password for the origin account is valid */
                else if (!authenticate(request.value.header.password, &actual_args->accounts[request.value.header.account_id])) {
                    reply.value.header.ret_code = RC_LOGIN_FAIL;
                }
                /* check critical criteria and make the tranfer */
                else {
                    
                    /* For ease of readig the code, get the ids from the request struct */
                    uint32_t origin_id, destination_id;
                    origin_id = request.value.header.account_id;
                    destination_id = request.value.transfer.account_id;

                    /* In order to prevent deadlocks, the first account to get locked is always the one 
                       with the smallest id of the two */
                    if (origin_id > destination_id) {
                        pthread_mutex_lock(&actual_args->account_mutex[destination_id]);
                        if (logSyncMech(actual_args->log_fd, actual_args->id, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_ACCOUNT, request.value.header.pid) < 0) {
                            printf("Error writing to logfile! \n");
                        }
                        pthread_mutex_lock(&actual_args->account_mutex[origin_id]);
                        if (logSyncMech(actual_args->log_fd, actual_args->id, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_ACCOUNT, request.value.header.pid) < 0) {
                            printf("Error writing to logfile! \n");
                        }
                    }
                    else {
                        pthread_mutex_lock(&actual_args->account_mutex[origin_id]);
                        if (logSyncMech(actual_args->log_fd, actual_args->id, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_ACCOUNT, request.value.header.pid) < 0) {
                            printf("Error writing to logfile! \n");
                        }
                        pthread_mutex_lock(&actual_args->account_mutex[destination_id]);
                        if (logSyncMech(actual_args->log_fd, actual_args->id, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_ACCOUNT, request.value.header.pid) < 0) {
                            printf("Error writing to logfile! \n");
                        }
                    }

                    usleep(request.value.header.op_delay_ms * 1000);           // TODO: deal with errors
                    if (logSyncDelay(actual_args->log_fd, actual_args->id, request.value.header.account_id, request.value.header.op_delay_ms * 1000) < 0) {
                        printf("Error writing to logfile\n");
                    }

                    /* Now that the accounts are locked, we can check if the origin account has 
                       enough money to realize the transfer */
                    if (actual_args->accounts[origin_id].balance < request.value.transfer.amount) {
                        reply.value.header.ret_code = RC_NO_FUNDS;
                    }
                    /* Check if the destination account can store the money after the transfer */
                    else if (actual_args->accounts[destination_id].balance + request.value.transfer.amount > MAX_BALANCE) {
                        reply.value.header.ret_code = RC_TOO_HIGH;
                    }
                    /* All conditions are assured, only now can we make the transfer safely */
                    else {
                        actual_args->accounts[origin_id].balance -= request.value.transfer.amount;
                        actual_args->accounts[destination_id].balance += request.value.transfer.amount;

                        reply.value.header.ret_code = RC_OK;
                    }

                    pthread_mutex_unlock(&actual_args->account_mutex[origin_id]);
                    if (logSyncMech(actual_args->log_fd, actual_args->id, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_ACCOUNT, request.value.header.pid) < 0) {
                        printf("Error writing to logfile! \n");
                    }
                    pthread_mutex_unlock(&actual_args->account_mutex[destination_id]);
                    if (logSyncMech(actual_args->log_fd, actual_args->id, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_ACCOUNT, request.value.header.pid) < 0) {
                        printf("Error writing to logfile! \n");
                    }

                }

                break;
            
            case OP_SHUTDOWN:

                reply.type = request.type;
                reply.length = sizeof(rep_shutdown_t);
                reply.value.header.account_id = request.value.header.account_id;


                /* check if the admin was the one to request the server shutdown */
                if (request.value.header.account_id) {
                    reply.value.header.ret_code = RC_OP_NALLOW;
                }
                /* check if the password for the admin account is correct */
                else if (!authenticate(request.value.header.password, &actual_args->accounts[request.value.header.account_id])) {
                    reply.value.header.ret_code = RC_LOGIN_FAIL;
                }
                /* order the shutdown */
                else {  
                    
                    printf("O %d - Start building shutdown reply\n", actual_args->id);
                    *actual_args->shutdown = 1;
                    printf("O %d - Shutdown flag is set\n", actual_args->id);
                    reply.value.shutdown.active_offices = *actual_args->active_threads;
                    printf("O %d - Got the number of active threads\n", actual_args->id);
                    reply.value.header.ret_code = RC_OK;
                    printf("O %d - Built shutdown reply\n", actual_args->id);

                    usleep(request.value.header.op_delay_ms * 1000);        // TODO: deal with errors
                    if (logDelay(actual_args->log_fd, actual_args->id, request.value.header.op_delay_ms) < 0) {
                        printf("Error writing to logfile\n");
                    }

                }

                break;

            default:
                printf("Not a valid operation!\n");
                break;
        }


        /* Get the user FIFO to send the reply */
        char user_fifo[20];
        sprintf(user_fifo, "%s%d", USER_FIFO_PATH_PREFIX, request.value.header.pid);

        int user_fifo_fd = open(user_fifo, O_WRONLY | O_APPEND | O_NONBLOCK);       // TODO: deal with errors (errno is set accordingly)

        if (user_fifo_fd < 0) {
            reply.value.header.ret_code = RC_USR_DOWN;
            printf("Not able to find user fifo!\n");
            continue;
        }

        write(user_fifo_fd, &reply, sizeof(tlv_reply_t));

        close(user_fifo_fd);

        if (logReply(actual_args->log_fd, actual_args->id, &reply) < 0) {
            printf("Error writing to logfile\n");
        }
        *actual_args->active_threads = *actual_args->active_threads - 1;

    }    
    
    free(actual_args);
    
    pthread_exit(NULL);
}