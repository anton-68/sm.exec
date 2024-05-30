/* SM.EXEC
   Event module
   (c) anton.bondarenko@gmail.com */

#ifndef SM_EVENT_H
#define SM_EVENT_H

#include <stdint.h>	
#include "sm_sys.h"
#include "sm_hash.h" 

typedef	struct sm_event	{
	struct sm_event *next;
	uint32_t data_size;
	struct {
		unsigned int id					 : 16;
		unsigned int data_offset		 :  8; // in x64 words
		unsigned int tailed_flag		 :  1;
		unsigned int disposable_flag	 :  1;
		unsigned int pool_flag		 	 :  1;
		unsigned int handle_flag		 :  1; // managed by 'handler'
		unsigned int hash_key_flag		 :  1;
		unsigned int priority_flag		 :  1;
		unsigned int /* reserved */		 :  0; // 2 bits
	} ctl;
} sm_event;
typedef long long int sm_event_priority;
typedef sm_word_t sm_event_handle;

/* Lifecycle */
sm_event *sm_event_create(uint32_t data_size);
sm_event *sm_event_ext_create(uint32_t data_size, bool disposable_flag, bool hash_key_flag, bool priority_flag, sm_queue * home);
void sm_event_free(sm_event *e);
void sm_event_purge(sm_event *e);
void sm_event_park(sm_event *e);
bool sm_event_is_valid(sm_event *e);
bool sm_event_is_disposable(sm_event *e);
/* Chaining */
void sm_event_link(sm_event *e1, sm_event *e2);
sm_event *sm_event_unlink(sm_event *e);
sm_event *sm_event_tail(sm_event *e);
bool sm_event_is_linked(sm_event *e);
/* Id */
size_t sm_event_id(event *e);
void sm_event_set_id(event *e, size_t id);
/* Data */
size_t sm_event_data_size(event	*e) {return(size_t)e->data_size;}
void *sm_event_data_ptr(event *e);
/* Home queue */
sm_queue* sm_event_pool_ptr(sm_event *e);
/* Handle */
void *sm_event_handle_ptr(sm_event *e);
/* Hash key */
sm_hash_key *sm_event_hash_key_ptr(sm_event *e);
/* Priority */
long long int *sm_event_priority_ptr(sm_event *e);

#endif //SM_EVENT_H
