#include <stdio.h>
#include <string.h>

#include "args.h"

//user programm

instruction *data;

int main(int argc,char* argv[]) {

    data = create_instruction();

    if(get_arguments(argc, argv, data) == -1){
        return -1;
    }

    printf("ID : %d\n", get_id(data));
    printf("Password : %s\n", get_password(data));
    printf("Latency : %d\n", get_latency(data));
    printf("Operation : %d\n", get_operation(data));
    if(get_flag(data)){
        printf("Flag true\n");
    }
    printf("Arg1 : %s\n", get_arg1(data));
    printf("Arg2 : %s\n", get_arg2(data));
    printf("Arg3 : %s\n", get_arg3(data));

    return 0;
}