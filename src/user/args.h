#ifndef ARGS_H
#define ARGS_H

#include "instruction.h"

int get_arguments(int argc, char* argv[], instruction *data);

int arg_split(instruction *data, char* args, int opt);


#endif