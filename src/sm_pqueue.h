/* SM.EXEC
   Priority queues
   anton.bondarenko@gmail.com */

#ifndef SM_PQUEUE_H
#define SM_PQUEUE_H

#include "sm_event.h"
#include "../oam/logger.h"

/* sm_pqueue */

typedef struct sm_pqueue {
	size_t capacity;
	size_t size;
	bool synchronized;
    pthread_mutex_t enqueue_lock;
	pthread_mutex_t dequeue_lock;
	pthread_cond_t empty;
	sm_event **heap;
} sm_pqueue;

// Public methods

sm_pqueue *sm_pqueue_create(size_t capacity, bool synchronized);
void sm_pqueue_free(sm_pqueue *q);
size_t sm_pqueue_size(sm_pqueue *q);

sm_event *sm_pqueue_top(const sm_pqueue * q);
int sm_pqueue_enqueue(sm_event *e, sm_pqueue *q);
sm_event *sm_pqueue_dequeue(sm_pqueue *q);

#endif //SM_PQUEUE_H