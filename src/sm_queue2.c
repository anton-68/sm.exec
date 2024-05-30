/* SM.EXEC
   2-priority queue
   anton.bondarenko@gmail.com */

#include <stdlib.h>	
#include "sm_queue2.h"

static sm_event *dequeue2(sm_queue2 *q){
	sm_event * e = q->h[0]->next;
    if(e != NULL) {
        q->h[0]->next = q->h[0]->next->next;
        if(e->next == NULL)
            q->t[0] = q->h[0];
        else
            e->next = NULL;
		q->size--;
    }
	else {
		e = q->h[1]->next;
    	if(e != NULL) {
        	q->h[1]->next = q->h[1]->next->next;
        	if(e->next == NULL)
            	q->t[1] = q->h[1];
        	else
            	e->next = NULL;
			q->size--;
    	}
	}
    return e;
}

static void enqueue2(sm_queue2 * q, sm_event * e, bool p) {
    q->t[p]->next = e;
    q->t[p] = e;
    q->t[p]->next = NULL;
	q->size++;
}

static void append2(sm_queue2 * q1, sm_queue * q2) {
	q1->t[1] = q2->tail;
	q1->t[1]->next = q1->h[1]->next;
	q1->size = q1->size + q2->size;
	q2->tail = q2->head;
	q2->head->next = NULL;
	q2->size = 0;
	sm_queue_free(q2);
}

sm_queue2 *sm_queue2_create(bool synchronized) {
    sm_queue2 *q;
    if((q = malloc(sizeof(sm_queue2))) == NULL) {
        SM_LOG(SM_CORE, SM_LOG_ERR, "Failed to allocate queue2 header");
        return NULL;
    }
	q->ctl->synchronized = synchronized;
	q->size = 0;
    sm_event * e;
	for(size_t i = 0; i <= 1; i++) {
		if((e = sm_event_create(SM_DUMMY_PAYLOAD_SIZE)) == NULL) {
        	SM_LOG(SM_CORE, SM_LOG_ERR, "Failed to create queue dummy event");
        	free(q);
        	return NULL;
    	}
    	*((unsigned*)sm_event_data_ptr(e)) = SM_DUMMY_PAYLOAD;
    	q->h[i] = q->t[i] = e;
	}
	SM_LOCK_INIT(q, sm_queue2_free);
    return q;
}

void sm_queue2_free(sm_queue2 *q) {
    sm_event *tmp, *e;
	for(size_t i = 0; i <= 1; i++) {
		e = q->h[i];
    	while(e != NULL) {
        	tmp = e->next;
        	sm_event_free(e);
        	e = tmp;
    	}
	}
	SM_LOCK_DESTROY(q);
	free(q);
}

int sm_queue2_append(sm_queue2 *q1, sm_queue *q2) {
	SM_LOCK(q1);
	append2(q1, q2);
	SM_SIGNAL_UNLOCK(q1);
	return EXIT_SUCCESS;
}

size_t sm_queue2_size(sm_queue2 *q) {
    return q->size;
}

sm_event * sm_queue2_top(const sm_queue2 * q) {
    return q->h[0]->next ? q->h[0]->next : q->h[1]->next;
}

int sm_queue2_enqueue_high(sm_queue2 * q, sm_event * e){
	SM_LOCK(q);
    enqueue2(q, e, 0);
	SM_SIGNAL_UNLOCK(q);
    return EXIT_SUCCESS;
}

int sm_queue2_enqueue_low(sm_queue2 * q, sm_event * e) {
	SM_LOCK(q);
    enqueue2(q, e, 1);
	SM_SIGNAL_UNLOCK(q);
    return EXIT_SUCCESS;
}

sm_event * sm_queue2_dequeue(sm_queue2 * q) {
	sm_event * e;
	SM_LOCK_WAIT(q, e = dequeue2(q)); 
    return e;
}
