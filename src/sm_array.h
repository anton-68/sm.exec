/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Array class
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#ifndef SM_ARRAY_H
#define SM_ARRAY_H

#include "sm_state.h"
#include "../lib/bj_hash/bj_hash.h"

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

sm_array *sm_array_create(size_t key_length, size_t state_size, sm_fsm **fsm, bool S, bool C, bool E, bool H);

void sm_array_free(sm_array **a);
sm_state *sm_array_find_state(sm_array *a, const void *key, size_t key_length);
sm_state *sm_array_get_state(sm_array *a, const void *key, size_t key_length);
void sm_array_release_state(sm_array *a, sm_state **s);  
void sm_array_park_state(sm_array *a, sm_state **s);  

#endif //SM_ARRAY_H


