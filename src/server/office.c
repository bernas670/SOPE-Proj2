#include <stdlib.h>

#include "office.h"
#include "../sope.h"


void *office_main(void *args) {
    office_args_t *actual_args = args;

    //logBankOfficeOpen(actual_args->log_fd, actual_args->id, pthread_self());

    while(!actual_args->shutdown /* && !message_queue.empty() */) {

    }

    //logBankOfficeClose(actual_args->log_fd, actual_args->id, pthread_self());

    free(actual_args);
    return 0;
}