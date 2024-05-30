/* SM.EXEC 
   Test applications
   anton.bondarenko@gmail.com */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../src/sm_sys.h"
#include "../src/sm_app.h"
#include "../src/sm_event.h"
#include "../src/sm_state.h"

int sm_test_app1(sm_event *e) {
	printf("test app # 1 invoked, event data: %s\n", (char *)e->data);
	return 0;	
}

int sm_test_app2(sm_event *e) {
	printf("test app # 2 invoked, event data: %s\n", (char *)e->data);
	return 0;	
}

int sm_test_app3(sm_event *e) {
	printf("test app # 3 invoked, event data: %s\n", (char *)e->data);
	return 0;	
}

int sm_test_app4(sm_event *e) {
	printf("test app # 4 invoked, event data: %s\n", (char *)e->data);
	return 0;	
}

int sm_nope(sm_event *e) {
	printf("nope app invoked, event data: %s\n", (char *)e->data);
	return 0;	
}

int WAIT(sm_event *e, sm_state *s) {
	printf("\nWAIT app invoked for event Id : %u", (unsigned)e->id);
	fflush(stdout);
	sleep((e->id)*3);
	e->disposable = false;
	printf("\nWAIT done for event Id : %u, ", (unsigned)e->id);
	printf("q0.top = %lu, ", sm_queue2_get(((sm_queue2 **)(s->exec->data))[0])->id);
	printf("q1.top = %lu", sm_queue2_get(((sm_queue2 **)(s->exec->data))[1])->id);
	fflush(stdout);
	return 0;
}

int QUE0(sm_event *e, sm_state *s) {
	printf("\nQUE0 app invoked for event Id : %u", (unsigned)e->id);
	fflush(stdout);
	e->disposable = false;
	//printf("\nQUE0 stack address: %p", ((sm_queue **)(s->process->context))[0]);
	sm_lock_enqueue2(e, ((sm_queue2 **)(s->exec->data))[0]);
	printf("\nQUE0 done for event Id : %u", (unsigned)e->id);
	fflush(stdout);
	return 0;
}

int QUE1(sm_event *e, sm_state *s) {
	printf("\nQUE1 app invoked for event Id : %u", (unsigned)e->id);
	fflush(stdout);
	e->disposable = false;
	sm_lock_enqueue2(e, ((sm_queue2 **)(s->exec->data))[1]);
	printf("\nQUE1 done for event Id : %u", (unsigned)e->id);
	fflush(stdout);
	return 0;
}

int NOAP(sm_event *e, sm_state *s) {
	printf("\nNOAP invoked for event Id : %u ... done", (unsigned)e->id);
	fflush(stdout);
	return 0;
}