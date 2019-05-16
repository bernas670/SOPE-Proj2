#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../sope.h"
#include "operations.h"
#include "office.h"
#include "request_queue.h"

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
    if (strlen(argv[2]) < MIN_PASSWORD_LEN || strlen(argv[2]) > MAX_PASSWORD_LEN) {
        printf("Invalid password! Must have at least %d characters and a maximum of %d. \n", MIN_PASSWORD_LEN, MAX_PASSWORD_LEN);
        return ARGS_ERROR;
    }
    /* Admin's password is valid */

    
    /* Create bank account array to store all accounts and mutex array to lock account action */
    bank_account_t accounts[MAX_BANK_ACCOUNTS + 1];
    bank_account_t admin_account;
    admin_account.account_id = ERROR_ACCOUNT_ID;
    for (int i = 1; i <= MAX_BANK_ACCOUNTS + 1; i++) {
        accounts[i] = admin_account;
    }
    pthread_mutex_t account_mutex[MAX_BANK_ACCOUNTS + 1];


    /* Create admin account */
    admin_account.account_id = ADMIN_ACCOUNT_ID;
    admin_account.balance = 0;
    generateSalt(&admin_account);
    char hash[HASH_LEN + 1];
    if (generateHash(argv[2], &admin_account, hash)) {
        printf("Error generating admin hash! \n");
        return HASH_ERROR;
    }
    strcpy(admin_account.hash, hash);
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


    /* Create a request queue, for the server to store the requests */
    request_queue_t *queue = create_request_queue(num_offices);
    if (queue == NULL) {
        printf("Could not create a queue for the requests! \n");
        return 1;
    }
    pthread_mutex_t queue_lock;
    pthread_mutex_init(&queue_lock, NULL);                         // TODO: deal with errors

    pthread_cond_t empty_cond, full_cond;
    pthread_cond_init(&empty_cond, NULL);
    pthread_cond_init(&full_cond, NULL);
    /* Queue is created and ready for use */


    /* Create counter threads */
    pthread_t threads[num_offices + 1];
    threads[MAIN_THREAD_ID] = MAIN_THREAD_ID;
    for (int i = 1; i <= num_offices; i++) {

        office_args_t *args = malloc(sizeof(office_args_t));
        args->log_fd = logfile_fd;
        args->id = i;
        args->shutdown = &shutdown;
        args->accounts = accounts;
        args->account_mutex = account_mutex;
        args->queue = queue;
        args->queue_lock = &queue_lock;
        args->empty_cond = &empty_cond;
        args->full_cond = &full_cond;

        pthread_create(&threads[i], NULL, office_main, args);       // TODO: deal with errors
        printf("Created thread %d\n", i);
    }
    /* All counters are created */


    /* Create and open FIFO */
    mkfifo(SERVER_FIFO_PATH, 0666);                                 // TODO: deal with errors
    printf("Created server FIFO\n");
    int request_fd = open(SERVER_FIFO_PATH, O_RDONLY | O_NONBLOCK); // TODO: deal with errors
    int fifo_open = 1;
    /* FIFO is created and open */


    /* Loop for the server to receive messages
       If the server is ordered to shutdown this loop will end */
    int request_size = sizeof(tlv_request_t);
    tlv_request_t request_buf;
    int fifo_eof = 0;
    
    while (!shutdown || !fifo_eof) {

        if (shutdown && fifo_open) {
            fifo_open = 0;
            chmod(SERVER_FIFO_PATH, 0444);  // TODO: deal with errors
            printf("Server is in shutdown mode!\n");
        }

        pthread_mutex_lock(&queue_lock);
        while(is_full(queue)) {
            pthread_cond_wait(&full_cond, &queue_lock);
        }
        pthread_mutex_unlock(&queue_lock);

        fifo_eof = read(request_fd, &request_buf, request_size);

        if (fifo_eof > 0) {     // TODO: deal with errors
            printf("Got a request!\n");

            
            pthread_mutex_lock(&queue_lock);
            if (logSyncMech(logfile_fd, MAIN_THREAD_ID, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_PRODUCER, request_buf.value.header.pid) < 0) {
                printf("Error writing to logfile! \n");
            }
            
            if (push(queue, request_buf)) {
                printf("Could not add request to queue, queue is full! \n");
            }
            pthread_cond_signal(&empty_cond);

            pthread_mutex_unlock(&queue_lock);
            if (logSyncMech(logfile_fd, MAIN_THREAD_ID, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_PRODUCER, request_buf.value.header.pid) < 0) {
                printf("Error writing to logfile! \n");
            }
        } 
    }


    /* Close and request FIFO from systme because the shutdown command was received */
    close(request_fd);
    remove(SERVER_FIFO_PATH);


    /* Wait for all requests in the queue to be processed */
    for (int i = 1; i <= num_offices; i++) {
        pthread_join(threads[i], NULL);
    }


    /* Close the server's logfile */
    close(logfile_fd);

    return 0;
}