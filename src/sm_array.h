/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Array class
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#ifndef SM_ARRAY_H
#define SM_ARRAY_H

#include "sm_state.h"
#include "sm_hash.h"

typedef struct __attribute__((aligned(SM_WORD))) sm_array
{
    sm_state **table;
    uint32_t (*hash_function)(const void *key, size_t key_length, uint32_t initval);
    uint32_t array_size;
    uint32_t mask;
    sm_state *queue_head;
    sm_state *queue_tail;
    uint32_t queue_size;
    uint32_t synchronized;
    pthread_mutex_t lock;
    pthread_cond_t empty;
} sm_array;

/*
State flags:
E - Event trace (linked event(s)) flag
T - Tx object address
H - Handle address flag
K - Hash key allocation strategy
*/
sm_array *sm_array_create(size_t key_length,
                          size_t queue_size,
                          size_t state_size,
                          sm_fsm **fsm,
                          bool synchronized, bool E, bool T, bool H, bool K);
void sm_array_destroy(sm_array **a);
#define SM_ARRAY_DESTROY(A) sm_array_destroy((&(A)))
#define SM_ARRAY_QUEUE_TOP(A) (SM_STATE_NEXT((A)->queue_head))
#define SM_ARRAY_QUEUE_SIZE(A) ((A)->queue_size)
//sm_state *sm_array_find_state(sm_array *a, const void *key, size_t key_length);
sm_state *sm_array_get_state(sm_array *a, const void *key, size_t key_length);
int sm_array_release_state(sm_state **s);
#define SM_ARRAY_RELEASE_STATE(S) sm_array_release_state((&(S)))
int sm_array_park_state(sm_state **s); 
//#define SM_ARRAY_PARK_STATE(S) {if((S)->ctl.T) {SM_STATE_TX((S)) = NULL;}(S)=NULL;}
#define SM_ARRAY_PARK_STATE(S) sm_array_park_state((&(S)))
int sm_array_to_string(sm_array *a, char *buffer);

#endif //SM_ARRAY_H


