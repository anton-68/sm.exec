/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Array module tests
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#include "test_utils.h"

static sm_hash_key k0, k1, k2;
static char *str0 = "keystring_0";
static char *str1 = "keystring_1";
static char *str2 = "keystring_2";

void test_array_seq(sm_array *a)
{
    sm_state *tmp;
    sm_print_array(a);
    printf("\n**************\n");
    printf("checkpoint #11\n");
    printf("**************\n\n");
    sm_state *s0= sm_array_get_state(a, k0.string, k0.length);
    sm_print_state(s0);
    sm_state *s1 = sm_array_get_state(a, k1.string, k1.length);
    sm_print_state(s1);
    sm_state *s2 = sm_array_get_state(a, k2.string, k2.length);
    sm_print_state(s2);
    sm_print_array(a);
    printf("\n**************\n");
    printf("checkpoint #12\n");
    printf("**************\n\n");
    tmp = s0;
    sm_array_park_state(&s0);
    sm_print_state(tmp);
    sm_print_array(a);
    printf("\n**************\n");
    printf("checkpoint #13\n");
    printf("**************\n\n");
    SM_ARRAY_PARK_STATE(s1);
    SM_ARRAY_PARK_STATE(s2);
    sm_print_array(a);
    printf("\n**************\n");
    printf("checkpoint #14\n");
    printf("**************\n\n");
    s0 = sm_array_get_state(a, k0.string, k0.length);
    sm_print_state(s0);
    printf("\n****************\n");
    printf("checkpoint #14.1\n");
    printf("****************\n\n");
    tmp = s0;
    SM_ARRAY_RELEASE_STATE(s0);
    sm_print_state(tmp);
    sm_print_array(a);
    printf("\n**************\n");
    printf("checkpoint #15\n");
    printf("**************\n\n");
    SM_ARRAY_RELEASE_STATE(s1);
    printf("\n****************\n");
    printf("checkpoint #15.1\n");
    printf("****************\n\n");
    SM_ARRAY_RELEASE_STATE(s2);
    printf("\n****************\n");
    printf(  "checkpoint #15.2\n");
    printf(  "****************\n\n");
    sm_print_array(a);
    printf("\n**************\n");
    printf("checkpoint #16\n");
    printf("**************\n\n");
}

int main()
{
    uint32_t hash_mask = hashmask(8);
    sm_hash_set_key(&k0, str0, strlen(str0), hash_mask);
    sm_print_hash_key(&k0);
    sm_hash_set_key(&k1, str1, strlen(str1), hash_mask);
    sm_print_hash_key(&k1);
    sm_hash_set_key(&k2, str2, strlen(str2), hash_mask);
    sm_print_hash_key(&k2);

    printf("\n*************\n");
    printf("checkpoint #1\n");
    printf("*************\n\n");
    
    char *fsm_name = "FSM0";
    sm_fsm fsm0;
    fsm0.name = fsm_name;
    fsm0.initial = 77;
    sm_fsm *fsm0p = &fsm0;
    sm_fsm **fsm0r = &fsm0p;

    printf("\n*************\n");
    printf("checkpoint #2\n");
    printf("*************\n\n");

    sm_array *a0 = sm_array_create(8, 256, 256, fsm0r, false, false, false, false, false);

    printf("\n*************\n");
    printf("checkpoint #3\n");
    printf("*************\n\n");

    sm_print_state(a0->queue_head);
    sm_print_state(SM_STATE_NEXT(a0->queue_head));
    sm_print_state(SM_STATE_NEXT(SM_STATE_NEXT(a0->queue_head)));
    sm_print_state(SM_STATE_NEXT(SM_STATE_NEXT(SM_STATE_NEXT(a0->queue_head))));

    printf("\n*************\n");
    printf("checkpoint #4\n");
    printf("*************\n\n");
        
    test_array_seq(a0);
    SM_ARRAY_DESTROY(a0);

    printf("\n***************\n");
    printf("checkpoint #4.1\n");
    printf("***************\n\n");

    a0 = sm_array_create(8, 256, 256, fsm0r, true, false, false, false, false);
    test_array_seq(a0);
    SM_ARRAY_DESTROY(a0);

    printf("\n*************\n");
    printf("checkpoint #5\n");
    printf("*************\n\n");

    a0 = sm_array_create(8, 256, 256, fsm0r, true, false, true, false, false);
    test_array_seq(a0);
    SM_ARRAY_DESTROY(a0); //!!!

    printf("\n*************\n");
    printf("checkpoint #6\n");
    printf("*************\n\n");

    a0 = sm_array_create(8, 256, 256, fsm0r, true, false, true, false, true);
    test_array_seq(a0);
    SM_ARRAY_DESTROY(a0);

    printf("\n*************\n");
    printf("checkpoint #7\n");
    printf("*************\n\n");

    a0 = sm_array_create(8, 256, 256, fsm0r, true, true, true, true, true);
    test_array_seq(a0);
    SM_ARRAY_DESTROY(a0);

    printf("\n*************\n");
    printf("checkpoint #7\n");
    printf("*************\n\n");
}