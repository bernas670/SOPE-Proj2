#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "instruction.h"
#include "../constants.h"

int arg_split(instruction *data, char* args, int opt){

    const char s[] = " ";
    char* token;

    if(opt == 0){

        // testar se o primeiro arg é valido -- EM FALTA !!
        token = strtok(args, s);
        for(unsigned int i = 0; i < strlen(token); i++){
            if(token[i] < '0' || token[i] > '9'){
                printf("Arg1 error\n");
                return -1;
            }
        }
        set_arg1(data, token);

        token = strtok(NULL, s);
        for(unsigned int i = 0; i < strlen(token); i++){
            if(token[i] < '0' || token[i] > '9'){
                printf("Arg2 error\n");
                return -1;
            }
        }
        set_arg2(data, token);

        token = strtok(NULL, s);
        if(strlen(token) < 8 || strlen(token) > 20){
            printf("Arg3 error\n");
            return -1;
        }
        set_arg3(data, token);

    }
    else if(opt == 2){
        token = strtok(args, s);
        for(unsigned int i = 0; i < strlen(token); i++){
            if(token[i] < '0' || token[i] > '9'){
                printf("Arg1 error\n");
                return -1;
            }
        }
        set_arg1(data, token);

        token = strtok(NULL, s);
        for(unsigned int i = 0; i < strlen(token); i++){
            if(token[i] < '0' || token[i] > '9'){
                printf("Arg2 error\n");
                return -1;
            }
        }
        set_arg2(data, token);
    }

    return 0;
}

int get_arguments(int argc, char* argv[],instruction *data){

    if(argc != 6){
        printf("invalid arguments\n");
        return -1;
    }

    //process id
    
    // falta testar se é valido
    for(unsigned int i = 0; i < strlen(argv[1]); i++){
            if((argv[1])[i] < '0' || (argv[1])[i]> '9'){
                printf("id error\n");
                return -1;
            }
        }
    set_id(data, atoi(argv[1]));



    //process password
    int length = strlen(argv[2]);

    if(length < MIN_PASSWORD_LEN || length > MAX_PASSWORD_LEN){
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
        set_flag(data, false);
    }

    if(atoi(argv[4]) == 0 || atoi(argv[4]) == 2){
        set_flag(data, true);
    }

    // process last argument 
    if(get_flag(data)){

        const char s[4] = " ";
        char* token;
        int counter = 0;
        char* args = strdup(argv[5]);

        // testa se o ultimo argumento é valido
        if(atoi(argv[4]) == 0){

            token = strtok(argv[5], s);

            while(token != NULL){
                token = strtok(NULL, s);
                counter++;
            }

            if(counter != 3){
                printf("invalid args\n");
                return -1;
            }
            else{
                if(arg_split(data, args, 0) == -1){
                    return -1;
                }
            }

        }

        // testa se o ultimo argumento é valido
        else if(atoi(argv[4]) == 2){
            
            token = strtok(argv[5], s);

            while(token != NULL){
                token = strtok(NULL, s);
                counter++;
            }

            if(counter != 2){
                printf("invalid args\n");
                return -1;
            }
            else{
                if(arg_split(data, args, 2) == -1){
                    return -1;
                }
            }

        }

    }

    return 0;
}



