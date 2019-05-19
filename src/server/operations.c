#include "operations.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>


void generateSalt(bank_account_t* account) {

    const char characters[] = "0123456789abcdef";
    
    time_t t;    
    srand((unsigned) time(&t));

    for (size_t i = 0; i < SALT_LEN; i++) {
        account->salt[i] = characters[rand() % strlen(characters)];
    }
    account->salt[SALT_LEN] = '\0';

}


int generateHash(char* password, bank_account_t* account, char* hash) {

    const int buf_size = 100;
    char buf[buf_size];

    sprintf(buf, "echo -n %s%s | sha256sum", password, account->salt);

    FILE* filep = popen(buf, "r"); //READ-ONLY

    if (filep == NULL)
        return 1;

    fread(buf, 1, buf_size, filep);
    pclose(filep);

    buf[HASH_LEN] = '\0';
    strcpy(hash, buf);    

    return 0;      
}


int authenticate(char* password, bank_account_t* account) {

    char hash[HASH_LEN + 1];
    generateHash(password, account, hash);

    if (strcmp(hash, account->hash)) {
        return 0;
    }

    return 1;
}
