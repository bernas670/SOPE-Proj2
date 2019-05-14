#include "operations.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>


void generateSalt(bank_account_t* account) {

    const char characters[] = "0123456789abcdef";
    
    time_t t;    
    srand((unsigned) time(&t));

    for (size_t i = 0; i < strlen(account->salt); i++) {
        account->salt[i] = characters[rand() % strlen(characters)];
    }
}


int generateHash(char* password, bank_account_t* account) {

    const int buf_size = 100;
    char buf[buf_size];

    sprintf(buf, "echo -n %s%s | sha256sum", password, account->salt);

    FILE* filep = popen(buf, "r"); //READ-ONLY

    if (filep == NULL)
        return 1;

    fread(buf, 1, buf_size, filep);
    pclose(filep);

    buf[strlen(account->hash)] = '\0';
    strcpy(account->hash, buf);
    return 0;      
}