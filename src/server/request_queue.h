#ifndef REQUEST_QUEUE_H
#define REQUEST_QUEUE_H


#include "../sope.h"


typedef struct request_queue {
    tlv_request_t **requests;   /**< Queue elements */
    unsigned int in;            /**< In index for the array */
    unsigned int out;           /**< Out index for the array */
    unsigned int size;          /**< Maximum size of the queue */
    unsigned int counter;       /**< Number of elements of the queue */
} request_queue_t;

/**
 * @brief Create a request queue with a fixed size
 * 
 * @param size                  Maximum initial size desired for the queue
 * @return request_queue_t*     Pointer to a request queue, NULL in case of error
 */
request_queue_t *create_request_queue(unsigned int size);

/**
 * @brief Delete a request queue
 * 
 * @param queue     Pointer to the queue that will be deleted
 */
void delete_request_queue(request_queue_t *q);

/**
 * @brief Push a request to the queue, the request is put on 
 *        on the queue if the queue is not full
 * 
 * @param queue     Pointer to the queue to which the request will be pushed
 * @param request   Pointer to the request which will be put on queue
 * @return int      Returns 0 if the push is successful, 1 otherwise
 */
int push(request_queue_t *q, tlv_request_t *request);

/**
 * @brief Pop the pointer to request that is at the head of the queue
 * 
 * @param q                 Pointer to the queue
 * @return tlv_request_t*   Returns a pointer to a request if successful, NULL in case of error
 */
tlv_request_t *pop(request_queue_t *q);

/**
 * @brief Checks if the specified queue is empty or not
 * 
 * @param queue     Pointer to the queue that will be analised
 * @return int      Returns 0 if the queue is empty, 1 if it is not
 */
int is_empty(request_queue_t *q);

/**
 * @brief Gets the number of elements present in the queue
 * 
 * @param queue     Pointer to the queue that will be analised
 * @return int      Returns the number of elements of the queue
 */
int num_elements(request_queue_t * q);

#endif
