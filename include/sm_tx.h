/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Thread-worker descriptor class
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#ifndef SM_TX_H
#define SM_TX_H

#include "sm_state.h"
#include "sm_directory.h"
#include "sm_exec.h"
#include "sm_apply.h"

//#define SM_TX_STACK_POINTER(tx) (sm_state **)((char *)(tx)->data + (tx)->data_size)
#define SM_TX_STACK_POINTER(tx) (sm_state **)((char *)(tx)->data + (tx)->data_size - sizeof(sm_state **))

struct sm_state;
typedef struct sm_tx
{
    sm_exec *exec;
    sm_queue2 **input_queue;
    struct sm_state *state;
    struct sm_state *default_state;
    void *data;
    size_t data_size;
    size_t data_block_size;
    bool synchronized;
} sm_tx;

// Public methods
sm_tx *sm_tx_create(sm_exec *exec,
                    sm_fsm **f,
                    size_t size,
                    size_t state_size,
                    sm_queue2 **q,
                    bool synchronized);
void sm_tx_destroy(sm_tx **tx);

// Thread-worker app prototype
void *sm_tx_runner(void *arg);

// State stack
int sm_tx_push_state(sm_tx *tx, sm_state **s);
int sm_tx_pop_state(sm_tx *tx);
bool sm_tx_stack_empty(sm_tx *tx);
int sm_tx_to_string(sm_tx *tx, char *buffer);

#endif // SM_TX_H