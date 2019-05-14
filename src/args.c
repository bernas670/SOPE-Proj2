#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "instruction.h"



int get_arguments(int argc, const* argv[],instruction *data){

    if(!isdigit(atoi(argv[1]))){
        printf("not an integer");
        return -1; //erro
    }
    set_id(data,argv[1]);

    

}