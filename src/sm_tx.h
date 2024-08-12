/* SM.EXEC
   SM thread descriptor
   anton.bondarenko@gmail.com */

#ifndef SM_TX_H
#define SM_TX_H

#include <stdlib.h>
#include "sm_sys.h" 
//#include "sm_queue2.h"
#include "sm_state.h"
#include "sm_directory.h"
#include "sm_logger.h"
//#include "sm_exec.h"

/*
#define SM_TX_STACK_POINTER(tx) (sm_state **)((char *)(tx)->data + (tx)->data_size ) 

struct sm_state;
typedef struct sm_tx {
    sm_exec *exec;
    sm_queue2 **input_queue;
    struct sm_state *state;
    struct sm_state *default_state;
    void* data;
    bool is_synchronized;
    size_t data_size;
    size_t data_block_size;
} sm_tx;

// Public methods
sm_tx *sm_tx_create(sm_exec *exec,
                    sm_fsm **f, 
                    size_t size,
                    size_t state_size,
                    sm_queue2 **q,
                    bool sync);
void sm_tx_free(sm_tx *tx);


// Thread-worker app prototype
void *sm_tx_runner(void *arg);


// State stack
int sm_tx_push_state(sm_tx * tx, sm_state *s);
int sm_tx_pop_state(sm_tx * tx);
bool sm_tx_stack_empty(sm_tx *tx);
*/
#endif //SM_TX_H 