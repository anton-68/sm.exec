/* SM.EXEC
   Event module
   (c) anton.bondarenko@gmail.com */

#ifndef SM_EVENT_H
#define SM_EVENT_H

#include <stdint.h>     // uint32_t
#include "sm_sys.h"

struct sm_queue;

// sm_event
typedef struct sm_event {
// mandatory--------------------------------------------------	
	struct sm_event *next;								//  8
	struct {
		uint16_t data_addr  : 3;
		uint16_t home_addr	: 3;
		uint16_t priorities : 3;
		uint16_t hash_key	: 3;
		uint16_t variable	: 3;
		uint16_t disposable : 1; 
	} opt_map;											//  2
	uint16_t id;										//  2
	uint32_t data_size;									//	4
// optional fixed --------------------------------------------	
	//void *data;										//  8				0:+2
	//struct sm_queue *home;							//  8				1:+3
	//long long priority[SM_NUM_OF_PRIORITY_STAGES];	//  8 x 2 = 16		2:+4
	//void *key;										//  8 \				4:+6
	//uint32_t key_length;								//  4 }}  = 16		
	//uint32_t key_hash;								//  4 /	
// optional variable -----------------------------------------	
	//void 												//  reserved		6:+8
} sm_event;

// Public methods
sm_event *sm_event_create(size_t payload_size);
void sm_event_free(sm_event *e);
void sm_event_park(sm_event *e);

#endif //SM_EVENT_H
