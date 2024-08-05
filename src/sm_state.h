/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
State class
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#ifndef SM_STATE_H
#define SM_STATE_H

#include "sm_hash.h"
#include "sm_event.h"
#include "sm_fsm.h"

/*
sm_state - LP64
===============

 0         1          2         3          4         5          6
 0123456789012345 6789012345678901 23456789012345678901234567 890123
+----------------+----------------+--------------------------+------+ <-----
|   service_id   |    state_id    |           size           |flags | fixed
+----------------+----------------+--------------------------+------+ part
|                                fsm                                |
+-------------------------------------------------------------------+ <-----
:                    home array ([D]epot) address                   : o p
+ - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - + p a
:                          key string addr                          : t r
+ - - - - - - - - - - - - - - - - + - - - - - - - - - - - - - - - - + i t
:            key length           :              key hash           : o
+ - - - - - - - - - - - - - - - - + - - - - - - - - - - - - - - - - + n
:                                next                               : a
+-------------------------------------------------------------------+ l
:                     [E]vent stack head address                    :
+-------------------------------------------------------------------+
:                        [T]x object address                        :
+-------------------------------------------------------------------+
:                   [H]andle (Lua wrapper) address                  :
+-------------------------------------------------------------------+ <-----

sm_state - ILP32
================

 0         1         2          3
 0123456789012345 6789012345 678901
+----------------+-----------------+ <-----
|   service_id   |     state_id    | fixed
+----------------+----------+------+ part
|          size             |flags |
+---------------------------+------+
|               fsm                |
+----------------------------------+ <-----
:   home array ([D]epot) address   : o p
+- - - - - - - - - - - - - - - - - + p a
:         key string addr          : t r
+- - - - - - - - - - - - - - - - - + i t
:            key length            : o
+- - - - - - - - - - - - - - - - - + n
:             key hash             : a
+- - - - - - - - - - - - - - - - - + l
:               next               :
+----------------------------------+
:        [E]vent stack head        :
+----------------------------------+
:        [T]x object address       :
+----------------------------------+
:  [H]andle (Lua wrapper) address  :
+----------------------------------+ <-----

Flags
=====
D - Depot address flag
E - Event trace (linked event(s)) flag
T - Tx object address
H - Handle address flag
K - Hash key allocation strategy 
*/

#define SM_STATE_HASH_DST(S) (((S)->ctl.K)?SM_STATE_DATA((S)):NULL)
// #define SM_STATE_HASH_DST(S) NULL

typedef struct __attribute__((aligned(SM_WORD))) sm_state
{
    uint16_t service_id;
    uint16_t state_id;
    union
    {
        uint32_t type;
        struct
        {
            uint32_t size : 26; // in 64-bit words
            uint32_t D    :  1;
            uint32_t E    :  1;
            uint32_t T    :  1; 
            uint32_t H    :  1;
            uint32_t K    :  1;
            uint32_t      :  0; // reserved
        } ctl;
    };
    sm_fsm **fsm;
} sm_state;

struct sm_array;
#define SM_STATE_DEPOT(S) \
    (*(struct sm_array **)((char *)(S) + sizeof(sm_state)))

/*
#define SM_STATE_DEPOT(S) \
    (*(struct sm_array **)((char *)(S) + SM_WORD + 8))
*/

#define SM_STATE_HASH_KEY(S) \
    ((sm_hash_key *)((char *)(S) + sizeof(sm_state) + SM_WORD))

/*
#define SM_STATE_KEY_STRING(S) \
    (*(char **)((char *)(S) + SM_WORD * (1 + (S)->ctl.D) + 8))

#define SM_STATE_KEY_LENGTH(S) \
    (*(uint32_t *)((char *)(S) + SM_WORD * (2 + (S)->ctl.D) + 8))

#define SM_STATE_KEY_HASH(S) \
    (*(uint32_t *)((char *)(S) + SM_WORD * (2 + (S)->ctl.D) + 12))
*/

#define SM_STATE_NEXT(S) \
    (*(sm_state**)((char *)(S) + sizeof(sm_state) + SM_WORD + sizeof(sm_hash_key)))

/*
#define SM_STATE_NEXT(S) \
    (*(struct sm_state **)((char *)(S) + SM_WORD * (1 + (S)->ctl.D + (S)->ctl.K) + 8 * (1 + (S)->ctl.K)))
*/

#define SM_STATE_EVENT_TRACE(S) \
    (*(sm_event **)((char *)(S) + sizeof(sm_state) + (SM_WORD * 2 + sizeof(sm_hash_key)) * (S)->ctl.D))

/*
#define SM_STATE_EVENT_TRACE(S) \
    (*(sm_event **)((char *)(S) + SM_WORD * (1 + (S)->ctl.D + (S)->ctl.K + (S)->ctl.C) + 8 * (1 + (S)->ctl.K)))
*/

#define SM_STATE_TX(S) \
    (*(void **)((char *)(S) + sizeof(sm_state) + (SM_WORD * 2 + sizeof(sm_hash_key)) * (S)->ctl.D + SM_WORD * (S)->ctl.E))

#define SM_STATE_HANDLE(S) \
    (*(void **)((char *)(S) + sizeof(sm_state) + (SM_WORD * 2 + sizeof(sm_hash_key)) * (S)->ctl.D + SM_WORD * ((S)->ctl.E + (S)->ctl.T)))

/*
#define SM_STATE_HANDLE(S) \
    (*(void **)((char *)(S) + SM_WORD * (2 + (S)->ctl.D + (S)->ctl.K + (S)->ctl.C + (S)->ctl.E) + 8 * (1 + (S)->ctl.K)))
*/

#define SM_STATE_DATA(S) \
    ((void *)((char *)(S) + sizeof(sm_state) + (SM_WORD * 2 + sizeof(sm_hash_key)) * (S)->ctl.D + SM_WORD * ((S)->ctl.E + (S)->ctl.T + (S)->ctl.H)))

#define SM_STATE_DATA_K(S) \
    ((void *)((char *)(S) + sizeof(sm_state) + (SM_WORD * 2 + sizeof(sm_hash_key)) * (S)->ctl.D + SM_WORD * ((S)->ctl.E + (S)->ctl.T + (S)->ctl.H) + SM_STATE_HASH_KEY(S)->length * (S)->ctl.D * (S)->ctl.K))

/*
#define SM_STATE_DATA(S) \
    ((void *)((char *)(S) + SM_WORD * (1 + (S)->ctl.D + (S)->ctl.K + 2 * (S)->ctl.C + (S)->ctl.E + (S)->ctl.H) + 8 * (1 + (S)->ctl.K)))
*/

#define SM_STATE_DATA_SIZE(S) ((uint32_t)((S)->ctl.size) << 6)

sm_state *sm_state_create(sm_fsm **f, uint32_t size, struct sm_array *depot, bool E, bool T, bool H, bool K);
void sm_state_destroy(sm_state **s);
#define SM_STATE_DESTROY(S) sm_state_destroy((&(S)))
void sm_state_erase(sm_state *s); 
void sm_state_dispose(sm_state **s);
#define SM_STATE_DISPOSE(S) sm_state_dispose((&(S)))
void sm_state_push_event(sm_state *s, sm_event **e);
#define SM_STATE_PUSH_EVENT(S, E) sm_state_push_event((S), (&(E)))
sm_event *sm_state_pop_event(sm_state *s);
int sm_state_to_string(sm_state *s, char *buffer);

#endif //SM_STATE_H 
