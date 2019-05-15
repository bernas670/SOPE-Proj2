#ifndef OPERATIONS_H
#define OPERATIONS_H

#include "../sope.h"


void generateSalt(bank_account_t* account);
int generateHash(char* password, bank_account_t* account, char* hash);


/**
 * @brief Authenticator function, checks if the password is correct for a given account
 * 
 * @param password  Password for the account
 * @param account   Account the user is trying to log in to
 * @return int      Returns 1 if the password is correct and 0 otherwise
 */
int authenticate(char* password, bank_account_t* account);



#endif