#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <stdbool.h>

struct st_instruction;
typedef struct st_instruction instruction;

instruction* create_instruction();
void delete_instruction(instruction *ptr);

int get_id(instruction *ptr);
void set_id(instruction *ptr, int i_id);

char *get_password(instruction *ptr);
void set_password(instruction *ptr, char *i_password);

int get_latency(instruction *ptr);
void set_latency(instruction *ptr, int i_latency);

int get_operation(instruction *ptr);
void set_operation(instruction *ptr, int i_operation);

bool get_flag(instruction *ptr);
void set_flag(instruction *ptr, bool i_flag);

char *get_arg1(instruction *ptr);
void set_arg1(instruction *ptr, char *i_args);

char *get_arg2(instruction *ptr);
void set_arg2(instruction *ptr, char *i_args);

char *get_arg3(instruction *ptr);
void set_arg3(instruction *ptr, char *i_args);

#endif
