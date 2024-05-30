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
#include "../../src/sm_thread.h"
#include "../../oam/logger.h"


#define OUTPUT_BUF_LEN (1024 * 32)


/* construct fsm & dafault state*/

int WAIT(sm_event *e) {
	printf("\nWAIT app invoked");
	printf("\nprocessed event Id : %u", (unsigned)e->id);
	sleep(e->id);
	printf("\nWAIT done");
	return 0;
}

int NOAP(sm_event *e) {
	printf("\nNOAP invoked");
	printf("\nprocessed event Id : %u", (unsigned)e->id);
	printf("\ndoing nothing");
	printf("\nNOAP done");
	return 0;
}

/* driver */

int main()
{   
	char fn[] = "wait.json";
	FILE * file;
	file = fopen(fn, "r");
	if (!file) {
		REPORT(ERROR, "Input FSM filename missing or incorrect\n");
		exit(0);
	}
	printf("\nSM.EXEC FSM module test\ninput filename: %s\n", fn);
	char jstr[32 * 1024];
	printf("reading file... \n");
	int i=0;
	while(!feof(file))
		jstr[i++] = fgetc(file);
	jstr[--i]='\0';
	fclose(file);
	printf("json scanned...\n");
	sm_app_table *at;
	at = sm_app_table_create();
	at = sm_app_table_set(at, "WAIT", WAIT);
	at = sm_app_table_set(at, "NOAP", NOAP);
	printf("application regestry created...\n");
	sm_fsm *fsm = sm_fsm_create(jstr, at, SM_MOORE);
	if(fsm == NULL) 
		return EXIT_FAILURE;
	printf("fsm created...\n");
	char *res = sm_fsm_to_string(fsm);
	printf("%s", res);
	free(res);
	//sm_fsm_table *ft = sm_fsm_table_create();
	//ft = sm_app_table_set(ft, "WAIT", fsm);
	//printf("fsm table created...\n");
	//sm_process_desc *proc = sm_process_create(256, at, ft);
	sm_queue *q = sm_queue_create(0, 0, false);
	printf("queue created...\n");
	sm_event *e;
	unsigned *uptr;
	for(unsigned i = 1; i <= 4; i++) {
		e = sm_event_create(16);
		e->id = (SM_EVENT_ID)i;
		uptr = (unsigned *)e->data;
		*uptr = i;
		sm_queue_enqueue(e, q);
	}
	printf("queue populated...\n");
	printf("queue size = %lu\n", sm_queue_size(q));
	sm_thread_desc * tx = sm_thread_create(fsm->ref, 256, q, 0, 0, NULL, false);
	printf("thread descriptor created...\n");
	sm_thread_runner(tx);
	printf("\n\nfinished...\n");
	// clean-up				
	sm_thread_free(tx);
	sm_app_table_free(at);
	return EXIT_SUCCESS;
}