/* SM.EXEC
   Event module
   (c) anton.bondarenko@gmail.com */

#ifndef SM_EVENT_H
#define SM_EVENT_H

#include <stdint.h>
#include "sm_sys.h"

// sm_event
struct sm_queue;
typedef struct sm_event {
	struct sm_event *next;
	void *data;
	uint32_t data_size;
	uint32_t id;
	struct sm_queue *home;
	long long priority[2];
	void *key;
	uint32_t key_length;
	uint32_t key_hash;
	bool disposable;
} sm_event;

// Public methods
sm_event *sm_event_create(uint32_t payload_size);
void sm_event_free(sm_event *e);
void sm_event_park(sm_event *e);

#endif //SM_EVENT_H
