#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "instruction.h"

struct st_instruction{
    int pid;
    int id;
    char* password;
    int latency;
    int operation;
    bool has_args;
    char* args;
};

instruction* create_instruction(){
    instruction *ptr= malloc(sizeof(instruction));
    
    if(ptr==NULL){
        return NULL;
    }

    memset(ptr,0,sizeof(instruction));

    ptr->pid = getpid();

    return ptr;
}

void delete_instruction(instruction *ptr){
    free(ptr);
}



/* Getters and Setters */

int get_id(instruction *ptr){
    return ptr->id;
}
void set_id(instruction *ptr, int i_id){
    ptr->id = i_id;
}

char *get_password(instruction *ptr){
    return ptr->password;
}
void set_password(instruction *ptr, char *i_password){
    ptr->password = i_password;
}

int get_latency(instruction *ptr){
    return ptr->latency;
}
void set_latency(instruction *ptr, int i_latency){
    ptr->latency = i_latency;
}

int get_operation(instruction *ptr){
    return ptr->operation;
}
void set_operation(instruction *ptr, int i_operation){
    ptr->operation = i_operation;
}

bool get_flag(instruction *ptr){
    return ptr->has_args;
}
void set_flag(instruction *ptr, bool i_flag){
    ptr->has_args = i_flag;
}

char *get_args(instruction *ptr){
    return ptr->args;
}
void set_args(instruction *ptr, char *i_args){
    ptr->args = i_args;
}