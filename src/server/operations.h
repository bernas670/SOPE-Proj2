#ifndef OPERATIONS_H
#define OPERATIONS_H

#include "../sope.h"


void generateSalt(bank_account_t* account);
int generateHash(char* password, bank_account_t* account, char* hash);



#endif