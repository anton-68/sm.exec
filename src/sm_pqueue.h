/* SM.EXEC
   Priority queues
   anton.bondarenko@gmail.com */

#ifndef SM_PQUEUE_H
#define SM_PQUEUE_H

#include "sm_event.h"

typedef enum {
	no_linkage,
	by_low,
	by_high,
	by_both
} sm_pqueue_linkage;

typedef struct sm_pqueue {
	pthread_mutex_t lock;
	pthread_cond_t empty;
	sm_event **heap;
	size_t capacity;
	size_t size;
	struct {
		unsigned int synchronized		: 1;
		unsigned int /* 31 bits */		: 0;	
	} ctl;
} sm_pqueue;

sm_pqueue *sm_pqueue_create(size_t capacity, bool synchronized);
void sm_pqueue_free(sm_pqueue *q);
size_t sm_pqueue_size(sm_pqueue *q);
sm_event *sm_pqueue_top(const sm_pqueue * q);
int sm_pqueue_enqueue(sm_pqueue *q, sm_event *e);
sm_event *sm_pqueue_dequeue(sm_pqueue *q);

#endif //SM_PQUEUE_H
