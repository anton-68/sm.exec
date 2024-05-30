/* SM.EXEC
   Process and thread-worker descriptors
   anton.bondarenko@gmail.com */

#ifndef SM_THREAD_H
#define SM_THREAD_H

#include <stdlib.h>
#include "sm_sys.h"
#include "sm_queue.h"
#include "sm_state.h"
#include "sm_fsm.h"
#include "sm_app.h"
#include "../oam/logger.h"
      
// process
struct sm_thread_desc;
typedef struct sm_process_desc {
	struct sm_thread_desc *master_thread;
	sm_app_table *app_table;
	sm_fsm_table *fsm_table;
	size_t context_size;
	void *context;
} sm_process_desc;

// Public methods
sm_process_desc *sm_process_create(size_t s, sm_app_table *a, sm_fsm_table *f);
void *sm_process_free(sm_process_desc *p);

// thread
typedef struct sm_thread_desc {
	sm_process_desc *process;
	sm_queue *input_queue;
	sm_state *state;
	size_t context_size;
	void* context;
} sm_thread_desc;

// Public methods
sm_thread_desc *sm_thread_create(sm_fsm **f, 
								 size_t s,
								 sm_queue *q,
								 size_t qs,
								 size_t es,
						    	 sm_process_desc *p,
								 bool sync);
void *sm_thread_free(sm_thread_desc *p);


// Thread-runner app
void sm_thread_runner(void *arg);

#endif //SM_THREAD_H 