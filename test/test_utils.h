/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Test utils
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#ifndef SM_TEST_UTILS_H
#define SM_TEST_UTILS_H

#include <stdlib.h>
#include "../src/sm_hash.h"
#include "../src/sm_event.h"
#include "../src/sm_state.h"
#include "../src/sm_queue.h"
#include "../src/sm_fsm.h"

#define SM_TEST_PRINT_BUFFER 2048

void sm_print_hash_key(const sm_hash_key *k); 
void sm_print_event(const sm_event *e);
void sm_print_state(sm_state *s);
void sm_print_queue(sm_queue *q);

#endif//SM_TEST_UTILS_H
