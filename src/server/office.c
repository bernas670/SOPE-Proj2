#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "office.h"
#include "../sope.h"
#include "operations.h"


void *office_main(void *args) {
    office_args_t *actual_args = args;


    /* Log the openning of the office to the server logfile */
    if (logBankOfficeOpen(actual_args->log_fd, actual_args->id, pthread_self()) < 0) {
        printf("Error writing to logfile!\n");
    }


    /* The thread will keep looking for new requests, as long as the
       server is live or the request queue is not empty */
    while(true) {   // TODO: find a way to shutdown the threads

        pthread_mutex_lock(actual_args->queue_lock);        // TODO: log the action
        while(is_empty(actual_args->queue)) {
            pthread_cond_wait(actual_args->empty_cond, actual_args->queue_lock);    // TODO: log the action
        }
        pthread_mutex_unlock(actual_args->queue_lock);      // TODO: log this action

        tlv_request_t request;

        pthread_mutex_lock(actual_args->queue_lock);
        if (logSyncMech(actual_args->log_fd, actual_args->id, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_CONSUMER, request.value.header.pid) < 0) {
                printf("Error writing to logfile! \n");
        }

        request = pop(actual_args->queue);
        pthread_cond_signal(actual_args->full_cond);        // TODO: log this action

        usleep(request.value.header.op_delay_ms);           // TODO: deal with errors

        tlv_reply_t reply;

        switch (request.type) {

            case OP_CREATE_ACCOUNT:     // TODO: apply delay and log the delay

                /* check if the admin was the one to request the creation */
                if (request.value.header.account_id) {
                    reply.value.header.ret_code = RC_OP_NALLOW;
                    reply.value.header.account_id = request.value.header.account_id;
                }
                /* check if the password for the admin account is correct */
                else if (!authenticate(request.value.header.password, &actual_args->accounts[request.value.header.account_id])) {
                    reply.value.header.ret_code = RC_LOGIN_FAIL;
                    reply.value.header.account_id = request.value.header.account_id;
                }
                /* check if the account id is not used */
                else if (actual_args->accounts[request.value.create.account_id].account_id == ERROR_ACCOUNT_ID) {
                    reply.value.header.ret_code = RC_ID_IN_USE;
                    reply.value.header.account_id = request.value.header.account_id;
                }
                /* create the account */
                else {
                    pthread_mutex_init(&actual_args->account_mutex[request.value.create.account_id], NULL); // TODO: deal with errors

                    pthread_mutex_lock(&actual_args->account_mutex[request.value.create.account_id]);       // TODO: log action

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

                    reply.value.header.account_id = new_account.account_id;
                    reply.value.header.ret_code = RC_OK;

                    logAccountCreation(actual_args->log_fd, actual_args->id, &new_account);

                    pthread_mutex_unlock(&actual_args->account_mutex[request.value.create.account_id]);     // TODO: log action
                }

                reply.type = request.type;
                reply.length = 0;
                
                break;
            
            case OP_BALANCE:

                reply.type = request.type;
                reply.length = sizeof(rep_balance_t);

                break;

            case OP_TRANSFER:

                reply.type = request.type;
                reply.length = sizeof(rep_transfer_t);

                break;
            
            case OP_SHUTDOWN:

                reply.type = request.type;
                reply.length = sizeof(rep_shutdown_t);

                break;

            default:
                printf("Not a valid operation!\n");
                break;
        }

        logReply(actual_args->log_fd, actual_args->id, &reply);     // TODO: deal with errors

        /* Get the user FIFO to send the reply */
        char user_fifo[20];
        sprintf(user_fifo, "%s%d", USER_FIFO_PATH_PREFIX, request.value.header.pid);

        int user_fifo_fd = open(user_fifo, O_WRONLY | O_APPEND | O_NONBLOCK);       // TODO: deal with errors (errno is set accordingly)

        if (user_fifo < 0) {
            printf("Not able to find fifo!\n");
            continue;
        }

        write(user_fifo_fd, &reply, sizeof(tlv_reply_t));   // TODO: deal with errors
        
    }


    /* Log the closure of the office to the server logfile */
    if (logBankOfficeClose(actual_args->log_fd, actual_args->id, pthread_self()) < 0) {
        printf("Error writing to logfile!\n");
    }    
    

    free(actual_args);


    pthread_exit(NULL);
}