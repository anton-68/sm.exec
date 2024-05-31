/* SM.EXEC
   Queues   
   anton.bondarenko@gmail.com */

#include <stdlib.h>	
#include "sm_queue.h"
#include "sm_logger.h"

static void enqueue(sm_event * e, sm_queue * q) {
    q->tail->next = e;
    q->tail = e;
    q->tail->next = NULL;
	q->size++;
}

static sm_event *dequeue(sm_queue * q) {
    sm_event * e = q->head->next;
    if(e != NULL) {
        q->head->next = q->head->next->next;
        if(e->next == NULL)
            q->tail = q->head;
        else
            e->next = NULL;
		q->size--;
    }
    return e;
}

static void append(sm_queue * q1, sm_queue * q2) {
	q1->tail = q2->tail;
	q1->tail->next = q1->head->next;
	q1->size = q1->size + q2->size;
	q2->tail = q2->head;
	q2->head->next = NULL;
	q2->size = 0;
	sm_queue_free(q2);
}

sm_queue *sm_queue_create(size_t event_size, size_t num_of_events, bool synchronized,  
						  bool handle_flag, bool hash_key_flag, bool priority_flag) {
	sm_queue *q;
    if((q = malloc(sizeof(sm_queue))) == NULL) {
        SM_LOG(SM_CORE, SM_LOG_ERR, "Failed to allocate queue header");
        return NULL;
    }   
	sm_event *e;
    if((e = sm_event_create(SM_DUMMY_PAYLOAD_SIZE)) == NULL) {
        SM_LOG(SM_CORE, SM_LOG_ERR, "Failed to create queue dummy event");
        free(q);
        return NULL;
    }
    *((unsigned*)sm_event_data_ptr(e)) = SM_DUMMY_PAYLOAD;
	q->head = q->tail = e;
	sm_event *epool;
	if((epool = sm_event_ext_create_pool(num_of_events,	event_size, 1, handle_flag,
										 hash_key_flag,	priority_flag, q)) == NULL) {
        SM_LOG(SM_CORE, SM_LOG_ERR, "Failed to create event pool");
        free(q);
        return NULL;
    }
	q->tail = epool;
	q->head->next = epool->next;
	epool->next = NULL;
	q->size = num_of_events;
	q->ctl.synchronized = synchronized;
	SM_LOCK_INIT(q, sm_queue_free);
    return q;
}

void sm_queue_free(sm_queue *q) {
    sm_event * tmp; 
    sm_event * e = q->head;
    while(e != NULL) {
        tmp = e->next;
        sm_event_free(e);
        e = tmp;
    }
	SM_LOCK_DESTROY(q);
	free(q);
}

int sm_queue_append(sm_queue *q1, sm_queue *q2) {
	SM_LOCK(q1);
	append(q1, q2);
	SM_SIGNAL_UNLOCK(q1);
	return EXIT_SUCCESS;
}

size_t sm_queue_size(sm_queue * q) {
    return q->size;
}

sm_event * sm_queue_top(const sm_queue * q) {
    return q->head->next;
}

int sm_queue_enqueue(sm_event * e, sm_queue * q){
	SM_LOCK(q);
    enqueue(e, q);
	SM_SIGNAL_UNLOCK(q);
    return EXIT_SUCCESS;
}

sm_event *sm_queue_dequeue(sm_queue * q) {
	sm_event * e;
	SM_LOCK_WAIT(q, e = dequeue(q)); 
    return e;
}
