/* SM.EXEC
   State
   anton.bondarenko@gmail.com */

#ifndef SM_STATE_H
#define SM_STATE_H

#include <stdlib.h>
#include "sm_sys.h"
#include "sm_event.h"
#include "sm_fsm.h"
#include "../oam/logger.h"

                        



// state
typedef struct sm_state {
	SM_STATE_ID id;
    struct sm_state *next;
	size_t data_size;
    void *data;
	sm_fsm *fsm;
	void *key;
	size_t key_length;
    uint32_t key_hash;
	sm_event *trace;
} sm_state;

// Public methods
bool sm_state_key_match(sm_state *c, void *const key, size_t key_length);
sm_state *sm_state_create(size_t payload_size);
void sm_state_purge(sm_state *c);
void sm_state_free(sm_state *c);
void sm_apply_event(sm_state *s, sm_event *e);

#endif //SM_STATE_H 