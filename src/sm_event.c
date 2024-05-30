/* SM.EXEC
   Event module   
   (c) anton.bondarenko@gmail.com */

#include <stdlib.h>			// malloc(), free(), NULL, size_t, 
#include <string.h>			// memset()

#include "../oam/logger.h"
#include "sm_event.h"
#include "sm_queue.h"
#include "sm_memory.h"

/* sm_event */

sm_event *sm_event_create(size_t payload_size) {
    sm_event *e;
    if((e = malloc(sizeof(sm_event))) == NULL) {
        REPORT(ERROR, "malloc()");
        return e;
    }
	if(SM_MEMORY_MANAGER)
		e->data_size = sm_memory_size_align(payload_size, sizeof(sm_chunk));
	else
		e->data_size = payload_size;
    if(e->data_size > 0) {
        if ((e->data = malloc(e->data_size)) == NULL) {
            REPORT(ERROR, "malloc()");
            free(e);
            return NULL;
        }
		memset(e->data, 0, e->data_size);
    }
    else {
        e->data = NULL;
	}
    e->next = NULL;
	e->id = 0;
	for (int stage = 0; stage < SM_NUM_OF_PRIORITY_STAGES; stage++)
		e->priority[stage] = 0;
	e->home = NULL;
	e->key = NULL;
	e->key_hash = 0;
	e->key_length = 0;
	e->disposable = false;
	return e;    
}
    
void sm_event_free(sm_event *e) {
	free(e->data);
	free(e);
	e = NULL;
}

void sm_event_park(sm_event *e) {
	if(e->home == NULL)
		sm_event_free(e);
	else
		sm_queue_enqueue(e, e->home);
}
