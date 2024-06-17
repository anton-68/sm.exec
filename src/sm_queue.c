/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Queue class
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#include "sm_queue.h"

// Private methods

static void enqueue(sm_event *e, sm_queue *q);
static sm_event *dequeue(sm_queue *q);

// Public methods

sm_queue *sm_queue_create(uint32_t event_size,
						  bool Q, bool K, bool P, bool H,
						  unsigned num_of_events,
						  bool synchronized)
{

	sm_queue *q;
    if((q = malloc(sizeof(sm_queue))) == NULL)
	{
		SM_REPORT(SM_LOG_ERR, "malloc() returned NULL");
		return NULL;
	}
	sm_event * e;
	if ((e = sm_event_create(0, false, false, false, false)) == NULL)
	{
		SM_REPORT(SM_LOG_ERR, "sm_event_create() returned NULL");
		free(q);
		return NULL;
	}
    q->head = q->tail = e;
	q->size = 0;

    for(int i = 0; i < num_of_events; i++) {
        if((e = sm_event_create(event_size, Q, K, P, H)) == NULL) {
			SM_REPORT(SM_LOG_ERR, "event_create()");
			sm_queue_free(q);
            return NULL;
        }
		SM_EVENT_DEPOT(e) = q;
		enqueue(e, q);
    }

	q->synchronized = synchronized;
	if(q->synchronized){
    	pthread_mutexattr_t attr;
    	if(pthread_mutexattr_init(&attr) != EXIT_SUCCESS) {
			SM_REPORT(SM_LOG_ERR, "pthread_mutexattr_init() failed");
			sm_queue_free(q);
        	return NULL;
    	}
    	if(pthread_mutexattr_settype(&attr, SM_MUTEX_TYPE) != EXIT_SUCCESS){
			SM_REPORT(SM_LOG_ERR, "pthread_mutexattr_settype() failed");
			sm_queue_free(q);
        	return NULL;
    	}    
    	if(pthread_mutex_init(&(q->lock), &attr) != EXIT_SUCCESS) {
			SM_REPORT(SM_LOG_ERR, "pthread_mutex_init() failed");
			sm_queue_free(q);
        	return NULL;
    	}
    	if(pthread_cond_init(&(q->empty),NULL) != EXIT_SUCCESS) {
			SM_REPORT(SM_LOG_ERR, "pthread_cond_init() failed");
			pthread_cond_signal(&q->empty);
        	sm_queue_free(q);
        	return NULL;
    	}
	}
	SM_REPORT(SM_LOG_DEBUG, "sm_queue created");
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
	SM_REPORT(SM_LOG_DEBUG, "sm_queue purged");
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
			SM_REPORT(SM_LOG_ERR, "pthread_mutex_lock() failed");
			return EXIT_FAILURE;
   		}
	}
    enqueue(e, q);
	if(q->synchronized){
    	if(pthread_cond_signal(&(q->empty)) != EXIT_SUCCESS) {
			SM_REPORT(SM_LOG_ERR, "pthread_cond_signal() failed");
			return EXIT_FAILURE;
    	}
    	if(pthread_mutex_unlock(&(q->lock)) != EXIT_SUCCESS) {
			SM_REPORT(SM_LOG_ERR, "pthread_mutex_unlock() failed");
			return EXIT_FAILURE;
    	}
	}
	SM_REPORT(SM_LOG_DEBUG, "event enqueed");
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
			SM_REPORT(SM_LOG_ERR, "pthread_mutex_lock() failed");
			return NULL;
    	}
    	while((e = dequeue(q)) == NULL) {
        	__tl_result = pthread_cond_wait(&(q->empty), &(q->lock));
        	if (__tl_result != EXIT_SUCCESS) {
				SM_REPORT(SM_LOG_ERR, "pthread_cond_wait() failed");
				return NULL;
        	}
    	}
    	__tl_result = pthread_mutex_unlock(&(q->lock));
    	if(__tl_result != EXIT_SUCCESS) {
			SM_REPORT(SM_LOG_ERR, "pthread_mutex_unlock() failed, retrying...");
			//  enqueue(e, q);
			/*
			if (e != NULL){
				e->next = q->head->next;
				q->head->next = e;
				if (q->tail == q->head)
					q->tail = e;
			}
        	return NULL;
			*/
			usleep(10000);
			__tl_result = pthread_mutex_unlock(&(q->lock));
			if (__tl_result != EXIT_SUCCESS)
				SM_REPORT(SM_LOG_ERR, "pthread_mutex_unlock() failed again, giving up...");
		}
	}
	else
		e = dequeue(q);
	SM_REPORT(SM_LOG_DEBUG, "event dequeued");
	return e;
}

int sm_queue_to_string(sm_queue *q, char *buffer)
{
	char *s = buffer;
	s += sprintf(s, "address: %p\n", q);
	s += sprintf(s, "size: %lu\n", sm_queue_size(q));
	s += sprintf(s, "synchronized: %u\n", q->synchronized);
	return (int)((char *)s - (char *)buffer);
}
