/* SM.EXEC
   Priority queues
   anton.bondarenko@gmail.com */

#include <stdlib.h>	
#include "sm_logger.h"
#include "sm_pqueue.h"

static bool less(sm_pqueue * pq, size_t e1, size_t e2) {
	for (size_t stage = 0; stage < SM_NUM_OF_PRIORITY_STAGES; stage++) {
		if(sm_event_priority_ptr(pq->heap[e1])[stage] < sm_event_priority_ptr(pq->heap[e2])[stage])
			return true;
		else {
			if(sm_event_priority_ptr(pq->heap[e1])[stage] > sm_event_priority_ptr(pq->heap[e2])[stage])
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

static int enqueue(sm_pqueue *pq, sm_event *e) {
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

sm_pqueue *sm_pqueue_create(size_t capacity, bool synchronized) {
    sm_pqueue *pq;
    if((pq = malloc(sizeof(sm_pqueue))) == NULL) {
        SM_LOG(SM_CORE, SM_LOG_ERR, "Failed to allocate pqueue header");
        return NULL;
    }
	pq->capacity = capacity;
	pq->size = 0;
	pq->ctl.synchronized = synchronized;
	if((pq->heap = calloc(capacity, sizeof(sm_event *))) == NULL) {
        SM_LOG(SM_CORE, SM_LOG_ERR, "Failed to allocate pqueue heap");
        return NULL;
    }
	SM_LOCK_INIT(pq, sm_pqueue_free);
    return pq;
}

void sm_pqueue_free(sm_pqueue *pq) {
	for (size_t ep = 0; ep < pq->size; ep++)
		sm_event_free(pq->heap[ep]);
	free(pq->heap);
	SM_LOCK_DESTROY(pq);
	free(pq);
}

size_t sm_pqueue_size(sm_pqueue * pq) {
    return pq->size;
}

sm_event * sm_pqueue_top(const sm_pqueue * pq) {
    return pq->size == 0 ? NULL : pq->heap[0];
}

int sm_pqueue_enqueue(sm_pqueue *pq, sm_event *e) {
	if(sm_event_priority_ptr(e) == NULL) {
		SM_LOG(SM_CORE, SM_LOG_DEBUG, "Attempting to enqueue event without priority field in pqueue");
		return EXIT_FAILURE;
	}
	SM_LOCK(pq);
    int res = enqueue(pq, e);
	SM_SIGNAL_UNLOCK(pq);
    return res;
}	

sm_event * sm_pqueue_dequeue(sm_pqueue * pq) {
	sm_event * e;
	SM_LOCK_WAIT(pq, e = dequeue(pq)); 
    return e;
}
