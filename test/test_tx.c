/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Thread worker module tests
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#include "test_utils.h"

int main()
{
    sm_directory *d = sm_directory_create();
    sm_exec *e = sm_exec_create(256, d);
    sm_queue2 *q = sm_queue2_create();
    FILE *file = fopen("sm0.json", "r");
    if (!file)
    {
        SM_REPORT_ERROR("Input FSM file 1 missing or incorrect");
        exit(0);
    }
    char jstr[32 * 1024];
    int i = 0;
    while (!feof(file))
        jstr[i++] = fgetc(file);
    jstr[--i] = '\0';
    fclose(file);
    sm_fsm *f = sm_fsm_create(jstr, d);
    
    sm_tx *tx = sm_tx_create(e, &f, 1024, 256, &q, false);

    sm_print_tx(tx);

    sm_print_state(tx->state);
    sm_state *s0 = sm_state_create(&f, 256, NULL, false, true, false, false);
    s0->state_id = 77;
    sm_tx_push_state(tx, &s0);
    sm_print_state(tx->state);
    sm_state *s1 = sm_state_create(&f, 256, NULL, false, true, false, false);
    s1->state_id = 78;
    sm_tx_push_state(tx, &s1);
    sm_print_state(tx->state);
    sm_tx_pop_state(tx);
    sm_print_state(tx->state);
    sm_tx_pop_state(tx);
    sm_print_state(tx->state);

    sm_tx_destroy(&tx);

}