/* SM.EXEC
   Event module
   (c) anton.bondarenko@gmail.com */

#ifndef SM_EVENT_H
#define SM_EVENT_H

#include <stdint.h>     // uint32_t
#include "sm_sys.h"
//#include "sm_queue.h"



struct sm_queue;

// sm_event
typedef struct sm_event {
	SM_EVENT_ID id;
	struct sm_event *next;
	size_t data_size;
    void *data;
	bool to_keep;
	struct sm_queue *home;
} sm_event;

// Public methods
sm_event *sm_event_create(size_t payload_size);
void sm_event_free(sm_event *e);
void sm_event_park(sm_event *e);

#endif //SM_EVENT_H
