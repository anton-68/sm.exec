/* SM.EXEC
   Process and thread-worker descriptors
   anton.bondarenko@gmail.com */

#include <stdlib.h>			// malloc(), free(), NULL, size_t, 
#include <stdio.h>
#include <pthread.h>

#include "sm_logger.h"
#include "sm_fsm.h"
#include "sm_event.h"
#include "sm_queue.h"
#include "sm_thread.h"

// process

// Public methods

sm_process_desc *sm_process_create(size_t s, sm_app_table *a, sm_fsm_table *f) {
	sm_process_desc *pd;
	if((pd = malloc(sizeof(sm_process_desc))) == NULL) {
        REPORT(ERROR, "malloc()");
        return NULL;
    }
	if((pd->context = malloc(s)) == NULL) {
		free(pd);
        REPORT(ERROR, "malloc()");
        return NULL;
    }
	pd->master_thread = NULL;
	pd->context_size = s;
	pd->app_table = a;
	pd->fsm_table = f;
	return pd;
}

void sm_process_free(sm_process_desc *pd) {
	if(pd->master_thread != NULL)
		sm_thread_free(pd->master_thread);
	sm_fsm_table_free(pd->fsm_table);
	sm_app_table_free(pd->app_table);				  
	free(pd->context);
	free(pd);
}

// thread

// Public methods
sm_thread_desc *sm_thread_create(sm_fsm **f, 
								 size_t s,
								 sm_queue *q,
								 size_t qs,
								 size_t es,
						    	 sm_process_desc *p,
								 bool sync){
	sm_thread_desc *td;
	if((td = malloc(sizeof(sm_thread_desc))) == NULL) {
        REPORT(ERROR, "malloc()");
        return NULL;
    }
	if((td->context = malloc(s)) == NULL) {
		free(td);
        REPORT(ERROR, "malloc()");
        return NULL;
    }
	td->process = p;
	if(q == NULL) {	
		if((td->input_queue = sm_queue_create(qs, es, sync)) == NULL) {
			free(td->context);
			free(td);
        	REPORT(ERROR, "sm_queue_create()");
        	return NULL;	
		}
	}
	else
		td->input_queue = q;
	td->process = p;
	td->state = sm_state_create(f, 16);
	td->state->process = p;
	return td;
}

void sm_thread_free(sm_thread_desc *td){
	sm_queue_free(td->input_queue);
	sm_state_free(td->state);
	free(td->context);
	free(td);
}

// Thread-runner app
void sm_thread_runner(void *arg) {
	sm_thread_desc *tx = (sm_thread_desc *)arg;
	sm_event *event = NULL;
	if(tx->input_queue->synchronized) {
		while(true) {
			event = sm_queue_dequeue(tx->input_queue);
			sm_apply_event(tx->state, event);
			if(event->disposable)
				sm_event_park(event);
			if(tx->state->id == *(tx->state->fsm)->final)
				tx->state = 
			tx->state->id = (*tx->state->fsm)->initial;	
		}
	}
	else {
		while((event = sm_queue_dequeue(tx->input_queue)) != NULL) {
			sm_apply_event(tx->state, event);
			if(tx->state->id == (*tx->state->fsm)->final) {
				if(event->disposable)
					sm_event_park(event);
				tx->state->id = (*tx->state->fsm)->initial;
			}
		}
	}
}
