/* SM.EXEC
   SM executor descriptor
   anton.bondarenko@gmail.com */

#ifndef SM_EXEC_H
#define SM_EXEC_H

#include <stdlib.h>
#include "sm_sys.h"
#include "sm_queue.h"
#include "sm_state.h"
#include "sm_fsm.h"
#include "sm_app.h"
#include "sm_logger.h"
      
// process
struct sm_thread_desc;
typedef struct sm_exec {
	struct sm_x *master_tx;
	sm_directory *dir;
	size_t data_size;
	void *data;
} sm_exec;

// Public methods
sm_exec *sm_process_create(size_t s, sm_app_table *a, sm_fsm_table *f);
void sm_process_free(sm_exec *p);

// Thread-worker descriptor
typedef struct sm_tx {
	sm_exec *exec;
	sm_queue2 *input_queue;
	sm_state *state;
	size_t data_size;
	void* data;
} sm_tx;

// Public methods
sm_thread_desc *sm_thread_create(sm_fsm **f, 
								 size_t s,
								 sm_queue *q,
								 size_t qs,
								 size_t es,
						    	 sm_process_desc *p,
								 bool sync);
void sm_thread_free(sm_thread_desc *p);

// Thread-worker app
void sm_tx_runner(void *arg);

#endif //SM_EXEC_H 
