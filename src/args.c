#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "instruction.h"
#include "constants.h"



int get_arguments(int argc, char* argv[],instruction *data){

    //process id
    
    // falta testar se é valido
    set_id(data, atoi(argv[1]));



    //process password
    int length = strlen(argv[2]);

    if(length-2 < MIN_PASSWORD_LEN || length-2 > MAX_PASSWORD_LEN){
        printf("size not allowed\n");
        return -1;
    }
    set_password(data, argv[2]);


    //process latency
    if(atoi(argv[3]) > MAX_OP_DELAY_MS){
        printf("latency not allowed\n");
        return -1;
    }
    set_latency(data, atoi(argv[3]));


    //process instruction
    if(atoi(argv[4]) < 0 || atoi(argv[4]) > 3){
        printf("operation not allowed\n");
        return -1;
    }

    set_operation(data, atoi(argv[4]));

    //process flag
    if(atoi(argv[4]) == 1 || atoi(argv[4]) == 3){
        set_flag(data,false);
    }

    if(atoi(argv[4]) == 0 || atoi(argv[4]) == 2){
        set_flag(data,true);
    }

    // process last argument 
    if(get_flag){

        set_args(data, argv[5]);

        // ATENÇÃO !! preciso separar args de maneira diferente dependendo da opção do argv[4]

    }


}