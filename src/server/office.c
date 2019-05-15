#include <stdlib.h>

#include "office.h"
#include "../sope.h"


void *office_main(void *args) {
    office_args_t *actual_args = args;


    /* Log the openning of the office to the server logfile */
    if (logBankOfficeOpen(actual_args->log_fd, actual_args->id, pthread_self()) < 0) {
        printf("Error writing to logfile!\n");
    }


    /* The thread will keep looking for new requests, as long as the
       server is live or the request queue is not empty */
    while(!*actual_args->shutdown) {

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
    }


    /* Log the closure of the office to the server logfile */
    if (logBankOfficeClose(actual_args->log_fd, actual_args->id, pthread_self()) < 0) {
        printf("Error writing to logfile!\n");
    }    
    

    free(actual_args);


    pthread_exit(NULL);
}