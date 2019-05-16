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

    /*
    char buf[3];
    sprintf(buf, "%d\n", actual_args->id);
    write(actual_args->log_fd, buf, strlen(buf));
    */

    /* The thread will keep looking for new requests, as long as the
       server is live or the request queue is not empty */
    while(true) {

        printf("O %d - Going to lock the queue\n", actual_args->id);
        pthread_mutex_lock(actual_args->queue_lock);        // TODO: log the action
        printf("O %d - Locked the queue\n", actual_args->id);
        printf("O %d - Checking if the queue is empty\n", actual_args->id);
        while(is_empty(actual_args->queue) && ((*actual_args->shutdown && *actual_args->fifo_eof > 0) || !*actual_args->shutdown)) {
            printf("O %d - Queue is empty\n", actual_args->id);
            //write(actual_args->log_fd, "O - Queue is empty", 19);
            pthread_cond_wait(actual_args->empty_cond, actual_args->queue_lock);    // TODO: log the action
        }
        printf("O %d - Broadcast received\n", actual_args->id);        
        printf("O %d - shutdown : %d, empty : %d, eof : %d\n", actual_args->id, *actual_args->shutdown, is_empty(actual_args->queue), *actual_args->fifo_eof);
        /* Exit condition for the thread */
        if (*actual_args->shutdown && is_empty(actual_args->queue) && *actual_args->fifo_eof <= 0) {
            pthread_mutex_unlock(actual_args->queue_lock);      // TODO: log this action
            printf("O %d - Shutting down office\n", actual_args->id);
            break;
        }
        /*
        printf("O %d - Going to unlock the queue\n", actual_args->id);
        pthread_mutex_unlock(actual_args->queue_lock);      // TODO: log this action
        */

        tlv_request_t request;

        /*
        printf("O %d - Locked the queue to get a request\n", actual_args->id);
        pthread_mutex_lock(actual_args->queue_lock);
        if (logSyncMech(actual_args->log_fd, actual_args->id, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_CONSUMER, request.value.header.pid) < 0) {
                printf("Error writing to logfile! \n");
        }
        */

        request = pop(actual_args->queue);
        pthread_cond_signal(actual_args->full_cond);        // TODO: log this action

        pthread_mutex_unlock(actual_args->queue_lock);      // TODO: log this action

        printf("O %d - Started processing a request!\n", actual_args->id);

        *actual_args->active_threads = *actual_args->active_threads + 1;

        logRequest(actual_args->log_fd, actual_args->id, &request);     // TODO: deal with errors

        tlv_reply_t reply;

        switch (request.type) {

            case OP_CREATE_ACCOUNT:     // TODO: apply delay and log the delay

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

                    pthread_mutex_lock(&actual_args->account_mutex[request.value.create.account_id]);       // TODO: log action

                    logSyncDelay(actual_args->log_fd, actual_args->id, request.value.header.account_id, request.value.header.op_delay_ms * 1000);
                    usleep(request.value.header.op_delay_ms * 1000);           // TODO: deal with errors

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

                    pthread_mutex_unlock(&actual_args->account_mutex[request.value.create.account_id]);     // TODO: log action
                }

                break;
            
            case OP_BALANCE:        // TODO: implement delays and corresponding logs

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
                    pthread_mutex_lock(&actual_args->account_mutex[request.value.header.account_id]);       // TODO: log this action

                    logSyncDelay(actual_args->log_fd, actual_args->id, request.value.header.account_id, request.value.header.op_delay_ms * 1000);
                    usleep(request.value.header.op_delay_ms * 1000);           // TODO: deal with errors

                    reply.value.balance.balance = actual_args->accounts[request.value.header.account_id].balance;

                    pthread_mutex_unlock(&actual_args->account_mutex[request.value.header.account_id]);     // TODO: log this action
                }

                break;

            case OP_TRANSFER:

                reply.type = request.type;
                reply.length = sizeof(rep_transfer_t);
                reply.value.header.account_id = request.value.header.account_id;
                reply.value.transfer.balance = 0;

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
                        // TODO: log these actions
                        pthread_mutex_lock(&actual_args->account_mutex[destination_id]);
                        pthread_mutex_lock(&actual_args->account_mutex[origin_id]);
                    }
                    else {
                        // TODO: log these actions
                        pthread_mutex_lock(&actual_args->account_mutex[origin_id]);
                        pthread_mutex_lock(&actual_args->account_mutex[destination_id]);
                    }

                    logSyncDelay(actual_args->log_fd, actual_args->id, request.value.header.account_id, request.value.header.op_delay_ms * 1000);
                    usleep(request.value.header.op_delay_ms * 1000);           // TODO: deal with errors

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
                        reply.value.transfer.balance = actual_args->accounts[origin_id].balance;
                    }

                    // TODO: log these actions
                    pthread_mutex_unlock(&actual_args->account_mutex[origin_id]);
                    pthread_mutex_unlock(&actual_args->account_mutex[destination_id]);

                }

                break;
            
            case OP_SHUTDOWN:       // FIXME: segmentation fault somewhere arround here
                                    // TODO: add delay

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

        logReply(actual_args->log_fd, actual_args->id, &reply);     // TODO: deal with errors
        *actual_args->active_threads = *actual_args->active_threads - 1;

        printf("O %d - Finished processing a request!\n", actual_args->id);
    }    
    
    printf("O %d - Freeing memory\n", actual_args->id);
    free(actual_args);
    
    pthread_exit(NULL);
}