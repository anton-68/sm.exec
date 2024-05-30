/* SM.EXEC
   FSM executor descriptor
   anton.bondarenko@gmail.com */

#include <stdlib.h>			// malloc(), free(), NULL, size_t, 
#include <stdio.h>
#include <pthread.h>

#include "../oam/logger.h"
#include "sm_fsm.h"
#include "sm_event.h"
#include "sm_state.h"
#include "sm_exec.h"
#include "sm_memory.h"

// Executor

// Public methods

sm_exec *sm_exec_create(size_t size, sm_directory *dir, sm_tx *tx) {
	sm_exec *exec;
	if((exec = malloc(sizeof(sm_exec))) == NULL) {
        REPORT(ERROR, "malloc()");
        return NULL;
    }
	if(SM_MEMORY_MANAGER)
		exec->data_size = sm_memory_size_align(size, sizeof(sm_chunk));
	else
		exec->data_size = size;
	if((exec->data = malloc(exec->data_size)) == NULL) {
		free(exec);
        REPORT(ERROR, "malloc()");
        return NULL;
    }
	exec->master_tx = tx;
	exec->dir = dir;
	return exec;
}

void sm_exec_free(sm_exec *exec) {
	if(exec->master_tx != NULL)
		sm_tx_free(exec->master_tx);
	sm_directory_free(exec->dir);			  
	free(exec->data);
	free(exec);
}


// Thread-worker descriptor

// Public methods
sm_tx *sm_tx_create(sm_exec *exec,
					sm_fsm **f, 
					size_t size,
					size_t state_size,
					sm_queue2 **q,
					bool sync){
	sm_tx *td;
	if((td = malloc(sizeof(sm_tx))) == NULL) {
        REPORT(ERROR, "malloc()");
        return NULL;
    }
	td->exec = exec;
	if(q == NULL) {
		td->input_queue_ptr = sm_queue2_create();
		td->input_queue = &td->input_queue_ptr;
	}
	else {
		td->input_queue_ptr = NULL;
		td->input_queue = q;
	}
	td->is_synchronized = sync;
	if((td->state = sm_state_create(f, state_size)) == NULL) {
		if(td->input_queue_ptr)
			sm_queue2_free(td->input_queue_ptr);
		free(td);
        REPORT(ERROR, "sm_queue_create()");
        return NULL;
	}
	td->state->exec = exec;
	if(SM_MEMORY_MANAGER)
		td->data_size = sm_memory_size_align(size, sizeof(sm_chunk));
	else
		td->data_size = size;
	if((td->data = malloc(size + sizeof(sm_state *))) == NULL) {
		if(td->input_queue_ptr)
			sm_queue2_free(td->input_queue_ptr);
		free(td->data);
		free(td);
        REPORT(ERROR, "malloc()");
        return NULL;
    }
	*SM_TX_STACK_POINTER(td) = NULL;
	return td;
}

void sm_tx_free(sm_tx *tx){
	//sm_queue2_free(*tx->input_queue);
	if(tx->input_queue_ptr)
			sm_queue2_free(tx->input_queue_ptr);
	sm_state_free(tx->state);
	free(tx->data);
	free(tx);
}

// Thread-runner app
void *sm_tx_runner(void *arg) {
	sm_tx *tx = (sm_tx *)arg;
	sm_event *event = NULL;
	if(tx->is_synchronized) {
		while(true) {
			while(tx->state->id != (*tx->state->fsm)->final) {
				event = sm_lock_dequeue2(*tx->input_queue);
				sm_apply_event(tx->state, event);
			}
			if(event->disposable)
				sm_event_park(event);
			tx->state->id = (*tx->state->fsm)->initial;	
		}
	}
	else {
		while((event = sm_dequeue2(*tx->input_queue)) != NULL) {
			sm_apply_event(tx->state, event);
			if(tx->state->id == (*tx->state->fsm)->final) {
				if(event->disposable)
					sm_event_park(event);
				tx->state->id = (*tx->state->fsm)->initial;
			}
		}
	}
	return 0;
}

// State stack push
int sm_tx_push_state(sm_tx * tx, sm_state * s){
	if(tx->data_size < sizeof(sm_state *))
		return EXIT_FAILURE;
	tx->data_size -= sizeof(sm_state *);
	*SM_TX_STACK_POINTER(tx) = tx->state;
	tx->state = s;	
	return EXIT_SUCCESS; 
}

// State pop
int sm_tx_pop_state(sm_tx * tx){
	tx->state = *SM_TX_STACK_POINTER(tx);
	tx->data_size += sizeof(sm_state *);	
	return EXIT_SUCCESS; 
}