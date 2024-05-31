/* SM.EXEC
   2-priority queue
   anton.bondarenko@gmail.com */

#ifndef SM_QUEUE2_H
#define SM_QUEUE2_H

#include "sm_event.h"
#include "sm_queue.h"

typedef struct sm_queue2 {
    pthread_mutex_t lock;
    pthread_cond_t empty;
    sm_event * h[2];
    sm_event * t[2];
	size_t size;
	struct {
		unsigned int synchronized		 :  1;
		unsigned int /* reserved */		 :  0; // 31 bits
	} ctl;
} sm_queue2;

sm_queue2 *sm_queue2_create(bool synchronized);
void sm_queue2_free(sm_queue2 *q);
int sm_queue2_append(sm_queue2 *q1, sm_queue *q2);
size_t sm_queue2_size(sm_queue2 *q);
sm_event *sm_queue2_top(const sm_queue2 * q);
int sm_queue2_enqueue_high(sm_queue2 * q, sm_event * e);
int sm_queue2_enqueue_low(sm_queue2 * q, sm_event * e);
sm_event *sm_queue2_dequeue(sm_queue2 *q);

#endif //SM_QUEUE2_H
