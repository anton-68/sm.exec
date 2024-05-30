/* SM.EXEC
   Event module   
   (c) anton.bondarenko@gmail.com */

#include <stdlib.h>
#include <string.h>
#include "sm_event.h"
#include "sm_queue.h"
#include "sm_logger.h"
#include "sm_memory.h"

#define _head 2
#define _pull(E) _head
#define _hndl(E) (_pull(E) + (E)->ctl.pool_flag ? 1 : 0)
#define _hshk(E) (_hndl(E) + ((E)->ctl.handle_flag ? 1 : 0))
#define _prrt(E) (_hshk(E) + ((E)->ctl.hash_key_flag ? 2 : 0))

sm_event *sm_event_create(size_t data_size) {
	size_t size = sm_memory_size_align(data_size, SM_WORD)
	if(size > UINT32_MAX) {
		SM_LOG(SM_CORE, SM_ERR, "Event data size is too large");
        return NULL;
    }
	if((sm_event *e = malloc(2 * SM_WORD + size)) == NULL) {
		SM_LOG(SM_CORE, SM_LOG_ERR, "malloc() failed to allocate sm_event");
        return e;
    }
	memset((void *)e, 0, 2 * SM_WORD + size);
	e->data_size = (uint32_t)size;
return e;
}

sm_event *sm_event_ext_create(uint32_t data_size
							  bool disposable_flag, 
						  	  bool hash_key_flag,
						 	  bool priority_flag,
						      sm_queue * home) {
	size_t offset = 0;
	if(pool_flag && home != NULL)
		offset += 1;
	if(handle_flag)
		offset += 1;
	if(hash_key_flag)
		offset += 2;
	if(priority_flag)
		offset += 2;
	if((sm_event *e = sm_event_create(data_size + offset * SM_WORD)) == NULL){
		SM_LOG(SM_CORE, SM_LOG_ERR, "Failed to create event");
        return e;
    }
	e->data_size -= offset * SM_WORD;
	e->ctl.data_offset += offset;
	e->ctl.disposable_flag = disposable_flag;
	if(home != NULL) {
		SM_EVENT_DEPOT(e) = home;
		e->ctl.pool_flag = 1;
	}
	e->ctl.hash_key_flag = hash_key_flag;
	e->ctl.priority_flag = priority_flag;									  
return e;
}
    
void sm_event_free(sm_event *e) {
	sm_event *e1;
	while(e != NULL) {
		e1 = sm_event_unlink(e);
		free(e);
		e = e1;
	}
}

static void purge(sm_event *e) {
	memset((void *)e + SM_EVENT_HASH_KEY_OFFSET(e), 0, 
		   (e->ctl.hash_key_flag + e->ctl.priority_flag) * 2 * SM_WORD 
		    + sm_event_data_size(e));
	e->ctl.id = 0;
	//e->ctl.tailed_flag = 0;
	e->ctl.disposable_flag = 0;
}

void sm_event_purge(sm_event *e) {
	sm_event *e1;
	while(e != NULL) {
		e1 = sm_event_tail(e);
		purge(e);
		e = e1;
	}
}

void sm_event_park(sm_event *e) {
	if(e->ctl->disposable_flag) {
		if(e->ctl->pool_flag) {
			sm_event_purge(e);
			sm_queue_enqueue(sm_event_pool_ptr(e), e);
		}
		else
			sm_event_free(e);
	}
}

bool sm_event_is_disposable(sm_event *e) {
	return (bool)e->ctl.disposable_flag;
}

bool sm_event_is_valid(sm_event *e) {
	return true;	
} // e != NULL && ...

void sm_event_link(sm_event *e1, sm_event *e2) {
	if(!e1->ctl.tailed_flag){
		if(e1->next == NULL) 
			e1->next = e2;
		if(e1->next == e2)
			e1->ctl.tailed_flag = 1;
	}
	else {
		SM_LOG(SM_CORE, SM_LOG_DEBUG, "Failed to link events"); 
	}
}

sm_event *sm_event_unlink(sm_event *e) {
	sm_event *e1 = NULL;
	if(!e->ctl.tailed_flag) {
		e1 = e->next;
		e->next = NULL;
		e->ctl.tailed_flag = 0;
	}
	return e1;
}

sm_event *sm_event_tail(sm_event *e) {
	return e->ctl.tailed_flag ? e->next : NULL;
}

bool sm_event_is_linked(sm_event *e) {
	return (bool)e->ctl.tailed_flag;
}

size_t sm_event_id(event *e) {
	return (size_r)e->ctl.id;
}

void sm_event_set_id(event *e, size_t id) {
	if(1<<sizeof(e->ctl.id) < id)
		SM_LOG(SM_CORE, SM_LOG_DEBUG, "Event id value exceeds allowed maximum");
	e->ctl.id = (uint16_t)id;
}

size_t sm_event_data_size(event	*e) {
	return (size_t)e->data_size;
}

void *sm_event_data_ptr(event *e) {
	 return (void *)((sm_word_t *)e + (ptrdiff_t)(_head + e->ctl.data_offset));
}

sm_queue* sm_event_pool_ptr(sm_event *e) {
	return (sm_queue *)(e->ctl.pool_flag ? (sm_word_t *)e + (ptrdiff_t)_pull(e) : NULL);
}

void *sm_event_handle_ptr(sm_event *e) {
	return (void *)(e->ctl.handle_flag ? (sm_word_t *)e + (ptrdiff_t)_hndl(e) : NULL);
}

sm_hash_key *sm_event_hash_key_ptr(sm_event *e) {
	return (sm_hash_key *)(e->ctl.hash_key_flag ? (sm_word_t *)e + _hshk(e): NULL);
}

sm_event_priority *sm_event_priority_ptr(sm_event *e){ 
	return (sm_event_priority *)(e->ctl.priority_flag ? (sm_word_t *)e + _prrt(e) : NULL);
}







