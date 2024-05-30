/* SM.EXEC
   SM thread tests
   anton.bondarenko@gmail.com */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
/* dlopen() flag */
#define SM_LUA_RTLD_FLAG RTLD_NOW | RTLD_GLOBAL

#include "../../src/sm_sys.h"
#include "../../src/sm_fsm.h"
#include "../../src/sm_app.h"
#include "../../src/sm_directory.h"
#include "../../src/sm_state.h"
#include "../../src/sm_exec.h"
#include "../../oam/logger.h"



/* driver */

int main()
{   
	void *applib = dlopen("../../app/sm_test_apps.so", SM_LUA_RTLD_FLAG);
	sm_directory *dir = sm_directory_create();
	sm_app WAIT = dlsym(applib, "WAIT");
	dir = sm_directory_set(dir, "WAIT", (void *)WAIT);
	sm_app NOAP = dlsym(applib, "NOAP");
	dir = sm_directory_set(dir, "NOAP", (void *)NOAP);
	sm_app QUE0 = dlsym(applib, "QUE0");
	dir = sm_directory_set(dir, "QUE0", (void *)QUE0);
	sm_app QUE1 = dlsym(applib, "QUE1");
	dir = sm_directory_set(dir, "QUE1", (void *)QUE1);
	
	char jstr[32 * 1024];
	FILE * file;
	sm_event *e;
	unsigned *uptr;
	
	// FSM 0
	char fsm0fn[] = "test_sm_fsm_0.json";
	file = fopen(fsm0fn, "r");
	if (!file) {
		REPORT(ERROR, "Input FSM0 filename missing or incorrect\n");
		exit(0);
	}
	int i=0;
	while(!feof(file))
		jstr[i++] = fgetc(file);
	jstr[--i]='\0';
	fclose(file);
	sm_fsm *fsm0 = sm_fsm_create(jstr, dir, SM_MEALY);
	if(fsm0 == NULL) 
		return EXIT_FAILURE;
	dir = sm_directory_set(dir, "FSM0", (void *)fsm0);
	
	// FSM 1
	char fsm1fn[] = "test_sm_fsm_1.json";
	file = fopen(fsm1fn, "r");
	if (!file) {
		REPORT(ERROR, "Input FSM1 filename missing or incorrect\n");
		exit(0);
	}
	i=0;
	while(!feof(file))
		jstr[i++] = fgetc(file);
	jstr[--i]='\0';
	fclose(file);	
	sm_fsm *fsm1 = sm_fsm_create(jstr, dir, SM_MEALY);
	if(fsm1 == NULL) 
		return EXIT_FAILURE;
	dir = sm_directory_set(dir, "FSM1", (void *)fsm1);
	
	// sm_exec/sm_tx
	
	sm_exec *exec = sm_exec_create(256, dir, NULL);
	sm_tx *tx0 = sm_tx_create(exec, (sm_fsm **)sm_directory_get_ref(dir, "FSM0"), 16, 16, NULL, true);
	sm_tx *tx1 = sm_tx_create(exec, (sm_fsm **)sm_directory_get_ref(dir, "FSM1"), 16, 16, NULL, true);
	
	for(unsigned i = 1; i <= 3; i++) {
		e = sm_event_create(16);
		e->id = (SM_EVENT_ID)i;
		uptr = (unsigned *)e->data;
		*uptr = i;
		sm_enqueue2(e, *tx0->input_queue);
	}

	for(unsigned i = 4; i <= 5; i++) {
		e = sm_event_create(16);
		e->id = (SM_EVENT_ID)i;
		uptr = (unsigned *)e->data;
		*uptr = i;
		sm_enqueue2(e, *tx1->input_queue);
	}
	
	sm_queue2 *queue_reg[2] = {*tx0->input_queue, *tx1->input_queue};
	memcpy(exec->data, (void *)queue_reg, sizeof(sm_queue2 *[2]));
	//exec->data = (void *)queue_reg;
	
	pthread_t t0;
	pthread_create(&t0, NULL, &sm_tx_runner, (void *)tx0);
	pthread_t t1;
	pthread_create(&t1, NULL, &sm_tx_runner, (void *)tx1);
	
	pthread_join(t0, NULL); 
	pthread_join(t1, NULL); 
	
	// clean-up				
	sm_tx_free(tx0);
	sm_tx_free(tx1);
	sm_directory_free(dir);
	return EXIT_SUCCESS;
}