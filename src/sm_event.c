/* SM.EXEC
   Event module   
   (c) anton.bondarenko@gmail.com */

#include <stdlib.h>
#include <string.h>
#include "sm_sys.h"
#include "sm_logger.h"
#include "sm_event.h"
#include "sm_queue.h"

#define _head 2
#define _pull(E) _head
#define _hndl(E) (_pull(E) + (E)->ctl.pool_flag ? 1 : 0)
#define _hshk(E) (_hndl(E) + ((E)->ctl.handle_flag ? 1 : 0))
#define _prrt(E) (_hshk(E) + ((E)->ctl.hash_key_flag ? 2 : 0))

sm_event *sm_event_create(size_t data_size) {
	size_t size = SM_ALIGN(data_size, SM_WORD);
	if(size > UINT32_MAX) {
		SM_LOG(SM_CORE, SM_LOG_ERR, "Event data size is too large");
        return NULL;
    }
	sm_event *e;
	if((e = malloc(2 * SM_WORD + size)) == NULL) {
		SM_LOG(SM_CORE, SM_LOG_ERR, "Failed to allocate event");
        return e;
    }
	memset((void *)e, 0, 2 * SM_WORD + size);
	e->data_size = (uint32_t)size;
return e;
}

sm_event *sm_event_ext_create(size_t data_size,
							  bool disposable_flag,
							  bool handle_flag,
						  	  bool hash_key_flag,
						 	  bool priority_flag,
						      sm_queue * home) {
	uint32_t offset = 0;
	if(home != NULL)
		offset += 1;
	if(handle_flag)
		offset += 1;
	if(hash_key_flag)
		offset += 2;
	if(priority_flag)
		offset += 2;
	sm_event *e;
	if((e = sm_event_create(data_size + offset * SM_WORD)) == NULL){
		SM_LOG(SM_CORE, SM_LOG_ERR, "Failed to create event");
        return e;
    }
	e->data_size -= offset * SM_WORD;
	e->ctl.data_offset += offset;
	e->ctl.disposable_flag = disposable_flag;
	e->ctl.handle_flag = handle_flag;
	if(home != NULL) {
		e->ctl.pool_flag = 1;
		*SM_WOFFSET(sm_queue *, e, 2) = home;
	}
	e->ctl.hash_key_flag = hash_key_flag;
	e->ctl.priority_flag = priority_flag;									  
return e;
}

sm_event *sm_event_ext_create_pool(size_t pool_size,
								   size_t data_size,
								   bool disposable_flag, 
							  	   bool handle_flag, 
								   bool hash_key_flag,
								   bool priority_flag,
								   sm_queue * home) {
	uint32_t offset = 0;
	if(home != NULL)
		offset += 1;
	if(handle_flag)
		offset += 1;
	if(hash_key_flag)
		offset += 2;
	if(priority_flag)
		offset += 2;
	size_t size = SM_ALIGN(data_size, SM_WORD);
	if(size > UINT32_MAX) {
		SM_LOG(SM_CORE, SM_LOG_ERR, "Event data size is too large");
        return NULL;
    }
	sm_event * epool;
	if((epool = (sm_event *)calloc(pool_size, size + (_head + offset) * SM_WORD)) == NULL) {
		SM_LOG(SM_CORE, SM_LOG_ERR, "Failed to allocate event pool");
        return epool;
    }
	sm_event *ehead = epool;
	pool_size++;
	while(--pool_size) {
		epool->next = SM_WOFFSET(sm_event, epool, _head + offset + size / SM_WORD);
		epool->data_size = (uint32_t)size + offset * SM_WORD;
		epool->ctl.data_offset = offset;
		epool->ctl.disposable_flag = disposable_flag;
		epool->ctl.handle_flag = handle_flag;
		if(home != NULL) {
			*SM_WOFFSET(sm_queue *, epool, 2) = home;
			epool->ctl.pool_flag = 1;
		}
		epool->ctl.hash_key_flag = hash_key_flag;
		epool->ctl.priority_flag = priority_flag;
		if(pool_size == 1)
			epool->next = ehead;
		else
			epool = epool->next;
	}
	//epool = SM_WOFFSET(sm_event, epool, - (_head + offset + size / SM_WORD));
	//epool->next = ehead;
	return epool;
}
	
void sm_event_free(sm_event *e) {
	free(e);
}

void sm_event_purge(sm_event *e) {
	memset((void *)e + _hshk(e), 0, 
		   (e->ctl.hash_key_flag + e->ctl.priority_flag) * 2 * SM_WORD 
		    + sm_event_data_size(e));
	e->ctl.id = 0;
	e->ctl.disposable_flag = 0;
}

void sm_event_park(sm_event *e) {
	if(e->ctl.disposable_flag) {
		if(e->ctl.pool_flag) {
			sm_event_purge(e);
			sm_queue_enqueue(e, sm_event_pool_ptr(e));
		}
		else
			sm_event_free(e);
	}
}

bool sm_event_is_disposable(sm_event *e) {
	return (bool)e->ctl.disposable_flag;
}

size_t sm_event_id(sm_event *e) {
	return (size_t)e->ctl.id;
}

void sm_event_set_id(sm_event *e, size_t id) {
	if(id > SM_EVENT_ID_MAX)
		SM_LOG(SM_CORE, SM_LOG_DEBUG, "Event id exceeding allowed maximum was trunkated");
	e->ctl.id = (uint16_t)id;
}

size_t sm_event_data_size(sm_event	*e) {
	return (size_t)e->data_size;
}

void *sm_event_data_ptr(sm_event *e) {
	return SM_WOFFSET(void, e, _head + e->ctl.data_offset);
}

sm_queue* sm_event_pool_ptr(sm_event *e) {
	return e->ctl.pool_flag ? SM_WOFFSET(sm_queue, e, _pull(e)) : NULL;
}

void *sm_event_handle_ptr(sm_event *e) {
	return e->ctl.handle_flag ? SM_WOFFSET(void, e, _hndl(e)) : NULL;
}

sm_hash_key *sm_event_hash_key_ptr(sm_event *e) {
	return e->ctl.hash_key_flag ? SM_WOFFSET(sm_hash_key, e, _hshk(e)) : NULL;
}

sm_event_priority *sm_event_priority_ptr(sm_event *e){ 
	return e->ctl.priority_flag ? SM_WOFFSET(sm_event_priority, e, _prrt(e)) : NULL;
}







