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
#include "../src/sm_queue2.h"
#include "../src/sm_array.h"
#include "../src/sm_directory.h"
#include "../src/sm_fsm.h"

#define SM_TEST_PRINT_BUFFER 2048

void sm_print_hash_key(const sm_hash_key *k); 
void sm_print_event(const sm_event *e);
void sm_print_state(sm_state *s);
void sm_print_queue(sm_queue *q);
void sm_print_queue2(sm_queue2 *q); 
void sm_print_array(sm_array *a);
void sm_print_directory(sm_directory *d);
void sm_print_fsm(sm_fsm *f, sm_directory *d);

#endif//SM_TEST_UTILS_H
