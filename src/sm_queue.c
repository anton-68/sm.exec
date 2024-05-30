/* SM.EXEC
   Queues   
   anton.bondarenko@gmail.com */

#include <stdlib.h>			// malloc-free

#include "sm_queue.h"

/* queue */

// Private methods

static void enqueue(sm_event *e, sm_queue *q);
static sm_event *dequeue(sm_queue *q);

// Public methods

sm_queue *sm_queue_create(size_t event_size, unsigned num_of_events, bool synchronized) {
    sm_queue *q;
    if((q = malloc(sizeof(sm_queue))) == NULL) {
        REPORT(ERROR, "malloc()");
        return NULL;
    }    
    sm_event * e;
    if((e = sm_event_create(TL_DUMMY_PAYLOAD_SIZE)) == NULL) {
        REPORT(ERROR, "event_create()");
        free(q);
        return NULL;
    }
    *((unsigned*)(e->data)) = TL_DUMMY_PAYLOAD;
	e->home = NULL;
    q->head = q->tail = e;
    int i;   
    for(i = 0; i < num_of_events; i++) {
        if((e = sm_event_create(event_size)) == NULL) {
            REPORT(ERROR, "event_create()");
            sm_queue_free(q);
            return NULL;
        }
		e->home = q;
        enqueue(e, q);
    }
	q->synchronized = synchronized;
	if(q->synchronized){
    	pthread_mutexattr_t attr;
    	if(pthread_mutexattr_init(&attr) != EXIT_SUCCESS) {
        	REPORT(ERROR, "pthread_mutexattr_init()");
        	sm_queue_free(q);
        	return NULL;
    	}
    	if(pthread_mutexattr_settype(&attr, TL_MUTEX_TYPE) != EXIT_SUCCESS){
        	REPORT(ERROR, "pthread_mutexattr_settype()");
        	sm_queue_free(q);
        	return NULL;
    	}    
    	if(pthread_mutex_init(&(q->lock), &attr) != EXIT_SUCCESS) {
        	REPORT(ERROR, "pthread_mutex_init()");
        	sm_queue_free(q);
        	return NULL;
    	}
    	if(pthread_cond_init(&(q->empty),NULL) != EXIT_SUCCESS) {
        	REPORT(ERROR, "pthread_cond_init()");
        	pthread_cond_signal(&q->empty);
        	sm_queue_free(q);
        	return NULL;
    	}
	}
	q->size = 0;
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
	if(q->synchronized){
    	pthread_mutex_destroy(&q->lock);
    	pthread_cond_destroy(&q->empty);
    	free(q);
	}
}

size_t sm_queue_size(sm_queue * q) {
    return q->size;
}

sm_event * sm_queue_top(const sm_queue * q) {
    return q->head->next;
}

void enqueue(sm_event * e, sm_queue * q) {
    q->tail->next = e;
    q->tail = e;
    q->tail->next = NULL;
	q->size++;
}

int sm_queue_enqueue(sm_event * e, sm_queue * q){
	if(q->synchronized){
    	if(pthread_mutex_lock(&(q->lock)) != EXIT_SUCCESS) {
        	REPORT(ERROR, "pthread_mutex_lock()");
        	return EXIT_FAILURE;
   		}
	}
    enqueue(e, q);
	if(q->synchronized){
    	if(pthread_cond_signal(&(q->empty)) != EXIT_SUCCESS) {
       		REPORT(ERROR, "pthread_cond_signal()");
        	return EXIT_FAILURE;
    	}
    	if(pthread_mutex_unlock(&(q->lock)) != EXIT_SUCCESS) {
        	REPORT(ERROR, "pthread_mutex_unlock()");
        	return EXIT_FAILURE;
    	}
	}
    return EXIT_SUCCESS;
}

sm_event *dequeue(sm_queue * q) {
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

sm_event *sm_queue_dequeue(sm_queue * q) {
	sm_event * e;
	if(q->synchronized){
    	int __tl_result = pthread_mutex_lock(&(q->lock));
    	if(__tl_result != EXIT_SUCCESS) {
        	REPORT(ERROR, "pthread_mutex_lock()");
        	return NULL;
    	}
    	while((e = dequeue(q)) == NULL) {
        	__tl_result = pthread_cond_wait(&(q->empty), &(q->lock));
        	if (__tl_result != EXIT_SUCCESS) {
            	REPORT(ERROR, "pthread_cond_wait()");
            	return NULL;
        	}
    	}
    	__tl_result = pthread_mutex_unlock(&(q->lock));
    	if(__tl_result != EXIT_SUCCESS) {
        	REPORT(ERROR, "pthread_mutex_unlock()");
        	// enqueue(e, q); 
			if (e != NULL){
				e->next = q->head->next;
				q->head->next = e;
				if (q->tail == q->head)
					q->tail = e;
			}
        	return NULL;
    	}
	}
	else
		e = dequeue(q);
    return e;
}



