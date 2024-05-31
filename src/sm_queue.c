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
	q->ctl.size++;
}

static sm_event *dequeue(sm_queue * q) {
    sm_event * e = q->head->next;
    if(e != NULL) {
        q->head->next = q->head->next->next;
        if(e->next == NULL)
            q->tail = q->head;
        else
            e->next = NULL;
		q->ctl.size--;
    }
    return e;
}

typedef struct _sm_queue_sync {
	sm_event * head;
	uint16_t id_ext;
	uint16_t id;
	struct { 
		unsigned int size				 : 28;
		unsigned int synchronized		 :  1;
		unsigned int /* reserved */		 :  0;
	} ctl;	
    sm_event * tail;
    pthread_mutex_t lock; // 40B <== 
    pthread_cond_t empty; // 48B <==
} _sm_queue_sync;

static void append(sm_queue * q1, sm_queue * q2) {
	q1->tail = q2->tail;
	q1->tail->next = q1->head->next;
	q1->ctl.size = q1->ctl.size + q2->ctl.size;
	q2->tail = q2->head;
	q2->head->next = NULL;
	q2->ctl.size = 0;
	sm_queue_free(q2);
}

static void _sm_queue_free_sync(_sm_queue_sync *q) {
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

sm_queue *sm_queue_create(size_t event_size, size_t num_of_events, bool synchronized,  
						  bool handle_flag, bool hash_key_flag, bool priority_flag) {
	sm_queue *q;
	if(synchronized) {
    	if((q = malloc(sizeof(_sm_queue_sync))) == NULL) {
        	SM_LOG(SM_CORE, SM_LOG_ERR, "Failed to allocate queue header");
        	return NULL;
    	}
	}
	else
		if((q = malloc(sizeof(sm_queue))) == NULL) {
        	SM_LOG(SM_CORE, SM_LOG_ERR, "Failed to allocate queue header");
        	return NULL;
    	}
	sm_event *e;
    if((e = sm_event_create(SM_DUMMY_PAYLOAD_SIZE, false, false, false, false)) == NULL) {
        SM_LOG(SM_CORE, SM_LOG_ERR, "Failed to create queue dummy event");
        free(q);
        return NULL;
    }
    *((unsigned*)sm_event_data_ptr(e)) = SM_DUMMY_PAYLOAD;
	q->head = q->tail = e;
	sm_event *epool;
	if((epool = sm_event_create_pool(num_of_events,	event_size, hash_key_flag,
									 priority_flag, handle_flag, true)) == NULL) {
        SM_LOG(SM_CORE, SM_LOG_ERR, "Failed to create event pool");
        free(q);
        return NULL;
    }
	
	// ASSIGN HOME QUEUE ADDRESSES!!!
	
	q->tail = epool;
	q->head->next = epool->next;
	epool->next = NULL;
	q->ctl.size = num_of_events;
	q->ctl.synchronized = synchronized;
	SM_LOCK_INIT((_sm_queue_sync *)q, _sm_queue_free_sync);
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
	SM_LOCK_DESTROY((_sm_queue_sync *)q);
	free(q);
}

int sm_queue_append(sm_queue *q1, sm_queue *q2) {
	SM_LOCK((_sm_queue_sync *)q1);
	append(q1, q2);
	SM_SIGNAL_UNLOCK((_sm_queue_sync *)q1);
	return EXIT_SUCCESS;
}

size_t sm_queue_size(sm_queue * q) {
    return q->ctl.size;
}

sm_event * sm_queue_top(const sm_queue * q) {
    return q->head->next;
}

int sm_queue_enqueue(sm_event * e, sm_queue * q){
	SM_LOCK((_sm_queue_sync *)q);
    enqueue(e, q);
	SM_SIGNAL_UNLOCK((_sm_queue_sync *)q);
    return EXIT_SUCCESS;
}

sm_event *sm_queue_dequeue(sm_queue * q) {
	sm_event * e;
	SM_LOCK_WAIT((_sm_queue_sync *)q, e = dequeue(q)); 
    return e;
}
