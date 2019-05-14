#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../sope.h"
#include "operations.h"
#include "office.h"

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


    /* Check if the admin's password respects the desired conditions */
    // TODO: check if this is the correct way of dealing with the password, account for the quotes or not?
    char admin_password[MAX_PASSWORD_LEN + 3];
    if (argv[2][0] != '"' || argv[2][strlen(argv[2])] != '"') {
        printf("Password must be written inside quotes (\"example\") \n");
        return ARGS_ERROR;
    }
    if (strlen(argv[2]) < MIN_PASSWORD_LEN - 2 || strlen(argv[2]) > MAX_PASSWORD_LEN - 2) {
        printf("Invalid password! Must have at least %d characters and a maximum of %d. \n", MIN_PASSWORD_LEN, MAX_PASSWORD_LEN);
        return ARGS_ERROR;
    }
    admin_password[strlen(admin_password)] = '\0';
    /* Admin's password is valid */

    
    /* Create bank account array to store all accounts and mutex array to lock account action */
    bank_account_t accounts[MAX_BANK_ACCOUNTS];
    pthread_mutex_t account_mutex[MAX_BANK_ACCOUNTS];


    /* Create admin account */
    bank_account_t admin_account;
    admin_account.account_id = ADMIN_ACCOUNT_ID;
    admin_account.balance = 0;
    generateSalt(&admin_account);
    if (generateHash(admin_password, &admin_account)) {
        printf("Error generating admin hash! \n");
        return HASH_ERROR;
    }
    /* Admin account created */


    /* Add the admins account to the array */
    accounts[ADMIN_ACCOUNT_ID] = admin_account;
    pthread_mutex_init(&account_mutex[ADMIN_ACCOUNT_ID], NULL);     // TODO: deal with errors
    /* Admin account created successfuly */

    
    /* This variable is set to 1 by one of the threads when the admin orders the server's shutdown */
    int shutdown = 0;


    /* Open logfile for the server */
    int logfile_fd = open(SERVER_LOGFILE, O_WRONLY | O_CREAT | O_APPEND, 0666); // TODO: deal with errors
    /* Logfile is created and open */


    /* Create counter threads */
    pthread_t threads[num_offices];
    for (int i = 0; i < num_offices; i++) {
        threads[i] = i;

        office_args_t *args = malloc(sizeof(office_args_t));
        args->log_fd = logfile_fd;
        args->id = i;
        args->shutdown = &shutdown;

        pthread_create(&threads[i], NULL, office_main, args);       // TODO: deal with errors
    }
    /* All counters are created */


    /* Create and open FIFO */
    mkfifo(SERVER_FIFO_PATH, 0666);                                 // TODO: deal with errors
    int request_fd = open(SERVER_FIFO_PATH, O_RDONLY);              // TODO: deal with errors
    /* FIFO is created and open */


    while (!shutdown) {

    }


    /* Close request FIFO */
    close(request_fd);

    /* Wait for all requests in the queue to be processed */
    for (int i = 0; i < num_offices; i++) {
        pthread_join(threads[i], NULL);
    }

    /* Close the server's logfile */
    close(logfile_fd);

    return 0;
}