/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Event class
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#ifndef SM_EVENT_H
#define SM_EVENT_H

#include "sm_logger.h"
#include "sm_hash.h"

/*
sm_event - LP64
===============

 0         1          2         3          4         5          6
 0123456789012345 6789012345678901 23456789012345678901234567 890123
+----------------+----------------+--------------------------+------+ <-----
|   service_id   |    event_id    |           size           |flags | fixed
+----------------+----------------+--------------------------+------+ part
|                               next                                |
+-------------------------------------------------------------------+ <-----
:                    [Q]ueue (home depot) address                   : o p
+-------------------------------------------------------------------+ p a
:                         [K]ey string addr                         : t r
+ - - - - - - - - - - - - - - - - + - - - - - - - - - - - - - - - - + i t
:          [K]ey length           :            [K]ey hash           : o
+---------------------------------+---------------------------------+ n
:                           [P]riority[0]                           : a
+ - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - + l
:                           [P]riority[1]                           :
+-------------------------------------------------------------------+
:                   [H]andle (Lua wrapper) address                  :
+-------------------------------------------------------------------+ <-----
NOTE: The priority field is also used for timestamp related operations


sm_event - ILP32
================

 0         1         2          3
 0123456789012345 6789012345 678901
+----------------+-----------------+ <-----
|   service_id   |     event_id    | fixed
+----------------+----------+------+ part
|          size             |flags |
+---------------------------+------+
|               next               |
+----------------------------------+ <-----
:   [Q]ueue (home depot) address   : o p
+----------------------------------+ p a
:        [K]ey string addr         : t r
+ - - - - - - - - - - - - - - - - -+ i t
:          [K]ey length            : o
+ - - - - - - - - - - - - - - - - -+ n
:           [K]ey hash             : a
+----------------------------------+ l
:          [P]riority[0]           :
+ - - - - - - - - - - - - - - - - -+
:          [P]riority[1]           :
+----------------------------------+
:  [H]andle (Lua wrapper) address  :
+----------------------------------+ <-----
NOTE: The priority field is also used for timestamp related operations

Flags
=====
Q - Queue (home depot) address flag
K - Key string / length / hash flag
P - Priority / timestamp flag
H - Handle address flag
L - Linked flag
D - Disposable flag
*/

typedef struct __attribute__((aligned(SM_WORD))) sm_event
{
    uint16_t service_id;
    uint16_t event_id;
    union {
        uint32_t type;
        struct
        {
            uint32_t size : 26; // in 64-bit words
            uint32_t Q    :  1;
            uint32_t K    :  1;
            uint32_t P    :  1;
            uint32_t H    :  1;
            uint32_t D    :  1;
            uint32_t L    :  1;
        } ctl;    
    };
    struct sm_event *next;
} sm_event;

struct sm_queue;
#define SM_EVENT_DEPOT(E) \
    (*(struct sm_queue **)((char *)(E) + SM_WORD + 8))

#define SM_EVENT_HASH_KEY(E) \
    ((sm_hash_key *)((char *)(E) + sizeof(sm_event) + SM_WORD * !!(E)->ctl.Q))
/*
#define SM_EVENT_KEY_STRING(E) \
    (*(char **)((char *)(E) + SM_WORD * (1 + (E)->ctl.Q) + 8))

#define SM_EVENT_KEY_LENGTH(E) \
    (*(uint32_t *)((char *)(E) + SM_WORD * (2 + (E)->ctl.Q) + 8))

#define SM_EVENT_KEY_HASH(E) \
    (*(uint32_t *)((char *)(E) + SM_WORD * (2 + (E)->ctl.Q) + 12))
*/

#define SM_EVENT_PRIORITY(E) \
    ((unsigned long *)((char *)(E) + sizeof(sm_event) + sizeof(sm_hash_key) + SM_WORD * !!(E)->ctl.Q))
    
    //SM_WORD * (1 + (E)->ctl.Q + (E)->ctl.K) + 8 * (1 + (E)->ctl.K)))
/*
#define SM_EVENT_PRIORITY_0(E) \
    (*(unsigned long *)((char *)(E) + SM_WORD * (1 + (E)->ctl.Q + (E)->ctl.K) + 8 * (1 + (E)->ctl.K)))

#define SM_EVENT_PRIORITY_1(E) \
    (*(unsigned long *)((char *)(E) + SM_WORD * (2 + (E)->ctl.Q + (E)->ctl.K) + 8 * (1 + (E)->ctl.K)))
*/
#define SM_EVENT_HANDLE(E) \
    (*(void **)((char *)(E) + sizeof(sm_event) + sizeof(sm_hash_key) + SM_WORD * (!!(E)->ctl.Q + !!(E)->ctl.P * 2)))
/*
#define SM_EVENT_HANDLE(E) \
    (*(void **)((char *)(E) + SM_WORD * (1 + (E)->ctl.Q + (E)->ctl.K + 2 * (E)->ctl.P) + 8 * (1 + (E)->ctl.K)))
*/
#define SM_EVENT_DATA(E) \
    ((void *)((char *)(E) + sizeof(sm_event) + sizeof(sm_hash_key) + SM_WORD * (!!(E)->ctl.Q + !!(E)->ctl.P * 2 + !!(E)->ctl.H)))
/*
#define SM_EVENT_DATA(E) \
    ((void *)((char *)(E) + SM_WORD * (1 + (E)->ctl.Q + (E)->ctl.K + 2 * (E)->ctl.P + (E)->ctl.H) + 8 * (1 + (E)->ctl.K)))
*/
#define SM_EVENT_DATA_SIZE(E) (size_t)((uint32_t)((E)->ctl.size) << 6)

sm_event *sm_event_create(uint32_t size, bool Q, bool K, bool P, bool H);
void sm_event_destroy(sm_event **e);
#define SM_EVENT_DESTROY(E) sm_event_destroy((&(E)))
void sm_event_erase(sm_event *e);
void sm_event_dispose(sm_event **e);
#define SM_EVENT_DISPOSE(E) sm_event_dispose((&(E)))
size_t sm_event_sizeof(const sm_event *e);
sm_event *sm_event_clone(sm_event *e);
sm_event *sm_event_chain_end(sm_event *e);
void sm_event_chain(sm_event *e0, sm_event **e1);
#define SM_EVENT_CHAIN(E0, E1) sm_event_chain((E0), (&(E1)))
void sm_event_unchain(sm_event *e);
int sm_event_to_string(const sm_event *e, char *buffer);

#endif //SM_EVENT_H
