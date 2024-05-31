/* SM.EXEC
   Event module   
   (c) anton.bondarenko@gmail.com */

#include <stdlib.h>
#include <string.h>
#include "sm_logger.h"
#include "sm_event.h"
#include "sm_queue.h"

#define _hshk(E) ((void *)(E) + 2)
#define _prrt(E) (_hshk(E) + 2 * (E)->ctl.key_flag)
#define _hndl(E) (_prrt(E) + (E)->ctl.priority_flag)
#define _home(E) ((sm_queue *)(_hndl(E) + (E)->ctl.handle_flag))
#define _data(E) (_home(E) + (E)->ctl.home_queue_flag)

#define HSHK(E) ((sm_hash_key *)(_hshk(E)))
#define PRRT(E) ((sm_event_priority *)(_prrt(E)))
#define HNDL(E) (*((sm_handle **)(_hndl(E))))
#define HOME(E) (*((sm_queue **)(_home(E))))
#define DATA (E) _data(E)

sm_event *sm_event_create(size_t data_size, bool key_flag, bool priority_flag, 
						  bool handle_flag, bool home_queue_flag){
	size_t dsize = SM_CEILING(data_size, 8);
	if(dsize >= 1<<24) {
		SM_LOG(SM_CORE, SM_LOG_ERR, "Event data size is too large");
        return NULL;
    }
	size_t hsize = 2 + key_flag * 2 + priority_flag * 2 + handle_flag + home_queue_flag;
	sm_event *e;
	if((e = malloc((hsize + dsize) * 8)) == NULL) {
		SM_LOG(SM_CORE, SM_LOG_ERR, "Failed to allocate event");
        return e;
    }
	memset((void *)e, 0, hsize + dsize);
	e->id_ext = SM_EVENT_TYPE_CODE;
	e->ctl.size = dsize;
	e->ctl.key_flag = key_flag;
	e->ctl.priority_flag = priority_flag;
	e->ctl.handle_flag = handle_flag;
	e->ctl.home_queue_flag = home_queue_flag;
	return e;
}

sm_event *sm_event_create_pool(size_t pool_size, size_t data_size, bool key_flag,
							   bool priority_flag, bool handle_flag, bool home_queue_flag) {
	size_t dsize = SM_CEILING(data_size, 8);
	if(dsize >= 1<<24) {
		SM_LOG(SM_CORE, SM_LOG_ERR, "Event data size is too large");
        return NULL;
    }
	size_t hsize = 2 + key_flag * 2 + priority_flag * 2 + handle_flag + home_queue_flag;
	sm_event *e;
	if((e = calloc(pool_size, (hsize + dsize) * 8)) == NULL) {
		SM_LOG(SM_CORE, SM_LOG_ERR, "Failed to allocate events");
        return e;
    }
	while(--pool_size) {
		e->next = e + hsize + dsize;
		e->id_ext = SM_EVENT_TYPE_CODE;
		e->ctl.size = (uint32_t)dsize;
		e->ctl.key_flag = key_flag;
		e->ctl.priority_flag = priority_flag;
		e->ctl.handle_flag = handle_flag;
		e->ctl.home_queue_flag = home_queue_flag;
		e = e->next;
	}
	e->next = NULL;
	return e;
}
		
void sm_event_free(sm_event *e) {
	if(e->ctl.disposable_flag == true) {
		if(e->ctl.handle_flag == true && HNDL(e) != NULL)
			HNDL(e)->free(HNDL(e)->object);
		free(e);
	}
	else
		SM_LOG(SM_CORE, SM_LOG_WARNING, "Attempt to free event which is not disposable");
}

void sm_event_purge(sm_event *e) {
	if(e->ctl.disposable_flag == true) {
		e->id_ext = SM_QUEUE_TYPE_CODE;
		e->id = 0;
		e->ctl.disposable_flag = 0;
		e->ctl.linked_flag = 0;
		if(e->ctl.key_flag == true) {
			HSHK(e)->key = NULL;
			HSHK(e)->key_length = 0;
			HSHK(e)->key_hash = 0;
		}		
		if(e->ctl.priority_flag == true) {
			PRRT(e)[0] = 0;
			PRRT(e)[1] = 0;
		}
		memset(_data(e), 0, e->ctl.size * 8);
			
	}
	else
		SM_LOG(SM_CORE, SM_LOG_WARNING, "Attempt to purge event which is not disposable");
}

void sm_event_recycle(sm_event *e) {
	if(e->ctl.disposable_flag) {
		sm_event_purge(e);
		if(e->ctl.handle_flag == true) {
			HNDL(e)->free(HNDL(e)->object);
			HNDL(e) = NULL;
		}
		if(e->ctl.home_queue_flag == true)
			sm_queue_enqueue(e, HOME(e));
		else
			sm_event_free(e);
	}
	else
		SM_LOG(SM_CORE, SM_LOG_WARNING, "Attempt to park event which is not disposable");
}

void sm_event_set_id(sm_event *e, size_t id) {
	if(id > SM_EVENT_ID_MAX)
		SM_LOG(SM_CORE, SM_LOG_DEBUG, "Event id exceeding allowed maximum was trunkated");
	e->id = (uint16_t)id;
}

size_t sm_event_id(sm_event *e) {
	return (size_t)e->id;
}

bool sm_event_check_type(sm_event *e) {
	return e->id_ext == SM_QUEUE_TYPE_CODE;
}

void *sm_event_data_ptr(sm_event *e) {
	return HOME(e);
}

size_t sm_event_data_size(sm_event *e) {
	return (size_t)(e->ctl.size * 8);
}

sm_hash_key *sm_event_hash_key_ptr(sm_event *e) {
	return HSHK(e);
}

sm_event_priority *sm_event_priority_ptr(sm_event *e){
	return PRRT(e);
}

sm_handle *sm_event_handle_ptr(sm_event *e) {
	return HNDL(e);
}

sm_queue *sm_event_home_queue_ptr(sm_event *e) {
	return HOME(e);
}

