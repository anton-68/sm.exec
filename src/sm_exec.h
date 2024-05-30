/* SM.EXEC
   FSM executor descriptor
   anton.bondarenko@gmail.com */

#ifndef SM_EXEC_H
#define SM_EXEC_H

#include <stdlib.h>
#include "sm_sys.h"
#include "sm_queue2.h"
#include "sm_state.h"
#include "sm_directory.h"
#include "../oam/logger.h"
      

// Executor
struct sm_tx;
typedef struct sm_exec {
	struct sm_tx *master_tx;
	sm_directory *dir;
	size_t data_size;
	void *data;
} sm_exec;

// Public methods
sm_exec *sm_exec_create(size_t s, sm_directory *dir, struct sm_tx *tx);
void sm_exec_free(sm_exec *exec);


// Thread-worker descriptor
struct sm_state;
typedef struct sm_tx {
	sm_exec *exec;
	sm_queue2 *input_queue_ptr;
	sm_queue2 **input_queue;
	bool is_synchronized;
	struct sm_state *state;
	size_t data_size;
	void* data;
} sm_tx;

// Public methods
sm_tx *sm_tx_create(sm_exec *exec,
					sm_fsm **f, 
					size_t size,
					size_t state_size,
					sm_queue2 **q,
					bool sync);
void sm_tx_free(sm_tx *tx);

// Thread-worker app
void *sm_tx_runner(void *arg);

#endif //SM_EXEC_H 
