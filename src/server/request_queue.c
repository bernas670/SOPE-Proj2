#include <stdlib.h>


#include "request_queue.h"


/**
 * @brief Adjusts the in and out attributes of the queue to the 
 *        size of the array, avoiding accessing inexistent array elements 
 * 
 * @param q     Pointer to the queue that needs adjusting
 */
static void adjust_queue(request_queue_t *q) {
    q->in = q->in % q->size;
    q->out = q->out % q->size;
}

request_queue_t *create_request_queue(unsigned int size) {

    request_queue_t *q = malloc(sizeof(request_queue_t));

    if (q == NULL) {
        return NULL;
    }

    q->size = size ? size : 1;
    q->requests = malloc(q->size * sizeof(tlv_request_t));

    if (q->requests == NULL) {
        free(q);
        return NULL;
    }

    q->in = q->out = q->counter = 0;

    return q;
}

void delete_request_queue(request_queue_t *q) {
    free(q->requests);
    free(q);
}

int push(request_queue_t *q, tlv_request_t *request) {
    if (q->counter == q->size) {
        return 1;
    }

    q->requests[q->in] = request;
    q->in++;
    q->counter++;

    adjust_queue(q);

    return 0;
}

tlv_request_t *pop(request_queue_t *q) {
    if (is_empty(q)) {
        return NULL;
    }

    tlv_request_t *r = q->requests[q->out];
    q->out++;
    q->counter--;

    adjust_queue(q);
    
    return r;
}

int is_empty(request_queue_t *q) {
    if (q->counter == 0) {
        return 1;
    }

    return 0;
}

int num_elements(request_queue_t * q) {
    return q->counter;
}
