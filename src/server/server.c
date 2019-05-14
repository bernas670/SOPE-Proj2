#include <stdlib.h>
#include <string.h>

#include "sope.h"

#define ARGS_ERROR 1
#define HASH_ERROR 2

int main(int argc, char* argv[]) {

    if (argc != 3) {
        printf("Invalid arguments! \n");
        return ARGS_ERROR;
    }

    int num_offices = atoi(argv[1]);
    if (num_offices <= 0 || num_offices > MAX_BANK_OFFICES) {
        printf("Invalid number of bank offices! \n");
        return ARGS_ERROR;
    }

    char admin_password[MAX_PASSWORD_LEN + 1];
    if (argv[2][0] != '"' || argv[2][strlen(argv[2])] != '"') {
        printf("Password must be written inside quotes (\"example\") \n");
        return ARGS_ERROR;
    }
    if (strlen(argv[2]) < MIN_PASSWORD_LEN - 2 || strlen(argv[2]) > MAX_PASSWORD_LEN - 2) {
        printf("Invalid password! Must have at least %d characters and a maximum of %d. \n", MIN_PASSWORD_LEN, MAX_PASSWORD_LEN);
        return ARGS_ERROR;
    }
    admin_password[strlen(admin_password)] = '\0';

    
    bank_account_t accounts[MAX_BANK_ACCOUNTS];


    /* Create admin account */
    bank_account_t admin_account;
    admin_account.account_id = ADMIN_ACCOUNT_ID;
    admin_account.balance = 0;
    generateSalt(&admin_account);
    if (generateHash(admin_password, &admin_account)) {
        printf("Error generating admin hash! \n");
        return HASH_ERROR;
    }
    /* Admin account created successfuly */


    /* Create counter threads */

    /* All counters are created */


    /* Create FIFO */

    /* FIFO is created */

    



    return 0;
}