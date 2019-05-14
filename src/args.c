#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "instruction.h"
#include "constants.h"



int get_arguments(int argc, const* argv[],instruction *data){

    //process id
    if(!isdigit(atoi(argv[1]))){
        printf("not an integer\n");
        return -1;
    }
    set_id(data,argv[1]);



  //process password
    int length = strlen(argv[2]);

    if( (argv[2])[0] != '"' || (argv[2])[length-1] != '"'){
        printf("not valid format\n");
        return -1;
    }

    if(length-2 < MIN_PASSWORD_LEN || length-2 > MAX_PASSWORD_LEN){
        printf("size not allowed\n");
        return -1;
    }
    set_password(data,argv[2]);


    //process latency
    if(argv[3]> MAX_OP_DELAY_MS){
        printf("latency not allowed\n");
        return -1;
    }
    set_latency(data,argv[3]);


    //process instruction
    if(argv[4] < 0 || argv[4] > 3){
        printf("operation not allowed\n");
        return -1;
    }

    set_operation(data,argv[4]);

    //process flag
    if(argv[4]== 1 || argv[4] == 3){
        set_flag(data,false);
    }

    if(argv[4]== 0 || argv[4] == 2){
        set_flag(data,true);
    }




}