/* SM.EXEC 
   Lua wrapper
   anton.bondarenko@gmail.com */


#include "lprefix.h"

#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
/* dlopen() flag */
#define SM_LUA_RTLD_FLAG RTLD_NOW | RTLD_GLOBAL

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "../src/sm_sys.h"
#include "../src/sm_event.h"
#include "../src/sm_queue.h"
#include "../src/sm_pqueue.h"
#include "../src/sm_queue2.h"
#include "../src/sm_app.h"
#include "../src/sm_fsm.h"
#include "../src/sm_state.h"
#include "../src/sm_array.h"

#ifndef SM_DEBUG
#define SM_DEBUG 
#endif

/************************
 ******* SM.EVENT *******
 ************************/

typedef struct SMEvent {
	sm_event *native;
	size_t linked;
} SMEvent;

#define check_sm_event(L, POS) (SMEvent *)luaL_checkudata((L), (POS), "sm.event")
#define next(E1) (*((SMEvent **)((E1)->native->next->data \
                               + (E1)->native->next->data_size \
							   - sizeof(SMEvent *))))
#define this(E1) (*((SMEvent **)((E1)->data + (E1)->data_size - sizeof(SMEvent *))))

#define	sm_push_event(L, E) lua_pushfstring((L), "sm_event id %I @ %p : %p -> %p, linked = %I, priority = (%I, %I)\n", \
					(E)->native->id, (E), (E)->native, ((E)->native == NULL) ? NULL : (E)->native->next, \
					(E)->linked, (E)->native->priority[0], (E)->native->priority[1]);

static int push_event_handler(lua_State *L, sm_event *e) {
	if(e == NULL)
		return EXIT_FAILURE;
	if(this(e) != NULL) { 		// handler was already created
		lua_pushlightuserdata(L, e);					// lud
		luaL_getmetatable(L, "sm.event");				// lud mt
		lua_getfield(L, -1, "inventory");				// lud mt inventory
		lua_rotate(L, -3, -1);							// mt inventory lud
	}
	else { 						// create new one
		SMEvent *lua_e = (SMEvent *)lua_newuserdata(L, sizeof(SMEvent));	// ud
		luaL_getmetatable(L, "sm.event");				// ud mt
		lua_getfield(L, -1, "inventory");				// ud mt inventory
		lua_rotate(L, -3, 1);							// inventory ud mt
		lua_setmetatable(L, -2);						// inventory ud
		lua_pushlightuserdata(L, e);					// inventory ud lud
		lua_rotate(L, -2, -1);							// inventory lud ud
		lua_settable(L, -3);							// inventory
		lua_pushlightuserdata(L, e);					// inventory lud	
		lua_e->native = e;
		lua_e->linked = -1;
		this(e) = lua_e;
	}
	lua_gettable(L, -2);								// ... inventory ud	
	return EXIT_SUCCESS;	
}

// stk: id, plsize
static int new_event(lua_State *L) {
	size_t id = (size_t)luaL_checkinteger(L, 1);
	luaL_argcheck(L, id >= 0, 1, "wrong event id");
	size_t plsize = (size_t)luaL_checkinteger(L, 2);
	luaL_argcheck(L, plsize >= 0, 1, "wrong event payload size");
	sm_event *ne = sm_event_create(plsize + sizeof(SMEvent *));
	if(ne == NULL){
		//return luaL_error(L, "@new_event(%I)", (lua_Integer)plsize);
		return luaL_error(L, "@new_event()");
	}
	ne->id = id;
	memset(ne->data, '\0', plsize);	
	this(ne) = NULL;
	if(push_event_handler(L, ne) == EXIT_FAILURE) {
		lua_pushnil(L);
		return 1;
	}
	SMEvent *lua_e = check_sm_event(L, -1);
	lua_e->linked = 0;
	return 1;
}

// stk: e, data
static int set_event(lua_State *L) {	
	SMEvent *e = check_sm_event(L, 1);
	const char *s = luaL_checkstring(L, 2);
	strncpy((char *)e->native->data, s, e->native->data_size - sizeof(SMEvent *) - 1);
	return 0;
}

// stk: e, id
static int set_event_id(lua_State *L) {	
	SMEvent *e = check_sm_event(L, 1);
	e->native->id = (size_t)luaL_checkinteger(L, 2);
	return 0;
}

// stk: e, priority[0], priority[1]
static int set_event_priority(lua_State *L) {	
	SMEvent *e = check_sm_event(L, 1);
	e->native->priority[0] = (size_t)luaL_checkinteger(L, 2);
	e->native->priority[1] = (size_t)luaL_checkinteger(L, 3);
	return 0;
}

// stk: e
static int get_event(lua_State *L) {	
	SMEvent *e = check_sm_event(L, 1);
	lua_pushinteger(L, e->native->id);
	lua_pushlstring(L, (char *)e->native->data, e->native->data_size - sizeof(SMEvent *));
	return 1;
}

// stk: e
static int get_event_id(lua_State *L) {	
	SMEvent *e = check_sm_event(L, 1);
	lua_pushinteger(L, e->native->id);
	return 1;
}

// stk: e
static int get_event_priority(lua_State *L) {	
	SMEvent *e = check_sm_event(L, 1);
	lua_pushinteger(L, e->native->priority[0]);
	lua_pushinteger(L, e->native->priority[1]);
	return 2;
}

// stk: e
static int event_size(lua_State *L) {	
	SMEvent *e = check_sm_event(L,1 );
	lua_pushinteger(L, (lua_Integer)e->native->data_size - sizeof(SMEvent *));
	return 1;
}	

// stk: e1, e2
static int link_event(lua_State *L) {	
	SMEvent *e1 = check_sm_event(L, 1);
	SMEvent *e2 = check_sm_event(L, 2);
	e1->native->next = e2->native;
	next(e1)->linked++;
	lua_pop(L, 1);
	return 1;
}

// stk: e1
static int unlink_event(lua_State *L) {	
	SMEvent *e1 = check_sm_event(L, 1);
	if(e1->native->next == NULL)
		return 0;
	next(e1)->linked--;
	e1->native->next = NULL;
	lua_pop(L, 1);
	return 0;
}

// stk: e
static int event_tostring(lua_State *L) {	
	SMEvent *e = check_sm_event(L, 1);
	sm_push_event(L, e);
/*	lua_pushfstring(L, "sm_event id %I @ %p : %p -> %p, linked = %I, priority = (%I, %I)", 
					e->native->id, e, e->native, (e->native == NULL) ? NULL : e->native->next,
					e->linked, e->native->priority[0], e->native->priority[1]);*/
	return 1;
}

// stk: e
static int next_event(lua_State *L) {
	SMEvent *e = check_sm_event(L, 1);
	if(e->native == NULL || e->native->next == NULL) {
		lua_pushnil(L);
		return 1;
	}
	if(push_event_handler(L, e->native->next) == EXIT_FAILURE) {
		lua_pushnil(L);
		return 1;
	}
	if(next(e)->linked == -1)
		next(e)->linked = 1;
	return 1;
}

// stk: e
static int collect_event(lua_State *L) {
#ifdef SM_DEBUG	
	printf("collect_event ...");
#endif	
	SMEvent *e = check_sm_event(L, 1);
	if(e->native == NULL) // is it a childfree? 
		return 0;
	if(e->linked > 0) { // is it linked-in otherwise?
		this(e->native) = NULL; // no handler from now on
		return 0;
	}
	if(e->native->next != NULL) // is it linked-out?
		next(e)->linked--;
	sm_event_free(e->native);
	e->native = NULL; // for surviving or ressurected handler
#ifdef SM_DEBUG	
	printf(" ok\n");
#endif
	return 0;
}
	
static const struct luaL_Reg smevent_m [] = {
	{"set", set_event},
	{"get", get_event},
	{"setid", set_event_id},
	{"getid", get_event_id},
	{"setpriority", set_event_priority},
	{"getpriority", get_event_priority},
	{"__concat", link_event},
	{"__unm", unlink_event},
	{"next", next_event},
	{"__len", event_size},
	{"__tostring", event_tostring},
	{"__gc", collect_event},
	{NULL, NULL}
};

/************************
 ******* SM.QUEUE *******
 ************************/

typedef struct SMQueue {
	sm_queue *native;
} SMQueue;

#define check_sm_queue(L) (SMQueue *)luaL_checkudata(L, 1, "sm.queue")

// stk: qsize, plsize, sync
static int new_queue(lua_State *L) {
	size_t qsize = (size_t)luaL_checkinteger(L, 1);
	luaL_argcheck(L, qsize >= 0, 1, "wrong queue size");
	size_t plsize = (size_t)luaL_checkinteger(L, 2);
	luaL_argcheck(L, plsize >= 0, 1, "wrong event payload size");
	bool sync;
	if(lua_isboolean(L, 3))
		 sync = (bool)lua_toboolean(L, 3);
	else
		return luaL_error(L, "wrong synchronized flag value");
	SMQueue *q = (SMQueue *)lua_newuserdata(L, sizeof(SMQueue));
	q->native = sm_queue_create(plsize + sizeof(SMEvent *), qsize, sync);
	if(q->native == NULL){
		//return luaL_error(L, "@new_queue(%I)", (lua_Integer)plsize);
		return luaL_error(L, "@new_queue()");
	}
	luaL_getmetatable(L, "sm.queue");
	lua_setmetatable(L, -2);
	sm_event *e = sm_queue_top(q->native);
	while(e->next != NULL) {
		memset(e->data, '\0', plsize);	
		*((SMEvent **)(e->data + plsize)) = NULL;
		e = e->next;
	}
	return 1;
}

// stk: q
static int queue_size(lua_State *L) {	
	SMQueue *q = check_sm_queue(L);
	lua_pushinteger(L, (lua_Integer)q->native->size);
	return 1;
}	

// stk: q
static int queue_tostring(lua_State *L) {
	SMQueue *q = check_sm_queue(L);
	luaL_Buffer b;
	luaL_buffinit(L, &b);
	lua_pushfstring(L, "sm_queue @ %p, size = %I, synchronized = %s\nEvents:\n", 
					q, (lua_Integer)q->native->size, 
					q->native->synchronized ? "true" : "false");
	luaL_addvalue(&b);
	sm_event *e = sm_queue_top(q->native);
	SMEvent *lua_e;
	while(e != NULL) {
		lua_e = this(e);
		sm_push_event(L, lua_e);
/*		lua_pushfstring(L, "sm_event id %I @ %p : %p -> %p, linked = %I, priority = (%I, %I)", 
					e->native->id, e, e->native, (e->native == NULL) ? NULL : e->native->next,
					e->linked, e->native->priority[0], e->native->priority[1]);	*/
/*		lua_pushfstring(L, "sm_event @ %p : %p -> %p, linked = %I\n", 
						lua_e, e, e->next, 
						(lua_e == NULL) ? -1 : lua_e->linked);*/
		luaL_addvalue(&b);
		e = e->next;
	}
	luaL_pushresult(&b);
	return 1;
}

// stk: q
static int collect_queue(lua_State *L) {
	SMQueue *q = check_sm_queue(L);
	sm_event *e = sm_queue_dequeue(q->native);
	while(e != NULL){
		if(this(e) == NULL)
			sm_event_free(e);
		else
			this(e)->linked--;
		e = sm_queue_dequeue(q->native);
	}
	sm_queue_free(q->native);
	return 0;
}

// stk: q
static int queuetop(lua_State *L) {
	SMQueue *q = check_sm_queue(L);
	sm_event *top_e = sm_queue_top(q->native);	
	if(top_e == NULL) {
		lua_pushnil(L);
		return 1;
	}
	if(push_event_handler(L, top_e) == EXIT_FAILURE) {
		lua_pushnil(L);
		return 1;
	}
	this(top_e)->linked = 1;
	return 1;
}

// stk: q
static int dequeue(lua_State *L) {
	SMQueue *q = check_sm_queue(L);
	sm_event *top_e = sm_queue_dequeue(q->native);	
	if(top_e == NULL) {
		lua_pushnil(L);
		return 1;
	}
	if(push_event_handler(L, top_e) == EXIT_FAILURE) {
		lua_pushnil(L);
		return 1;
	}
	this(top_e)->linked = 1;
	return 1;
}

// stk: q, e
static int enqueue(lua_State *L) {
	SMQueue *q = check_sm_queue(L);
	if(q == NULL) {
		return 0;
	}
	SMEvent *e = check_sm_event(L, 2);	
	if(e == NULL) {
		return 0;
	}
	sm_queue_enqueue(e->native, q->native);
	e->linked++;
	return 0;
}

static const struct luaL_Reg smqueue_m [] = {
	{"top", queuetop},
	{"enqueue", enqueue},
	{"dequeue", dequeue},
	{"__len", queue_size},
	{"__tostring", queue_tostring},
	{"__gc", collect_queue},
	{NULL, NULL}
};

/************************
 ******* SM.PQUEUE *******
 ************************/

typedef struct SMPQueue {
	sm_pqueue *native;
} SMPQueue;

#define check_sm_pqueue(L) (SMPQueue *)luaL_checkudata(L, 1, "sm.pqueue")

// stk: qsize, plsize, sync
static int new_pqueue(lua_State *L) {
	size_t cap = (size_t)luaL_checkinteger(L, 1);
	luaL_argcheck(L, cap >= 0, 1, "wrong capacity value");
	bool sync;
	if(lua_isboolean(L, 2))
		 sync = (bool)lua_toboolean(L, 3);
	else
		return luaL_error(L, "wrong synchronized flag value");
	SMPQueue *pq = (SMPQueue *)lua_newuserdata(L, sizeof(SMPQueue));
	pq->native = sm_pqueue_create(cap, sync);
	if(pq->native == NULL)
		return luaL_error(L, "@new_pqueue()");
	luaL_getmetatable(L, "sm.pqueue");
	lua_setmetatable(L, -2);
	return 1;
}

// stk: q
static int pqueue_size(lua_State *L) {	
	SMPQueue *pq = check_sm_pqueue(L);
	lua_pushinteger(L, (lua_Integer)pq->native->size);
	return 1;
}	


// stk: q
static int pqueue_tostring(lua_State *L) {
	SMPQueue *pq = check_sm_pqueue(L);
	luaL_Buffer b;
	luaL_buffinit(L, &b);
	lua_pushfstring(L, 
					"sm_pqueue @ %p, size = %I, capacity = %I, synchronized = %s\nEvents:\n", 
					pq, (lua_Integer)pq->native->size, (lua_Integer)pq->native->capacity, 
					pq->native->synchronized ? "true" : "false");
	luaL_addvalue(&b);
	sm_event *e;
	SMEvent *lua_e;
	for(size_t i = 0; i < pq->native->size; i++) {
		e = pq->native->heap[i];
		lua_e = this(e);
		sm_push_event(L, lua_e);
/*		lua_pushfstring(L, "sm_event id %I @ %p : %p -> %p, linked = %I, priority = (%I, %I)", 
					e->native->id, e, e->native, (e->native == NULL) ? NULL : e->native->next,
					e->linked, e->native->priority[0], e->native->priority[1]);*/		
/*		lua_pushfstring(L, "sm_event @ %p : %p -> %p, linked = %I\n", 
						lua_e, e, e->next, 
						(lua_e == NULL) ? -1 : lua_e->linked);*/
		luaL_addvalue(&b);
	}	
	luaL_pushresult(&b);
	return 1;
}

// stk: q
static int collect_pqueue(lua_State *L) {
	SMPQueue *pq = check_sm_pqueue(L);
	sm_event *e;
	for(size_t i = 0; i < pq->native->size; i++) {
		e = sm_pqueue_dequeue(pq->native);
		if(this(e) == NULL)
			sm_event_free(e);
		else
			this(e)->linked--;
	}
	sm_pqueue_free(pq->native);
	return 0;
}

// stk: q
static int pqueuetop(lua_State *L) {
	SMPQueue *pq = check_sm_pqueue(L);
	sm_event *e = sm_pqueue_top(pq->native);	
	if(e == NULL) {
		lua_pushnil(L);
		return 1;
	}
	if(push_event_handler(L, e) == EXIT_FAILURE) {
		lua_pushnil(L);
		return 1;
	}
	this(e)->linked = 1;
	return 1;
}

// stk: q
static int pdequeue(lua_State *L) {
	SMPQueue *pq = check_sm_pqueue(L);
	sm_event *e = sm_pqueue_dequeue(pq->native);	
	if(e == NULL) {
		lua_pushnil(L);
		return 1;
	}
	if(push_event_handler(L, e) == EXIT_FAILURE) {
		lua_pushnil(L);
		return 1;
	}
	this(e)->linked = 1;
	return 1;
}

// stk: q, e
static int penqueue(lua_State *L) {
	SMPQueue *pq = check_sm_pqueue(L);
	if(pq == NULL) {
		return 0;
	}
	SMEvent *e = check_sm_event(L, 2);	
	if(e == NULL) {
		return 0;
	}
	sm_pqueue_enqueue(e->native, pq->native);
	e->linked++;
	return 0;
}

static const struct luaL_Reg smpqueue_m [] = {
	{"top", pqueuetop},
	{"enqueue", penqueue},
	{"dequeue", pdequeue},
	{"__len", pqueue_size},
	{"__tostring", pqueue_tostring},
	{"__gc", collect_pqueue},
	{NULL, NULL}
};

/************************
 ******* SM.QUEUE2 *******
 ************************/

typedef struct SMQueue2 {
	sm_queue2 *native;
} SMQueue2;

#define check_sm_queue2(L, POS) (SMQueue2 *)luaL_checkudata((L), (POS), "sm.queue2")

// stk: 
static int new_queue2(lua_State *L) {
/*	size_t qsize = (size_t)luaL_checkinteger(L, 1);
	luaL_argcheck(L, qsize >= 0, 1, "wrong queue2 size");
	size_t plsize = (size_t)luaL_checkinteger(L, 2);
	luaL_argcheck(L, plsize >= 0, 1, "wrong event payload size"); */
	SMQueue2 *q = (SMQueue2 *)lua_newuserdata(L, sizeof(SMQueue2));
	q->native = sm_queue2_create(/*plsize + sizeof(SMEvent *), qsize*/);
	if(q->native == NULL)
		//return luaL_error(L, "@malloc(%I)", (lua_Integer)plsize);
		return luaL_error(L, "@new_queue2()");
	luaL_getmetatable(L, "sm.queue2");
	lua_setmetatable(L, -2);
/*	sm_event *e = sm_queue2_get(q->native);
	while(e->next != NULL) {
		memset(e->data, '\0', plsize);	
		*((SMEvent **)(e->data + plsize)) = NULL;
		e = e->next;
	} */
	return 1;
}

// stk: q2
static int queue2_size(lua_State *L) {	
	SMQueue2 *q = check_sm_queue2(L, 1); (void) q;
	lua_pushnil(L); //wtf? - see above
	return 1;
}	

// stk: q2
static int queue2_tostring(lua_State *L) {
	SMQueue2 *q = check_sm_queue2(L, 1);
	luaL_Buffer b;
	luaL_buffinit(L, &b);
	lua_pushfstring(L, "sm_queue2 @ %p\n", q);
	luaL_addvalue(&b);
	lua_pushfstring(L, "Lower priority events\n");
	luaL_addvalue(&b);
	SMEvent *lua_e;
	sm_event *e = sm_queue2_get(q->native);
	while(e != NULL) {
		lua_e = this(e);
		sm_push_event(L, lua_e);
/*		lua_pushfstring(L, "sm_event id %I @ %p : %p -> %p, linked = %I, priority = (%I, %I)", 
					e->native->id, e, e->native, (e->native == NULL) ? NULL : e->native->next,
					e->linked, e->native->priority[0], e->native->priority[1]);	*/
/*		lua_pushfstring(L, "sm_event @ %p : %p -> %p, linked = %I\n", 
						lua_e, e, e->next, 
						(lua_e == NULL) ? -1 : lua_e->linked);*/
		luaL_addvalue(&b);
		e = e->next;
	}
	lua_pushfstring(L, "Higher priority events\n");
	luaL_addvalue(&b);
	e = sm_queue2_get_high(q->native);
	while(e != NULL) {
		lua_e = this(e);
		sm_push_event(L, lua_e);
/*		lua_pushfstring(L, "sm_event @ %p : %p -> %p, linked = %I\n", 
						lua_e, e, e->next, 
						(lua_e == NULL) ? -1 : lua_e->linked);*/
		luaL_addvalue(&b);
		e = e->next;
	}
	luaL_pushresult(&b);
	return 1;
}

// stk: q2
static int collect_queue2(lua_State *L) {
	SMQueue2 *q = check_sm_queue2(L, 1);
	sm_event *e = sm_dequeue2(q->native);
	while(e != NULL){
		if(this(e) == NULL)
			sm_event_free(e);
		else
			this(e)->linked--;
		e = sm_dequeue2(q->native);
	}
	sm_queue2_free(q->native);
	return 0;
}

// stk: q2
static int queue2_get(lua_State *L) {
	SMQueue2 *q = check_sm_queue2(L, 1);
	sm_event *top_e = sm_queue2_get(q->native);	
	if(top_e == NULL) {
		lua_pushnil(L);
		return 1;
	}	
	if(push_event_handler(L, top_e) == EXIT_FAILURE) {
		lua_pushnil(L);
		return 1;
	}
	this(top_e)->linked = 1;
	return 1;
}

// stk: q2
static int queue2_get_high(lua_State *L) {
	SMQueue2 *q = check_sm_queue2(L, 1);
	sm_event *top_e = sm_queue2_get_high(q->native);	
	if(top_e == NULL) {
		lua_pushnil(L);
		return 1;
	}	
	if(push_event_handler(L, top_e) == EXIT_FAILURE) {
		lua_pushnil(L);
		return 1;
	}
	this(top_e)->linked = 1;
	return 1;
}

// stk: q2
static int dequeue2(lua_State *L) {
	SMQueue2 *q = check_sm_queue2(L, 1);
	sm_event *top_e = sm_dequeue2(q->native);	
	if(top_e == NULL) {
		lua_pushnil(L);
		return 1;
	}
	if(push_event_handler(L, top_e) == EXIT_FAILURE) {
		lua_pushnil(L);
		return 1;
	}
	this(top_e)->linked = 1;
	return 1;
}

// stk: q2
static int lock_dequeue2(lua_State *L) {
	SMQueue2 *q = check_sm_queue2(L, 1);
	sm_event *top_e = sm_lock_dequeue2(q->native);	
	if(top_e == NULL) {
		lua_pushnil(L);
		return 1;
	}
	if(push_event_handler(L, top_e) == EXIT_FAILURE) {
		lua_pushnil(L);
		return 1;
	}
	this(top_e)->linked = 1;
	return 1;
}

// stk: q2, e
static int enqueue2(lua_State *L) {
	SMQueue2 *q = check_sm_queue2(L, 1);
	if(q == NULL) {
		return 0;
	}
	SMEvent *e = check_sm_event(L, 2);	
	if(e == NULL) {
		return 0;
	}
	sm_enqueue2(e->native, q->native);
	e->linked++;
	return 0;
}

// stk: q2, e
static int enqueue2_high(lua_State *L) {
	SMQueue2 *q = check_sm_queue2(L, 1);
	if(q == NULL) {
		return 0;
	}
	SMEvent *e = check_sm_event(L, 2);	
	if(e == NULL) {
		return 0;
	}
	sm_enqueue2_high(e->native, q->native);
	e->linked++;
	return 0;
}

// stk: q2, e
static int lock_enqueue2(lua_State *L) {
	SMQueue2 *q = check_sm_queue2(L, 1);
	if(q == NULL) {
		return 0;
	}
	SMEvent *e = check_sm_event(L, 2);	
	if(e == NULL) {
		return 0;
	}
	sm_lock_enqueue2(e->native, q->native);
	e->linked++;
	return 0;
}

// stk: q2, e
static int lock_enqueue2_high(lua_State *L) {
	SMQueue2 *q = check_sm_queue2(L, 1);
	if(q == NULL) {
		return 0;
	}
	SMEvent *e = check_sm_event(L, 2);	
	if(e == NULL) {
		return 0;
	}
	sm_lock_enqueue2_high(e->native, q->native);
	e->linked++;
	return 0;
}

static const struct luaL_Reg smqueue2_m [] = {
	{"get", queue2_get},
	{"gethigh", queue2_get_high},
	{"enqueue", enqueue2},
	{"lockenqueue", lock_enqueue2},
	{"enqueuehigh", enqueue2_high},
	{"lockenqueuehigh", lock_enqueue2_high},
	{"dequeue", dequeue2},
	{"lockdequeue", lock_dequeue2},
	{"__len", queue2_size},
	{"__tostring", queue2_tostring},
	{"__gc", collect_queue2},
	{NULL, NULL}
};

/************************
 ******* SM.APP *********
 ************************/

typedef struct SMApp {
	sm_app native;
} SMApp;

#define check_sm_app(L, POS) (SMApp *)luaL_checkudata((L), (POS), "sm.app")
#define test_sm_app(L, POS) (SMApp *)luaL_testudata((L), (POS), "sm.app")

static int push_app_handler(lua_State *L, sm_app a) {
	if(a == NULL)
		return EXIT_FAILURE;
	luaL_getmetatable(L, "sm.app");						// mt
	lua_getfield(L, -1, "inventory");					// mt inventory
	lua_pushlightuserdata(L, a);						// mt inventory lud
	lua_gettable(L, -2);								// mt inventory ud?
	if(test_sm_app(L, 1) == NULL) { // create handler if not found
		lua_pop(L, 1);									// mt inventory
		SMApp *lua_a = (SMApp *)lua_newuserdata(L, sizeof(SMApp)); // mt inventory ud
		lua_rotate(L, -3, -1);							// inventory ud mt
		lua_setmetatable(L, -2);						// inventory ud
		lua_pushvalue(L, -1);							// inventory ud ud
		lua_pushlightuserdata(L, a);					// inventory ud ud lud
		lua_rotate(L, -2, -1);							// inventory ud lud ud
		lua_settable(L, -4);							// inventory ud
		lua_a->native = a;
	}	
	return EXIT_SUCCESS;	
}

// stk: handle, app_name
static int lookup_app(lua_State *L) { // not too much reentrant for the same app ...
	void *handle; 
	if((handle = lua_touserdata(L, 1)) == NULL)
		return luaL_error(L, "@lookup_app: lua_touserdata() returns NULL");
	const char *name = luaL_checkstring(L, 2);
	sm_app a = dlsym(handle, name);
	if(push_app_handler(L, a) != EXIT_SUCCESS) // love this function name))
		lua_pushnil(L);
	return 1;
}

// stk: app
static int app_tostring(lua_State *L) {	
	SMApp *a = check_sm_app(L, 1);
	lua_pushfstring(L, "sm_app @ %p", a->native);
	return 1;
}

// stk: app, e
static int call_app(lua_State *L) {	
	SMApp *a = check_sm_app(L, 1);
	SMEvent *e = check_sm_event(L, 2);
	SMState *s = check_sm_state(L, 3);
	(*(a->native))(e->native, s->native);
	return 0;
}

// stk: app
static int collect_app(lua_State *L) {	
#ifdef SM_DEBUG	
	printf("collect_app ...");
#endif	
	SMApp *app = check_sm_app(L, 1);
	if(app == NULL) {
#ifdef SM_DEBUG		
		printf(" already\n");
#endif
		return 0;
	}
#ifdef SM_DEBUG	
	printf(" ok\n");
#endif	
	return 0;
}	

static const struct luaL_Reg smapp_m [] = {
	{"__tostring", app_tostring},
	{"__call", call_app},
	{"__gc", collect_app},
	{NULL, NULL}
};

/************************
 **** SM.APP_TABLE ******
 ************************/

// stk: file
static int load_applib(lua_State *L) {
	const char *fn = luaL_checkstring(L, 1);
	lua_pushlightuserdata(L, dlopen(fn, SM_LUA_RTLD_FLAG));
	return 1;
}

typedef struct SMAppTable {
	sm_app_table *native;
} SMAppTable;

#define check_sm_apptab(L, POS) (SMAppTable *)luaL_checkudata((L), (POS), "sm.apptable")

// stk:
static int new_apptab(lua_State *L) {
	SMAppTable *at = (SMAppTable *)lua_newuserdata(L, sizeof(SMAppTable));
	at->native = sm_app_table_create();
	luaL_getmetatable(L, "sm.apptable");
	lua_setmetatable(L, -2);
	return 1;
}	

// stk: at, app, name
static int apptab_set(lua_State *L) {
	SMAppTable *at = check_sm_apptab(L, 1);
	SMApp *a = check_sm_app(L, 2);
	const char *name = luaL_checkstring(L, 3);
	at->native = sm_app_table_set(at->native, name, a->native);
	return 0;
}	

// stk: at, name
static int apptab_get(lua_State *L) {
	SMAppTable *at = check_sm_apptab(L, 1);
	const char *name = luaL_checkstring(L, 2);
	sm_app na = *(sm_app_table_get_ref(at->native, name));
	if(push_app_handler(L, na) != EXIT_SUCCESS) 
		lua_pushnil(L);
	return 1;
}

// stk: at, name
static int apptab_remove(lua_State *L) {
	SMAppTable *at = check_sm_apptab(L, 1);
	const char *name = luaL_checkstring(L, 2);
	sm_app_table_remove(at->native, (char *)name);
	return 0;
}	

// stk: at, name
static int apptab_size(lua_State *L) {	
	SMAppTable *lat = check_sm_apptab(L, 1);
	if(lat == NULL){
		lua_pushinteger(L, (lua_Integer)0);
		return 1;
	}
	size_t s = 0;
	sm_app_table *at = lat->native;
	while(at != NULL) {
		s++;
		at = at->next;
	}
	lua_pushinteger(L, (lua_Integer)s);
	return 1;
}	

// stk: at
static int apptab_tostring(lua_State *L) {	
	SMAppTable *ac = check_sm_apptab(L, 1);
	size_t s = 0;
	luaL_Buffer b;
	luaL_buffinit(L, &b);
	lua_pushfstring(L, "sm_app_table @ %p\n", ac);
	luaL_addvalue(&b);
	lua_pushfstring(L, "Applications\n");
	luaL_addvalue(&b);
	sm_app_table *at = ac->native;
	while(at != NULL) {
		s++;
		lua_pushfstring(L, "sm_app %s @ %p -> %p\n", at->name, at->ref, at->app);
		luaL_addvalue(&b);
		at = at->next;
	}
	lua_pushfstring(L, "sm_app_table size: %I\n", (lua_Integer)s);
	luaL_addvalue(&b);
	luaL_pushresult(&b);
	return 1;
}	

// stk: at
static int collect_apptab(lua_State *L) {
#ifdef SM_DEBUG	
	printf("collect_apptab ...");
#endif	
	SMAppTable *at = check_sm_apptab(L, 1);
	if(at == NULL)
		return 0;
	sm_app_table_free(at->native);
#ifdef SM_DEBUG	
	printf(" ok\n");
#endif	
	return 0;
}	

static const struct luaL_Reg smapptable_m [] = {
	{"set", apptab_set},
	{"get", apptab_get},
	{"remove", apptab_remove},
	{"__len", apptab_size},
	{"__tostring", apptab_tostring},
	{"__gc", collect_apptab},
	{NULL, NULL}
};
	
/**********************
 ******* SM.FSM *******
 **********************/

typedef struct SMFSM {
	sm_fsm *native;
	bool lock;
} SMFSM;

#define check_sm_fsm(L, POS) (SMFSM *)luaL_checkudata((L), (POS), "sm.fsm")
#define test_sm_fsm(L, POS) (SMFSM *)luaL_testudata((L), (POS), "sm.fsm")

static int push_fsm_handler(lua_State *L, sm_fsm *f) {
	if(f == NULL)
		return EXIT_FAILURE;
	luaL_getmetatable(L, "sm.fsm");						// mt
	lua_getfield(L, -1, "inventory");					// mt inventory
	lua_pushlightuserdata(L, f);						// mt inventory lud
	lua_gettable(L, -2);								// mt inventory ud?
	if(test_sm_fsm(L, 1) == NULL) { // create handler if not found
		lua_pop(L, 1);									// mt inventory
		SMFSM *lua_f = (SMFSM *)lua_newuserdata(L, sizeof(SMFSM)); // mt inventory ud
		lua_rotate(L, -3, -1);							// inventory ud mt
		lua_setmetatable(L, -2);						// inventory ud
		lua_pushvalue(L, -1);							// inventory ud ud
		lua_pushlightuserdata(L, f);					// inventory ud ud lud
		lua_rotate(L, -2, -1);							// inventory ud lud ud
		lua_settable(L, -4);							// inventory ud
		lua_f->native = f;
		lua_f->lock = false;
	}	
	return EXIT_SUCCESS;	
}

// stk: fsmj, at, type
// type = {"mealy" | "moore"}
static int new_fsm(lua_State *L) {
	const char *fsmj = luaL_checkstring(L, 1);
	SMAppTable *at = check_sm_apptab(L, 2);
	const char *mm = luaL_checkstring(L, 3);
	//SMFSM *f = (SMFSM *)lua_newuserdata(L, sizeof(SMFSM));
	sm_fsm_type t;
	if(!strcmp(mm, "mealy")) 
		t = SM_MEALY; 
	else { 
		if (!strcmp(mm, "moore")) 
			t = SM_MOORE;
		else 
			return luaL_error(L, "wrong automata type: %s", mm);
	}
	sm_fsm *f = sm_fsm_create(fsmj, at->native, t);
	if(f == NULL)
		return luaL_error(L, "@sm_fsm_create()");
	if(push_fsm_handler(L, f) != EXIT_SUCCESS)
		lua_pushnil(L);
	return 1;
}	

// stk: fsm
static int unlock_fsm(lua_State *L) {
	SMFSM *f = check_sm_fsm(L, 1);
	if(f == NULL)
		return 0;
	f->lock = false;
	return 0;
}	

// stk: fsm
static int collect_fsm(lua_State *L) {
#ifdef SM_DEBUG	
	printf("collect_fsm ...");
#endif	
	SMFSM *f = check_sm_fsm(L, 1);
	if(f == NULL)
		return 0;
	if(f->lock)
		return 0;
	sm_fsm_free(f->native);
#ifdef SM_DEBUG	
	printf(" ok\n");
#endif	
	return 0;
}	

// stk: fsm
static int fsm_tostring(lua_State *L) {	
	SMFSM *f = check_sm_fsm(L, 1);
	lua_pushfstring(L, "sm_fsm %p -> %p, linked = %s\n%s\n", f, f->native, 
					f->lock?"true":"false", sm_fsm_to_string(f->native));
	return 1;
}	

static const struct luaL_Reg smfsm_m [] = {
	{"__tostring", fsm_tostring},
	{"unlock", unlock_fsm},
	{"__gc", collect_fsm},
	{NULL, NULL}
};


/**********************
 **** SM.FSM_TABLE ****
 **********************/

typedef struct SMFSMTable {
	sm_fsm_table *native;
} SMFSMTable;

#define check_sm_fsmtab(L, POS) (SMFSMTable *)luaL_checkudata((L), (POS), "sm.fsmtable")

// stk:
static int new_fsmtab(lua_State *L) {
	SMFSMTable *ft = (SMFSMTable *)lua_newuserdata(L, sizeof(SMFSMTable));
	ft->native = sm_fsm_table_create();
	luaL_getmetatable(L, "sm.fsmtable");
	lua_setmetatable(L, -2);
	return 1;
}	

// stk: ft, fsm, name
static int fsmtab_set(lua_State *L) {
	SMFSMTable *ft = check_sm_fsmtab(L, 1);
	SMFSM *f = check_sm_fsm(L, 2);
	const char *name = luaL_checkstring(L, 3);
	ft->native = sm_fsm_table_set(ft->native, name, f->native);
	f->lock = true;
	return 0;
}	

// stk: ft, name
static int fsmtab_get(lua_State *L) {
	SMFSMTable *ft = check_sm_fsmtab(L, 1);
	const char *name = luaL_checkstring(L, 2);
	sm_fsm *f = *(sm_fsm_table_get_ref(ft->native, name));
	if(push_fsm_handler(L, f) != EXIT_SUCCESS) 
		lua_pushnil(L);
	return 1;
}

// stk: ft, name
static int fsmtab_remove(lua_State *L) {
	SMFSMTable *ft = check_sm_fsmtab(L, 1);
	const char *name = luaL_checkstring(L, 2);
	sm_fsm_table_remove(ft->native, (char *)name);
	return 0;
}	

// stk: ft, name
static int fsmtab_size(lua_State *L) {	
	SMFSMTable *ft = check_sm_fsmtab(L, 1);
	if(ft == NULL){
		lua_pushinteger(L, (lua_Integer)0);
		return 1;
	}
	size_t s = 0;
	sm_fsm_table *nft = ft->native;
	while(nft != NULL) {
		s++;
		nft = nft->next;
	}
	lua_pushinteger(L, (lua_Integer)s);
	return 1;
}

// stk: ft
static int fsmtab_tostring(lua_State *L) {	
	SMFSMTable *ft = check_sm_fsmtab(L, 1);
	size_t s = 0;
	luaL_Buffer b;
	luaL_buffinit(L, &b);
	lua_pushfstring(L, "sm_fsm_table @ %p\n", ft);
	luaL_addvalue(&b);
	lua_pushfstring(L, "State machines:\n");
	luaL_addvalue(&b);
	sm_fsm_table *nft = ft->native;
	while(nft != NULL) {
		s++;
		lua_pushfstring(L, "sm_fsm %s @ %p -> %p\n", nft->name, nft->ref, *nft->ref);
		luaL_addvalue(&b);
		nft = nft->next;
	}
	lua_pushfstring(L, "sm_fsm_table size: %I\n", (lua_Integer)s);
	luaL_addvalue(&b);
	luaL_pushresult(&b);
	return 1;
}	

// stk: ft
static int collect_fsmtab(lua_State *L) {
#ifdef SM_DEBUG	
	printf("collect_fsmtab ...");
#endif	
	SMFSMTable *ft = check_sm_fsmtab(L, 1);
	if(ft == NULL)
		return 0;
	sm_fsm_table_free(ft->native);
#ifdef SM_DEBUG	
	printf(" ok\n");
#endif	
	return 0;
}	

static const struct luaL_Reg smfsmtable_m [] = {
	{"set", fsmtab_set},
	{"get", fsmtab_get},
	{"remove", fsmtab_remove},
	{"__len", fsmtab_size},
	{"__tostring", fsmtab_tostring},
	{"__gc", collect_fsmtab},
	{NULL, NULL}
};

/**********************
 ****** SM.STATE ******
 **********************/

typedef struct SMState {
	sm_state *native;
	bool standalone;
} SMState;

#define check_sm_state(L, POS) (SMState *)luaL_checkudata((L), (POS), "sm.state")
#define test_sm_state(L, POS) (SMState *)luaL_testudata((L), (POS), "sm.state")

static int push_state_handler(lua_State *L, sm_state *s) {
	if(s == NULL)
		return EXIT_FAILURE;
	luaL_getmetatable(L, "sm.state");					// mt
	lua_getfield(L, -1, "inventory");					// mt inventory
	lua_pushlightuserdata(L, s);						// mt inventory lud
	lua_gettable(L, -2);								// mt inventory ud?
	if(test_sm_state(L, 1) == NULL) { // create handler if not found
		lua_pop(L, 1);									// mt inventory
		SMState *lua_s = (SMState *)lua_newuserdata(L, sizeof(SMState)); // mt inventory ud
		lua_rotate(L, -3, -1);							// inventory ud mt
		lua_setmetatable(L, -2);						// inventory ud
		lua_pushvalue(L, -1);							// inventory ud ud
		lua_pushlightuserdata(L, s);					// inventory ud ud lud
		lua_rotate(L, -2, -1);							// inventory ud lud ud
		lua_settable(L, -4);							// inventory ud
		lua_s->native = s;
	}	
	return EXIT_SUCCESS;	
}

// stk: fsm plsize
static int new_state(lua_State *L) {
	SMFSM *fsm = check_sm_fsm(L, 1);
	size_t plsize = (size_t)lua_tointeger(L, 2);
	sm_state *s = sm_state_create(fsm->native->ref, plsize);
	fsm->lock = true;
	if(s == NULL)
		return luaL_error(L, "@new_state()");
	if(push_state_handler(L, s) != EXIT_SUCCESS) {
		lua_pushnil(L);
		return 1;
	}
	SMState *lua_s = check_sm_state(L, -1);;
	lua_s->standalone = true;
	return 1;
}	

// stk: state event
static int state_trace_add(lua_State *L) {	
	SMEvent *e = check_sm_event(L, 2);
	if(e->native == NULL)
		return luaL_error(L, "empty event handler");
	if(e->linked > 0) 
		return luaL_error(L, "event is already liked in");
	if(e->native->next != NULL) 
		return luaL_error(L, "event is already liked out");
	SMState *s = check_sm_state(L, 1);
	if(s == NULL || s->native == NULL) // empty or childless
		return luaL_error(L, "empty state handler");
	e->native->next = s->native->trace;
	s->native->trace = e->native;
	return 0;
}

// stk: state
static int state_trace_get(lua_State *L) {	
	SMState *s = check_sm_state(L, 1);
	if(s == NULL || s->native == NULL) // empty or childless
		return luaL_error(L, "empty state handler");
	sm_event *top_e = s->native->trace;
	if(push_event_handler(L, top_e) == EXIT_FAILURE) {
		lua_pushnil(L);
		return 1;
	}
	this(top_e)->linked = 1;
	return 1;
}

// stk: state data
static int state_set(lua_State *L) {	
	SMState *s = check_sm_state(L, 1);
	if(s == NULL || s->native == NULL) // empty or childless
		return luaL_error(L, "empty state handler");
	const char *data = luaL_checkstring(L, 2);
	memset(s->native->data, '\0', s->native->data_size);
	char *d = (char *)s->native->data;
	memcpy(d, data, MIN(strlen(data), s->native->data_size));
	return 0;
}

// stk: state
static int state_get(lua_State *L) {	
	SMState *s = check_sm_state(L, 1);
	if(s == NULL || s->native == NULL) // empty or childless
		return luaL_error(L, "empty state handler");
	lua_pushlstring(L, (char *)s->native->data, s->native->data_size);
	return 1;
}

// stk: state keystring
static int state_set_key(lua_State *L) {	
	SMState *s = check_sm_state(L, 1);
	if(s == NULL || s->native == NULL) // empty or childless
		return luaL_error(L, "empty state handler");
	const char *key = luaL_checkstring(L, 2);
	size_t keylen = luaL_checkinteger(L, 3);
	sm_state_set_key(s->native, key, keylen);
	return 0;
}

// stk: state
static int state_get_key(lua_State *L) {	
	SMState *s = check_sm_state(L, 1);
	if(s == NULL || s->native == NULL) // empty or childless
		return luaL_error(L, "empty state handler");
	lua_pushlstring(L, (char *)s->native->key, s->native->key_length);
	return 1;
}

// stk: state SM_STATE_ID
static int state_set_id(lua_State *L) {	
	SMState *s = check_sm_state(L, 1);
	if(s == NULL || s->native == NULL) // empty or childless
		return luaL_error(L, "empty state handler");
	s->native->id = luaL_checkinteger(L, 2);
	return 0;
}

// stk: state
static int state_get_id(lua_State *L) {	
	SMState *s = check_sm_state(L, 1);
	if(s == NULL || s->native == NULL) // empty or childless
		return luaL_error(L, "empty state handler");
	lua_pushinteger(L, s->native->id);
	return 1;
}

// stk: state
static int collect_state(lua_State *L) {
#ifdef SM_DEBUG	
	printf("collect_state ...");
#endif	
	SMState *s = check_sm_state(L, 1);
	if(s == NULL || s->native == NULL) // empty or childless
		return 0;
	if(s->standalone)
		sm_state_free(s->native);
	s->native = NULL;
#ifdef SM_DEBUG	
	printf(" ok\n");
#endif	
	return 0;
}

// stk: state
static int purge_state(lua_State *L) {	
	SMState *s = check_sm_state(L, 1);
	if(s == NULL)
		return 0;
	sm_state_purge(s->native);
	s->standalone = false;
	return 0;
}

static int apply_event(lua_State *L) {
	SMState *s = check_sm_state(L, 1);
	if(s == NULL)
		return 0;
	SMEvent *e = (SMEvent *)luaL_checkudata(L, 2, "sm.event");
	if(e == NULL)
		return 0;
	sm_apply_event(s->native, e->native);
	return 0;
}	

// stk: state
static int state_tostring(lua_State *L) {	
	SMState *s = check_sm_state(L, 1);
	luaL_Buffer b;
	luaL_buffinit(L, &b);
	lua_pushfstring(L, "state %p -> %p:\n", s, s->native);
	luaL_addvalue(&b);
	lua_pushfstring(L, "state fsm @ %p", *s->native->fsm);
	luaL_addvalue(&b);
	if(s->native->key_length != 0)
		lua_pushfstring(L, ", key = %s", (char *)s->native->key);
	else
		lua_pushfstring(L, ", key is not set");
	luaL_addvalue(&b);
	lua_pushfstring(L, ", data: ");
	luaL_addvalue(&b);
	if(s->native->data_size != 0)
		lua_pushfstring(L, (char *)s->native->data);
	else
		lua_pushfstring(L, "");
	luaL_addvalue(&b);
	lua_pushfstring(L, "\n");
	luaL_addvalue(&b);
	luaL_pushresult(&b);
	return 1;
}
					
// stk: state
static int state_getid(lua_State *L) {	
	SMState *s = check_sm_state(L, 1);
	lua_pushfstring(L, "state ID: %I\n", (lua_Integer)s->native->id);
	return 1;
}

static const struct luaL_Reg smstate_m [] = {
	{"apply", apply_event},
	{"traceadd", state_trace_add},
	{"traceget", state_trace_get},
	{"getkey", state_get_key},
	{"setkey", state_set_key},
	{"get", state_get},
	{"set", state_set},
	{"getid", state_get_id},
	{"setid", state_set_id},
	{"purge", purge_state},
	{"__tostring", state_tostring},
	{"id", state_getid},
	{"__gc", collect_state},
	{NULL, NULL}
};

/**********************
 ****** SM.ARRAY ******
 **********************/

typedef struct SMArray {
	sm_array *native;
} SMArray;

#define check_sm_array(L, POS) (SMArray *)luaL_checkudata((L), (POS), "sm.array")

static int new_array(lua_State *L) {
	size_t stsize = (size_t)luaL_checkinteger(L, 1);
	luaL_argcheck(L, stsize >= 0, 1, "wrong array stack size");
	size_t plsize = (size_t)luaL_checkinteger(L, 2);
	luaL_argcheck(L, plsize >= 0, 1, "wrong state payload size");
	SMFSM *fsm;
	if(!lua_isnil(L,3))
		fsm = check_sm_fsm(L, 3);
	else
		fsm = NULL;
	bool sync;
	if(lua_isboolean(L, 4))
		 sync = (bool)lua_toboolean(L, 4);
	else
		return luaL_error(L, "wrong synchronized flag value");
	SMArray *a = (SMArray *)lua_newuserdata(L, sizeof(SMArray));
	a->native = sm_array_create(stsize, plsize, fsm->native->ref, sync);
	if(a->native == NULL) {
		//return luaL_error(L, "@new_array(%I)", (lua_Integer)plsize);
		return luaL_error(L, "@new_array()");
	}
	luaL_getmetatable(L, "sm.array");
	lua_setmetatable(L, -2);
	return 1;
}

// stk: array
static int collect_array(lua_State *L) {
	SMArray *a = check_sm_array(L,1);
	sm_array_free(a->native);
	return 0;
}

// stk: array
static int array_size(lua_State *L) {	
	SMArray *a = check_sm_array(L,1);
	lua_pushinteger(L, (lua_Integer)a->native->stack_size);
	return 1;
}

// stk: array, key
static int array_find_state(lua_State *L) {	
	SMArray *a = check_sm_array(L, 1);
	const char *key = luaL_checkstring(L, 2);
	size_t keylen = strlen(key);
	sm_state *s = sm_array_find_state(a->native, (void *const)key, keylen);
	if(push_state_handler(L, s) != EXIT_SUCCESS) {
		lua_pushnil(L);
		return 1;
	}
	SMState *lua_s = check_sm_state(L, -1);
	lua_s->standalone = false;
	return 1;
}

// stk: array, key
static int array_get_state(lua_State *L) {	
	SMArray *a = check_sm_array(L, 1);
	const char *key = luaL_checkstring(L, 2);
	size_t keylen = strlen(key);
	sm_state *s = sm_array_find_state(a->native, key, keylen);
	if(push_state_handler(L, s) != EXIT_SUCCESS) {
		lua_pushnil(L);
		return 1;
	}
	SMState *lua_s = check_sm_state(L, -1);
	lua_s->standalone = false;
	return 1;
}

// stk: array, state
static int array_release_state(lua_State *L) {	
	SMArray *a = check_sm_array(L, 1);
	SMState *s = check_sm_state(L, 2);
	sm_array_release_state(a->native, s->native);
	return 0;
}

// stk: array
static int array_tostring(lua_State *L) {
	SMArray *a = check_sm_array(L, 1);
	lua_pushfstring(L, "sm_array @ %p, size = %I, synchronized = %s\n", 
					a, (lua_Integer)a->native->stack_size, 
					a->native->synchronized ? "true" : "false");
	return 1;
}

static const struct luaL_Reg smarray_m [] = {
	{"find", array_find_state},
	{"get", array_get_state},
	{"release", array_release_state},
	{"__len", array_size},
	{"__tostring", array_tostring},
	{"__gc", collect_array},
	{NULL, NULL}
};

/**********************
 ******* SM.LIB *******
 **********************/

static const struct luaL_Reg smlib_f [] = {
	{"new_event", new_event},
	{"new_queue", new_queue},
	{"new_pqueue", new_pqueue},
	{"new_queue2", new_queue2},
	{"loadlib", load_applib},
	{"lookup", lookup_app}, 
	{"new_apptab", new_apptab},
	{"new_fsm", new_fsm},
	{"new_fsmtab", new_fsmtab},
	{"new_state", new_state},
	{"new_array", new_array},
	{NULL, NULL}
};

int luaopen_sm (lua_State *L) {
	luaL_newmetatable(L, "sm.event");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, smevent_m, 0);
	lua_newtable(L);
	lua_setfield(L, -2, "inventory");
	
	luaL_newmetatable(L, "sm.queue");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, smqueue_m, 0);
	
	luaL_newmetatable(L, "sm.pqueue");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, smpqueue_m, 0);

	luaL_newmetatable(L, "sm.queue2");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, smqueue2_m, 0);
	
	luaL_newmetatable(L, "sm.app");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, smapp_m, 0);
	lua_newtable(L);
	lua_setfield(L, -2, "inventory");
	
	luaL_newmetatable(L, "sm.apptable");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, smapptable_m, 0);
	
	luaL_newmetatable(L, "sm.fsm");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, smfsm_m, 0);
	lua_newtable(L);
	lua_setfield(L, -2, "inventory");
	
	luaL_newmetatable(L, "sm.fsmtable");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, smfsmtable_m, 0);

	luaL_newmetatable(L, "sm.state");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, smstate_m, 0);
	lua_newtable(L);
	lua_setfield(L, -2, "inventory");
	
	luaL_newmetatable(L, "sm.array");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, smarray_m, 0);
	
	luaL_newlib(L, smlib_f);
	return 1;
};
