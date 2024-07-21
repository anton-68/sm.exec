/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
State class
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#ifndef SM_STATE_H
#define SM_STATE_H

#include "sm_event.h"
#include "sm_fsm.h"

/*
sm_state - LP64
===============

 0         1          2         3          4         5          6
 0123456789012345 6789012345678901 23456789012345678901234567 890123
+----------------+----------------+--------------------------+------+ <-----
|   service_id   |    event_id    |           size           |flags | fixed
+----------------+----------------+--------------------------+------+ part
|                                fsm                                |
+-------------------------------------------------------------------+ <-----
:                    home array ([D]epot) address                   : o p
+-------------------------------------------------------------------+ p a
:                         [K]ey string addr                         : t r
+     -     -     -     -     -   + -     -     -     -     -     - + i t
:          [K]ey length           :            [K]ey hash           : o
+---------------------------------+---------------------------------+ n
:                           [T]x address                            : a
+-------------------------------------------------------------------+ l
:                        [E]vent stack head                         :
+-------------------------------------------------------------------+
:                   [H]andle (Lua wrapper) address                  :
+-------------------------------------------------------------------+ <-----

sm_state - ILP32
================

 0         1         2          3
 0123456789012345 6789012345 678901
+----------------+-----------------+ <-----
|   service_id   |     event_id    | fixed
+----------------+----------+------+ part
|          size             |flags |
+---------------------------+------+
|               fsm                |
+----------------------------------+ <-----
:   home array ([D]epot) address   : o p
+----------------------------------+ p a
:        [K]ey string addr         : t r
+    -     -     -     -     -     + i t
:           [K]ey length           : o
+    -     -     -     -     -     + n
:            [K]ey hash            : a
+----------------------------------+ l
:           [T]x address           :
+----------------------------------+
:        [E]vent stack head        :
+----------------------------------+
:  [H]andle (Lua wrapper) address  :
+----------------------------------+ <-----

Flags
=====
D - Depot address flag
K - Key-Length-Hash flag
T - Tx address flag
E - Event trace (linked event(s)) flag
H - Handle address flag
*/

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
            uint32_t K    :  1;
            uint32_t T    :  1;
            uint32_t E    :  1; 
            uint32_t H    :  1;
            uint32_t      :  0; // reserved
        } ctl;
    };
    sm_fsm **fsm;
} sm_state;

struct sm_array;
#define SM_STATE_DEPOT(S) \
    (*(struct sm_array **)((char *)(S) + SM_WORD + 8))

#define SM_STATE_KEY_STRING(S) \
    (*(char **)((char *)(S) + SM_WORD * (1 + (S)->ctl.D) + 8))

#define SM_STATE_KEY_LENGTH(S) \
    (*(uint32_t *)((char *)(S) + SM_WORD * (2 + (S)->ctl.D) + 8))

#define SM_STATE_KEY_HASH(S) \
    (*(uint32_t *)((char *)(S) + SM_WORD * (2 + (S)->ctl.D) + 12))

struct sm_tx;
#define SM_STATE_TX(S) \
    (*(struct sm_tx **)((char *)(S) + SM_WORD * (1 + (S)->ctl.D + (S)->ctl.K) + 8 * (1 + (S)->ctl.K)))

#define SM_STATE_EVENT_TRACE(S) \
    (*(sm_event **)((char *)(S) + SM_WORD * (1 + (S)->ctl.D + (S)->ctl.K + (S)->ctl.T) + 8 * (1 + (S)->ctl.K)))

#define SM_STATE_HANDLE(S) \
    (*(void **)((char *)(S) + SM_WORD * (2 + (S)->ctl.D + (S)->ctl.K + (S)->ctl.T + (S)->ctl.E) + 8 * (1 + (S)->ctl.K)))

#define SM_STATE_DATA(S) \
    ((void *)((char *)(S) + SM_WORD * (1 + (S)->ctl.D + (S)->ctl.K + 2 * (S)->ctl.T + (S)->ctl.E + (S)->ctl.H) + 8 * (1 + (S)->ctl.K)))

#define SM_STATE_DATA_SIZE(S) ((uint32_t)((S)->ctl.size) << 6)

sm_state *sm_state_create(sm_fsm **f, uint32_t size, bool D, bool K, bool T, bool E, bool H);
void sm_state_destroy(sm_state *s);
void sm_state_erase(sm_state *s); 
void sm_state_dispose(sm_state *s);
void sm_state_push_event(sm_state *s, sm_event *e);
sm_event *sm_state_pop_event(sm_state *s);
int sm_state_to_string(sm_state *s, char *buffer);

#endif //SM_STATE_H 
