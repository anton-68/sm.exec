/* SM.EXEC
   Queue module
   anton.bondarenko@gmail.com */

#ifndef SM_QUEUE_H
#define SM_QUEUE_H

#include "sm_event.h"
#include "sm_logger.h"

/* sm_queue */

typedef struct sm_queue {
    pthread_mutex_t lock;
    pthread_cond_t empty;
    sm_event * head;
    sm_event * tail;
	bool synchronized;
	size_t size;
} sm_queue;

// Public methods

sm_queue *sm_queue_create(size_t event_size, unsigned num_of_events, bool synchronized);
void sm_queue_free(sm_queue *q);
size_t sm_queue_size(sm_queue *q);

sm_event * sm_queue_top(const sm_queue * q);
int sm_queue_enqueue(sm_event *e, sm_queue *q);
sm_event *sm_queue_dequeue(sm_queue *q);

#endif //SM_QUEUE_H
