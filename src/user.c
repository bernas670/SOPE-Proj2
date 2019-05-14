#include <stdio.h>
#include <string.h>

#include "args.h"

//user programm

instruction *data;

int main(int argc,char* argv[]) {

    data = create_instruction();

    get_arguments(argc, argv, data);

    printf("ID : %d\n", get_id(data));
    printf("Password : %s\n", get_password(data));
    printf("Latency : %d\n", get_latency(data));
    printf("Operation : %d\n", get_operation(data));
    if(get_flag(data)){
        printf("Flag true\n");
    }
    printf("Args : %s\n", get_args(data));

    return 0;
}