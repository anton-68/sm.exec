/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Event class
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#ifndef SM_EVENT_H
#define SM_EVENT_H

#include "sm_sys.h"
#include "sm_logger.h"

/*
sm_event - LP64
===============
 0         1          2         3          4         5          6
 0123456789012345 6789012345678901 23456789012345678901234567 890123
+----------------+----------------+--------------------------+------+ <-----
|     app_id     |    event_id    |           size           |flags | fixed
+----------------+----------------+--------------------------+------+ part
|                               next                                |
+-------------------------------------------------------------------+ <-----
:                     home queue (depot) address                    : o p
+-------------------------------------------------------------------+ p a
:                           key string addr                         : t r
+     -     -     -     -     -   + -     -     -     -     -     - + i t
:           key length            :             key hash            : o
+---------------------------------+---------------------------------+ n
:                            priority[0]                            : a
+  -     -     -     -     -     -     -     -     -     -     -    + l
:                            priority[1]                            :
+-------------------------------------------------------------------+
:                    lua wrapper (handle) sddress                   :
+-------------------------------------------------------------------+ <-----
NOTE: The priority field is also used for timestamp related operations


sm_event - ILP32
================
 0         1         2          3
 0123456789012345 67890123 45678901
+----------------+-----------------+ <-----
|     app_id     |     event_id    | fixed
+----------------+--------+--------+ part
|          size           | flags  |
+-------------------------+--------+
|               next               |
+----------------------------------+ <-----
:    home queue (depot) address    : o p
+----------------------------------+ p a
:         key string addr          : t r
+    -     -     -     -     -     + i t
:            key length            : o
+    -     -     -     -     -     + n
:             key hash             : a
+----------------------------------+ l
:            priority[0]           :
+ -     -     -     -     -     -  +
:            priority[1]           :
+----------------------------------+
:   lua wrapper (handle) sddress   :
+----------------------------------+ <-----
NOTE: The priority field is also used for timestamp related operations


Flags
=====
Q - home queue address flag
K - key string / hash flag
P - priority / timestamp flag
H - handle address flag
L - linked flag
D - disposable flag
*/

struct sm_queue;
typedef struct __attribute__((aligned(SM_WORD))) sm_event
{
    uint16_t app_id;
    uint16_t event_id;
    union {
        uint32_t type;
        struct
        {
            uint32_t size : 26; // in 64-bit words
            uint32_t D    :  1;
            uint32_t L    :  1;
            uint32_t Q    :  1;
            uint32_t K    :  1;
            uint32_t P    :  1;
            uint32_t H    :  1;    
        } ctl;    
    };
    struct sm_event *next;
} sm_event;


// Accessors

#define SM_EVENT_DEPOT(E) \
    *(void**)((char *)(E) + SM_WORD + 8)

#define SM_EVENT_KEY_STRING(E) \
    *(char **)((char *)(E) + SM_WORD * (1 + (E)->ctl.Q) + 8)

#define SM_EVENT_KEY_LENGTH(E) \
    *(uint32_t *)((char *)(E) + SM_WORD * (2 + (E)->ctl.Q) + 8)

#define SM_EVENT_KEY_HASH(E) \
    *(uint32_t *)((char *)(E) + SM_WORD * (2 + (E)->ctl.Q) + 12)

#define SM_EVENT_PRIORITY_0(E) \
    *(unsigned long *)((char *)(E) + SM_WORD * (1 + (E)->ctl.Q + (E)->ctl.K) + 8 * (1 + (E)->ctl.K))

#define SM_EVENT_PRIORITY_1(E) \
    *(unsigned long *)((char *)(E) + SM_WORD * (2 + (E)->ctl.Q + (E)->ctl.K) + 8 * (1 + (E)->ctl.K))

#define SM_EVENT_HANDLE(E) \
    *(void **)((char *)(E) + SM_WORD * (1 + (E)->ctl.Q + (E)->ctl.K + 2 * (E)->ctl.P) + 8 * (1 + (E)->ctl.K))

#define SM_EVENT_DATA(E) \
    (void *)((char *)(E) + SM_WORD * (1 + (E)->ctl.Q + (E)->ctl.K + 2 * (E)->ctl.P + (E)->ctl.H) + 8 * (1 + (E)->ctl.K))

#define SM_EVENT_DATA_SIZE(E) ((size_t)((E)->ctl.size) << 6)


// Public methods

sm_event *sm_event_create(uint32_t size, bool Q, bool K, bool P, bool H);
void sm_event_free(sm_event *e); 
void sm_event_wipe(sm_event *e);
int sm_event_to_string(sm_event *e, char *buffer);
//int sm_event_set_key(sm_event *e, void* key, size_t length);

#endif //SM_EVENT_H
