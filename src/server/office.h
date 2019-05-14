#ifndef OFFICE_H
#define OFFICE_H


typedef struct office_args {
    int id;
    int log_fd;
    int* shutdown;
} office_args_t;


void *office_main(void *args);



#endif
