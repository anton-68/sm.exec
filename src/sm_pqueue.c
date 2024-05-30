/* SM.EXEC
   Priority queues
   anton.bondarenko@gmail.com */

#include <stdlib.h>	
#include "sm_pqueue.h"

static bool less(sm_pqueue * pq, size_t e1, size_t e2) {
	for (size_t stage = 0; stage < SM_NUM_OF_PRIORITY_STAGES; stage++) {
		if(pq->heap[e1]->priority[stage] < pq->heap[e2]->priority[stage])
			return true;
		else {
			if(pq->heap[e1]->priority[stage] > pq->heap[e2]->priority[stage])
				return false;
		}
	}
	return false;
}

static void exch(sm_pqueue * pq, size_t e1, size_t e2) {
	sm_event *tmp = pq->heap[e1];
	pq->heap[e1] = pq->heap[e2];
	pq->heap[e2] = tmp;
}
	
static void swim(sm_pqueue * pq, size_t e){
	while(e > 0 && less(pq, e/2, e))	{
		exch(pq, e, e/2);
		e = e/2;
	}
}

static void sink(sm_pqueue * pq, size_t e){
	while(2*e < pq->size) {
		size_t tmp = 2*e;
		if(tmp < pq->size-1 && less(pq, tmp, tmp+1)) 
			tmp++;
		if(!less(pq, e, tmp))
			break;
		exch(pq, e, tmp);
		e = tmp;
	}
}

static int enqueue(sm_event *e, sm_pqueue *pq) {
	if(pq->capacity - pq->size <= 0)
		return EXIT_FAILURE;
	pq->heap[pq->size] = e;
	swim(pq, pq->size++);
	return EXIT_SUCCESS;
}

static sm_event *dequeue(sm_pqueue *pq) {
	if(pq->size <= 0)
		return NULL;
    sm_event * e = pq->heap[0];
	pq->heap[0] = pq->heap[--pq->size];
	pq->heap[pq->size] = NULL;
	sink(pq, 0);
    return e;
}

// Public methods ///////////////////////////////////////////////////////////////

sm_pqueue *sm_pqueue_create(size_t capacity, bool synchronized) {
    sm_pqueue *pq;
    if((pq = malloc(sizeof(sm_pqueue))) == NULL) {
        REPORT(ERROR, "malloc()");
        return NULL;
    }
	pq->capacity = capacity;
	pq->size = 0;
	pq->ctl->synchronized = synchronized;
	if((pq->heap = calloc(capacity, sizeof(sm_event *))) == NULL) {
        REPORT(ERROR, "calloc()");
        return NULL;
    }
	if(pq->synchronized){
    	pthread_mutexattr_t attr;
    	if(pthread_mutexattr_init(&attr) != EXIT_SUCCESS) {
        	REPORT(ERROR, "pthread_mutexattr_init()");
        	sm_pqueue_free(pq);
        	return NULL;
    	}
    	if(pthread_mutexattr_settype(&attr, TL_MUTEX_TYPE) != EXIT_SUCCESS){
        	REPORT(ERROR, "pthread_mutexattr_settype()");
        	sm_pqueue_free(pq);
        	return NULL;
    	}    
    	if(pthread_mutex_init(&(pq->lock), &attr) != EXIT_SUCCESS) {
        	REPORT(ERROR, "pthread_mutex_init()");
        	sm_pqueue_free(pq);
        	return NULL;
    	}
    	if(pthread_cond_init(&(pq->empty),NULL) != EXIT_SUCCESS) {
        	REPORT(ERROR, "pthread_cond_init()");
        	pthread_cond_signal(&pq->empty);
        	sm_pqueue_free(pq);
        	return NULL;
    	}
	}	
    return pq;
}

void sm_pqueue_free(sm_pqueue *pq) {
	for (size_t ep = 0; ep < pq->size; ep++)
		sm_event_free(pq->heap[ep]);
	free(pq->heap);
	if(pq->synchronized){
    	pthread_mutex_destroy(&pq->lock);
    	pthread_cond_destroy(&pq->empty);
    	free(pq);
	}
}

size_t sm_pqueue_size(sm_pqueue * pq) {
    return pq->size;
}

sm_event * sm_pqueue_top(const sm_pqueue * pq) {
    return pq->size == 0 ? NULL : pq->heap[0];
}

int sm_pqueue_enqueue(sm_event * e, sm_pqueue * pq){
	if(pq->synchronized){
    	if(pthread_mutex_lock(&(pq->lock)) != EXIT_SUCCESS) {
        	REPORT(ERROR, "pthread_mutex_lock()");
        	return EXIT_FAILURE;
   		}
	}
    if(enqueue(e, pq) != EXIT_SUCCESS) {
       	REPORT(ERROR, "enqueue()");
        return EXIT_FAILURE;
    }
	if(pq->synchronized){
    	if(pthread_cond_signal(&(pq->empty)) != EXIT_SUCCESS) {
       		REPORT(ERROR, "pthread_cond_signal()");
        	return EXIT_FAILURE;
    	}
    	if(pthread_mutex_unlock(&(pq->lock)) != EXIT_SUCCESS) {
        	REPORT(ERROR, "pthread_mutex_unlock()");
        	return EXIT_FAILURE;
    	}
	}
    return EXIT_SUCCESS;
}

sm_event *sm_pqueue_dequeue(sm_pqueue * pq) {
	sm_event * e;
	if(pq->synchronized){
    	if(pthread_mutex_lock(&(pq->lock)) != EXIT_SUCCESS) {
        	REPORT(ERROR, "pthread_mutex_lock()");
        	return NULL;
    	}
    	while((e = dequeue(pq)) == NULL) {
        	if (pthread_cond_wait(&(pq->empty), &(pq->lock)) != EXIT_SUCCESS) {
            	REPORT(ERROR, "pthread_cond_wait()");
            	return NULL;
        	}
    	}
		if(pthread_mutex_unlock(&(pq->lock)) != EXIT_SUCCESS) {
        	REPORT(ERROR, "pthread_mutex_unlock()");
			return NULL;
    	}
	}
	else
		e = dequeue(pq);
    return e;
}