/* SM.EXEC
   Queue
   anton.bondarenko@gmail.com */

#ifndef SM_QUEUE_H
#define SM_QUEUE_H

#include "sm_logger.h"
#include "sm_event.h"

typedef struct sm_queue {
    pthread_mutex_t lock;
    pthread_cond_t empty;
    sm_event * head;
    sm_event * tail;
	size_t size;
	struct {
		unsigned int synchronized		 :  1;
		unsigned int /* reserved */		 :  0; // 31 bits
	} ctl;
} sm_queue;

sm_queue *sm_queue_create(size_t event_size, 
						  size_t num_of_events, 
						  bool synchronized,   
						  bool handle_flag,
						  bool hash_key_flag,
						  bool priority_flag);
void sm_queue_free(sm_queue *q);
int sm_queue_append(sm_queue *q1, sm_queue *q2);
size_t sm_queue_size(sm_queue *q);
sm_event * sm_queue_top(const sm_queue * q);
int sm_queue_enqueue(sm_event *e, sm_queue *q);
sm_event *sm_queue_dequeue(sm_queue *q);

#endif //SM_QUEUE_H
