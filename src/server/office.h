#ifndef OFFICE_H
#define OFFICE_H


#include <pthread.h>


#include "request_queue.h"


typedef struct office_args {
    int id;
    int log_fd;
    int *shutdown;
    int *active_threads;
    bank_account_t *accounts;
    pthread_mutex_t *account_mutex;
    request_queue_t *queue;
    pthread_mutex_t *queue_lock;
    pthread_cond_t *empty_cond;
    pthread_cond_t *full_cond;
} office_args_t;


void *office_main(void *args);

#endif
