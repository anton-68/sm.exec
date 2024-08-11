/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
FSM module tests
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#include "test_utils.h"

/* dummy apps */
int app1(sm_event *e, sm_state *s)
{
    printf("\nAPP1 invoked");
    printf("\nprocessed event Id : %u", e->event_id);
    printf("\ncurrent state Id : %u", s->state_id);
    printf("\nAPP1 done");
    return 0;
}

int app2(sm_event *e, sm_state *s)
{
    printf("\nAPP2 invoked");
    printf("\nprocessed event Id : %u", e->event_id);
    printf("\ncurrent state Id : %u", s->state_id);
    printf("\nAPP2 done");
    return 0;
}

int app3(sm_event *e, sm_state *s)
{
    printf("\nAPP3 invoked");
    printf("\nprocessed event Id : %u", e->event_id);
    printf("\ncurrent state Id : %u", s->state_id);
    printf("\nAPP3 done");
    return 0;
}

int noap(sm_event *e, sm_state *s)
{
    printf("\nNOAP invoked");
    printf("\ndoing nothing");
    printf("\nNOAP done");
    return 0;
}

/* driver */

int main(int argc, char *argv[])
{
    /*
        if (argc < 3)
        {
            printf("\nusage: %s <filename1.json> <filename2.json>\n", argv[0]);
            exit(0);
        }

        // char fn[1024];
        FILE *file;
    */
    FILE *file = fopen("sm0.json", "r");
    if (!file)
    {
        SM_REPORT_ERROR("Input FSM file 1 missing or incorrect");
        exit(0);
    }

    sm_directory *dir = sm_directory_create();
    printf("\nSystem registry created...\n\n");
    /*
        sm_queue2 *q2 = sm_queue2_create();
        dir = sm_directory_set(dir, "QUEUE_2", (void *)q2);
        printf("QUEUE_2 registered at %p\n", q2);

        sm_pqueue *pq = sm_pqueue_create(16, false);
        dir = sm_directory_set(dir, "LOGGER_QUEUE", (void *)pq);
        printf("LOGGER_QUEUE registered at %p\n", pq);
    */

    sm_queue *q2 = SM_QUEUE_CREATE_EMPTY(false);
    dir = sm_directory_set(dir, "QUEUE_2", (void *)q2);
    printf("QUEUE_2 registered at %p\n", q2);

    sm_queue *pq = SM_QUEUE_CREATE_EMPTY(true);
    dir = sm_directory_set(dir, "LOGGER_QUEUE", (void *)pq);
    printf("LOGGER_QUEUE registered at %p\n", pq);

    dir = sm_directory_set(dir, "APP1", (void *)app1);
    printf("APP1 registered at %p\n", app1);

    dir = sm_directory_set(dir, "APP2", (void *)app2);
    printf("APP2 registered at %p\n", app2);

    dir = sm_directory_set(dir, "APP3", (void *)app3);
    printf("APP3 registered at %p\n", app3);

    dir = sm_directory_set(dir, "NOAP", (void *)noap);
    printf("NOAP registered at %p\n", noap);

    sm_print_directory(dir);

    char jstr[32 * 1024];
    int i = 0;
    while (!feof(file))
        jstr[i++] = fgetc(file);
    jstr[--i] = '\0';
    fclose(file);
    printf("\nFSM file 1 red...\n\n");

    sm_fsm *sm0 = sm_fsm_create(jstr, dir);
    printf("FSM sm0 compiled:\n");
    char *res = sm_fsm_to_string(sm0, dir);
    printf("%s", res);
    dir = sm_directory_set(dir, "SM0", (void *)sm0);
    printf("FSM sm0 registered at %p\n", sm0);

    sm_array *d1 = sm_array_create(16, 1024, 256, (sm_fsm **)sm_directory_get_ref(dir, "SM0"), false, false, false, false);
    dir = sm_directory_set(dir, "DEPOT_1", (void *)d1);
    printf("\nDEPOT_1 registered at %p\n", d1);

    sm_print_directory(dir);

    file = fopen("sm1.json", "r");
    if (!file)
    {
        SM_REPORT_MESSAGE(SM_LOG_ERR, "input FSM file 2 missing or incorrect");
        exit(0);
    }

    i = 0;
    while (!feof(file))
        jstr[i++] = fgetc(file);
    jstr[--i] = '\0';
    fclose(file);
    printf("\nFSM file 2 red...\n");

    sm_fsm *sm1 = sm_fsm_create(jstr, dir);
    printf("\nFSM sm1 compiled:\n");
    res = sm_fsm_to_string(sm1, dir);
    printf("%s", res);
    dir = sm_directory_set(dir, "SM1", (void *)sm1);
    printf("\nFSM sm1 registered at %p\n", sm1);

    // sm_print_directory(dir);

    return 0;
}