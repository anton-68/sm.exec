/* SM.EXEC
   SM thread tests
   anton.bondarenko@gmail.com */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../src/sm_sys.h"
#include "../../src/sm_fsm.h"
#include "../../src/sm_app.h"
#include "../../src/sm_state.h"
#include "../../src/sm_thread.h"
#include "../../oam/logger.h"

static int WAIT(sm_event *e, sm_state *s) {
	printf("\nWAIT app invoked for event Id : %u", (unsigned)e->id);
	fflush(stdout);
	sleep((e->id)*3);
	e->disposable = false;
	printf("\nWAIT done for event Id : %u, ", (unsigned)e->id);
	printf("q0.top = %lu, ", sm_queue_top(((sm_queue **)(s->process->context))[0])->id);
	printf("q1.top = %lu", sm_queue_top(((sm_queue **)(s->process->context))[1])->id);
	fflush(stdout);
	return 0;
}

static int QUE0(sm_event *e, sm_state *s) {
	printf("\nQUE0 app invoked for event Id : %u", (unsigned)e->id);
	fflush(stdout);
	e->disposable = false;
	//printf("\nQUE0 stack address: %p", ((sm_queue **)(s->process->context))[0]);
	sm_queue_enqueue(e, ((sm_queue **)(s->process->context))[0]);
	printf("\nQUE0 done for event Id : %u", (unsigned)e->id);
	fflush(stdout);
	return 0;
}

static int QUE1(sm_event *e, sm_state *s) {
	printf("\nQUE1 app invoked for event Id : %u", (unsigned)e->id);
	fflush(stdout);
	e->disposable = false;
	sm_queue_enqueue(e, ((sm_queue **)(s->process->context))[1]);
	printf("\nQUE1 done for event Id : %u", (unsigned)e->id);
	fflush(stdout);
	return 0;
}

static int NOAP(sm_event *e, sm_state *s) {
	printf("\nNOAP invoked for event Id : %u ... done", (unsigned)e->id);
	fflush(stdout);
	return 0;
}

/* driver */

int main()
{   
	printf("\nSM.EXEC FSM module test\n");
	
	sm_app_table *at = sm_app_table_create();
	at = sm_app_table_set(at, "WAIT", WAIT);
	at = sm_app_table_set(at, "NOAP", NOAP);
	at = sm_app_table_set(at, "QUE0", QUE0);
	at = sm_app_table_set(at, "QUE1", QUE1);
	printf("Application regestry created...\n");
	
	sm_fsm_table *ft = sm_fsm_table_create();
	char jstr[32 * 1024];
	FILE * file;
	sm_event *e;
	unsigned *uptr;
	int i;

	char fsm0fn[] = "test_sm_fsm_0.json";
	file = fopen(fsm0fn, "r");
	if (!file) {
		REPORT(ERROR, "Input FSM0 filename missing or incorrect\n");
		exit(0);
	}
	printf("Input filename: %s\n", fsm0fn);
	printf("Reading file... \n");
	i=0;
	while(!feof(file))
		jstr[i++] = fgetc(file);
	jstr[--i]='\0';
	fclose(file);
	printf("JSON scanned...\n");
	sm_fsm *fsm0 = sm_fsm_create(jstr, at, SM_MEALY);
	if(fsm0 == NULL) 
		return EXIT_FAILURE;
	printf("FSM0 created...\n");
	ft = sm_fsm_table_set(ft, "FSM0", fsm0);
	//printf("ft = %p, FSM0 ref:%p\n", ft, sm_fsm_table_get_ref(ft, "FSM0"));
	printf("%s", sm_fsm_to_string(*(sm_fsm_table_get_ref(ft, "FSM0"))));
	
	char fsm1fn[] = "test_sm_fsm_1.json";
	file = fopen(fsm1fn, "r");
	if (!file) {
		REPORT(ERROR, "Input FSM1 filename missing or incorrect\n");
		exit(0);
	}
	printf("\nInput filename: %s\n", fsm1fn);
	printf("Reading file... \n");
	i=0;
	while(!feof(file))
		jstr[i++] = fgetc(file);
	jstr[--i]='\0';
	fclose(file);
	printf("JSON scanned...\n");	
	sm_fsm *fsm1 = sm_fsm_create(jstr, at, SM_MEALY);
	if(fsm1 == NULL) 
		return EXIT_FAILURE;
	printf("FSM1 created...\n");
	ft = sm_fsm_table_set(ft, "FSM1", fsm1);
	printf("%s", sm_fsm_to_string(*sm_fsm_table_get_ref(ft, "FSM1")));
	
	sm_process_desc *pd = sm_process_create(16, at, ft);
	
	sm_queue *q0 = sm_queue_create(0, 0, true);
	printf("Queue q0 created at %p\n", q0);
	for(unsigned i = 1; i <= 3; i++) {
		e = sm_event_create(16);
		e->id = (SM_EVENT_ID)i;
		uptr = (unsigned *)e->data;
		*uptr = i;
		sm_queue_enqueue(e, q0);
	}
	printf("Queue populated...\n");
	printf("Queue size = %lu\n", sm_queue_size(q0));	

	sm_queue *q1 = sm_queue_create(0, 0, true);
	printf("Queue q1 created at %p\n", q1);
	for(unsigned i = 4; i <= 5; i++) {
		e = sm_event_create(16);
		e->id = (SM_EVENT_ID)i;
		uptr = (unsigned *)e->data;
		*uptr = i;
		sm_queue_enqueue(e, q1);
	}
	printf("Queue populated...\n");
	printf("Queue size = %lu\n", sm_queue_size(q1));
	
	sm_queue *queue_reg[2] = {q0, q1};
	pd->context = (void *)queue_reg;
	
	sm_thread_desc *tx0 = sm_thread_create(sm_fsm_table_get_ref(ft, "FSM0"), 
											256, q0, 0, 0, pd, true);
	printf("Thread descriptor tx0 created...\n");
	sm_thread_desc * tx1 = sm_thread_create(sm_fsm_table_get_ref(ft, "FSM1"), 
											256, q1, 0, 0, pd, true);
	printf("Thread descriptor tx1 created...\n");
	
	pthread_t t0;
	pthread_create(&t0, NULL, &sm_thread_runner, (void *)tx0);
	pthread_t t1;
	pthread_create(&t1, NULL, &sm_thread_runner, (void *)tx1);

	pthread_join(t0, NULL); 
	pthread_join(t1, NULL); 
	
	printf("\n\nFinished...\n");
	
	// clean-up				
	sm_thread_free(tx0);
	sm_thread_free(tx1);
	sm_app_table_free(at);
	return EXIT_SUCCESS;
}