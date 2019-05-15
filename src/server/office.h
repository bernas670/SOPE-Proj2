#ifndef OFFICE_H
#define OFFICE_H


#include <pthread.h>


#include "request_queue.h"


typedef struct office_args {
    int id;
    int log_fd;
    int* shutdown;
    request_queue_t *queue;
    pthread_mutex_t *queue_mutex;
} office_args_t;


void *office_main(void *args);

#endif
