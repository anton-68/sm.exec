/* SM.EXEC
   2-priority queue
   anton.bondarenko@gmail.com */

#ifndef SM_QUEUE2_H
#define SM_QUEUE2_H

#include "sm_event.h"
#include "../oam/logger.h"





/* sm_queue2 */

typedef struct sm_queue2 {
    pthread_mutex_t queue0_lock;
    pthread_mutex_t queue1_lock;
    pthread_cond_t empty;
    sm_event * h0;
    sm_event * t0;
    sm_event * h1;
    sm_event * t1;
} sm_queue2;

sm_queue2 *sm_queue2_create(/*size_t event_size, unsigned num_of_events*/);
void sm_queue2_free(sm_queue2 *q);
bool sm_queue2_is_empty(sm_queue2 *q); // too artificial to exist, deprecate?

sm_event *sm_queue2_get(const sm_queue2 *q);
sm_event *sm_queue2_get_high(const sm_queue2 *q);
void sm_enqueue2(sm_event *e, sm_queue2 *q);
void sm_enqueue2_high(sm_event *e, sm_queue2 *q);
int sm_lock_enqueue2(sm_event *e, sm_queue2 *q);
int sm_lock_enqueue2_high(sm_event *e, sm_queue2 *q);
sm_event *sm_dequeue2(sm_queue2 *q);
sm_event *sm_lock_dequeue2(sm_queue2 *q);

#endif //SM_QUEUE2_H
