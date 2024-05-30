/* SM.EXEC 
   Lua wrapper
   anton.bondarenko@gmail.com */


#include "lprefix.h"

#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "../src/sm_sys.h"
#include "../src/sm_event.h"
#include "../src/sm_queue.h"
#include "../src/sm_queue2.h"
#include "../src/sm_app.h"
#include "../src/sm_fsm.h"
#include "../src/sm_state.h"
#include "../src/sm_array.h"

/************************
 ******* SM.EVENT *******
 ************************/

typedef struct SMEvent {
	sm_event *native;
	size_t linked;
} SMEvent;

#define check_sm_event(L) (SMEvent *)luaL_checkudata(L, 1, "sm.event")
/*
static int create_event(lua_State *L) {
	size_t id = (size_t)luaL_checkinteger(L, 1);
	size_t plsize = (size_t)lua_tointeger(L, 2);
	SMEvent *lua_e = (SMEvent *)lua_touserdata(L, 3);
	sm_event *ne = sm_event_create(plsize + sizeof(SMEvent *));
	if(ne == NULL)
		return luaL_error(L, "@malloc(%I)", (lua_Integer)plsize);
	ne->id = id;
	memset(ne->data, '\0', plsize);	
	*((SMEvent **)(ne->data + plsize)) = lua_e;
	lua_e->native = ne;
	lua_e->linked = 0;
	return 0;
}
*/

static int push_event_handler(sm_event *e) {
	if(e == NULL)
		return EXIT_FAILURE;
	if(this(e) != NULL) { 		// handler was already created
		lua_pushlightuserdata(L, e1->native->next);		// lud
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
														// ... inventory lud
	lua_gettable(L, -2);								// ... inventory ud	
	return 1;	
}

static int new_event(lua_State *L) {
	size_t id = (size_t)luaL_checkinteger(L, 1);
	luaL_argcheck(L, id >= 0, 1, "wrong event id");
	size_t plsize = (size_t)luaL_checkinteger(L, 2);
	luaL_argcheck(L, plsize >= 0, 1, "wrong event payload size");
	sm_event *ne = sm_event_create(plsize + sizeof(SMEvent *));
	if(ne == NULL)
		return luaL_error(L, "@malloc(%I)", (lua_Integer)plsize);
	ne->id = id;
	memset(ne->data, '\0', plsize);	
	this(ne) = NULL;
	if(push_event_handler(ne) == EXIT_FAILURE) {
		lua_pushnil(L);
		return 1;
	}
	ne->linked == 0;
}
														//
	
	luaL_getmetatable(L, "sm.event");								// mt
	lua_getfield(L, -1, "inventory");								// mt inventory
	SMEvent *e = (SMEvent *)lua_newuserdata(L, sizeof(SMEvent));	// mt inventory ud	
	lua_pushlightuserdata(L, ne);									// mt inventory ud lud
	lua_pushvalue(L, -2);											// mt inventory ud lud ud
	lua_settable(L, -4);											// mt inventory ud
	lua_rotate(L, -3, -1);											// inventory ud mt		   
	lua_setmetatable(L, -2);										// inventory ud 
	e->native = ne;
	*((SMEvent **)(ne->data + plsize)) = e;
	
	e->linked = 0;
	return 1;
/*	
	lua_pushcfunction(L, create_event);
	lua_pushinteger(L, id);
	lua_pushinteger(L, plsize);
	lua_pushlightuserdata(L, e);
	if(lua_pcall(L, 3, 0, 0) == LUA_OK)
		return 1;
	else {
		lua_error(L);
		lua_pushnil(L);
		return 1;
	}
*/
}

static int set_event(lua_State *L) {	
	SMEvent *e = check_sm_event(L);
	const char *s = luaL_checkstring(L, 2);
	strncpy((char *)e->native->data, s, e->native->data_size - sizeof(SMEvent *) - 1);
	return 0;
}

static int set_event_id(lua_State *L) {	
	SMEvent *e = check_sm_event(L);
	e->native->id = (size_t)luaL_checkinteger(L, 2);
	return 0;
}
	
static int get_event(lua_State *L) {	
	SMEvent *e = check_sm_event(L);
	lua_pushinteger(L, e->native->id);
	lua_pushlstring(L, (char *)e->native->data, e->native->data_size - sizeof(SMEvent *));
	return 1;
}

static int get_event_id(lua_State *L) {	
	SMEvent *e = check_sm_event(L);
	lua_pushinteger(L, e->native->id);
	return 1;
}
	
static int event_size(lua_State *L) {	
	SMEvent *e = check_sm_event(L);
	lua_pushinteger(L, (lua_Integer)e->native->data_size - sizeof(SMEvent *));
	return 1;
}	

// Tailed wrapper address for next from wrapper for this
#define next(E1) (*((SMEvent **)(E1->native->next->data \
                               + E1->native->next->data_size \
							   - sizeof(SMEvent *))))

// Tailed wrapper address for this from native for this
#define this(E1) (*((SMEvent **)(E1->data + E1->data_size - sizeof(SMEvent *))))

#define sm_abs(I) I < 0 ? (-I) : (I)

static int link_event(lua_State *L) {	
	SMEvent *e1 = check_sm_event(L);
	SMEvent *e2 = (SMEvent *)luaL_checkudata(L, 2, "sm.event");
	e1->native->next = e2->native;
	next(e1)->linked++;
	lua_pop(L, 1);
	return 1;
}

static int unlink_event(lua_State *L) {	
	SMEvent *e1 = check_sm_event(L);
	if(e1->native->next == NULL)
		return 0;
	next(e1)->linked--;
	e1->native->next = NULL;
	lua_pop(L, 1);
	return 0;
}

static int event_tostring(lua_State *L) {	
	SMEvent *e = check_sm_event(L);
	lua_pushfstring(L, "sm_event id %I @ %p : %p -> %p, linked = %I", e->native->id, e,  
					e->native, (e->native == NULL) ? NULL : e->native->next, e->linked);
	return 1;
}



static int next_event(lua_State *L) {
	SMEvent *e = check_sm_event(L);
	if(e->native == NULL || e->native->next == NULL) {
		lua_pushnil(L);
		return 1;
	}
	if(push_event_handler(e->native->next) == EXIT_FAILURE) {
		lua_pushnil(L);
		return 1;
	}
	if(next(e)->linked == -1)
		next(e)->linked == 1;
}

static int collect_event(lua_State *L) {
	SMEvent *e = check_sm_event(L);
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
	return 0;
}
	
static const struct luaL_Reg smevent_m [] = {
	{"set", set_event},
	{"get", get_event},
	{"setid", set_event_id},
	{"getid", get_event_id},
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

static int newqueue(lua_State *L) {
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
	if(q->native == NULL)
		return luaL_error(L, "@malloc(%I)", (lua_Integer)plsize);
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

static int queue_size(lua_State *L) {	
	SMQueue *q = check_sm_queue(L);
	lua_pushinteger(L, (lua_Integer)q->native->size);
	return 1;
}	

static int printqueue(lua_State *L) {
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
		lua_pushfstring(L, "sm_event @ %p : %p -> %p, linked = %I\n", 
						lua_e, e, e->next, 
						(lua_e == NULL) ? -1 : lua_e->linked);
		luaL_addvalue(&b);
		e = e->next;
	}
	luaL_pushresult(&b);
	return 1;
}

static int collectqueue(lua_State *L) {
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

// implementable in Lua
/*
static int toarray(lua_State *L) {
	SMQueue *q = check_sm_queue(L);
	// create array a
	sm_event *e = sm_queue_top(q->native);
	i = 1;
	SMEvent *lua_e
	while(e != NULL) {
		if(this(e) == NULL) {
			lua_e = (SMEvent *)lua_newuserdata(L, sizeof(SMEvent));
			luaL_getmetatable(L, "sm.event");
			lua_setmetatable(L, -2);
			lua_e->native = e;
			lua_e->linked = 1;
		}
		//a[i] = this(e);
	}
	return 1;
}
*/

static int queuetop(lua_State *L) {
	SMQueue *q = check_sm_queue(L);
	sm_event *top_e = sm_queue_top(q->native);	
	if(top_e == NULL) {
		lua_pushnil(L);
		return 1;
	}	
	if(this(top_e) != NULL)
		lua_pushlightuserdata(L, this(top_e));
	else {
		SMEvent *lua_e = (SMEvent *)lua_newuserdata(L, sizeof(SMEvent));
		luaL_getmetatable(L, "sm.event");
		lua_setmetatable(L, -2);
		lua_e->native = top_e;
		lua_e->linked = 1;
		this(top_e) = lua_e;
	}
	return 1;
}

static int dequeue(lua_State *L) {
	SMQueue *q = check_sm_queue(L);
	sm_event *top_e = sm_queue_dequeue(q->native);	
	if(top_e == NULL) {
		lua_pushnil(L);
		return 1;
	}
	if(this(top_e) != NULL) {
		this(top_e)->linked--;
		lua_pushlightuserdata(L, this(top_e));
	}
	else {
		SMEvent *lua_e = (SMEvent *)lua_newuserdata(L, sizeof(SMEvent));
		luaL_getmetatable(L, "sm.event");
		lua_setmetatable(L, -2);
		lua_e->native = top_e;
		lua_e->linked = 0;
		if(top_e->next != NULL && next(lua_e) != NULL)
			next(lua_e)->linked--;  
		this(top_e) = lua_e;
	}
	return 1;
}

#define check_sm_event2(L) (SMEvent *)luaL_checkudata(L, 2, "sm.event")

static int enqueue(lua_State *L) {
	SMQueue *q = check_sm_queue(L);
	if(q == NULL) {
		return 0;
	}
	SMEvent *e = check_sm_event2(L);	
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
	{"__tostring", printqueue},
	{"__gc", collectqueue},
	{NULL, NULL}
};


/************************
 ******* SM.QUEUE2 *******
 ************************/


typedef struct SMQueue2 {
	sm_queue2 *native;
} SMQueue2;

#define check_sm_queue2(L) (SMQueue2 *)luaL_checkudata(L, 1, "sm.queue2")

static int newqueue2(lua_State *L) {
	size_t qsize = (size_t)luaL_checkinteger(L, 1);
	luaL_argcheck(L, qsize >= 0, 1, "wrong queue2 size");
	size_t plsize = (size_t)luaL_checkinteger(L, 2);
	luaL_argcheck(L, plsize >= 0, 1, "wrong event payload size");
	SMQueue2 *q = (SMQueue2 *)lua_newuserdata(L, sizeof(SMQueue2));
	q->native = sm_queue2_create(plsize + sizeof(SMEvent *), qsize);
	if(q->native == NULL)
		return luaL_error(L, "@malloc(%I)", (lua_Integer)plsize);
	luaL_getmetatable(L, "sm.queue2");
	lua_setmetatable(L, -2);
	sm_event *e = sm_queue2_get(q->native);
	while(e->next != NULL) {
		memset(e->data, '\0', plsize);	
		*((SMEvent **)(e->data + plsize)) = NULL;
		e = e->next;
	}
	return 1;
}

static int queue2_size(lua_State *L) {	
	SMQueue2 *q = check_sm_queue2(L); (void) q;
	lua_pushnil(L);
	return 1;
}	

static int printqueue2(lua_State *L) {
	SMQueue2 *q = check_sm_queue2(L);
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
		lua_pushfstring(L, "sm_event @ %p : %p -> %p, linked = %I\n", 
						lua_e, e, e->next, 
						(lua_e == NULL) ? -1 : lua_e->linked);
		luaL_addvalue(&b);
		e = e->next;
	}
	lua_pushfstring(L, "Higher priority events\n");
	luaL_addvalue(&b);
	e = sm_queue2_get_high(q->native);
	while(e != NULL) {
		lua_e = this(e);
		lua_pushfstring(L, "sm_event @ %p : %p -> %p, linked = %I\n", 
						lua_e, e, e->next, 
						(lua_e == NULL) ? -1 : lua_e->linked);
		luaL_addvalue(&b);
		e = e->next;
	}
	luaL_pushresult(&b);
	return 1;
}

static int collectqueue2(lua_State *L) {
	SMQueue2 *q = check_sm_queue2(L);
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

static int queue2get(lua_State *L) {
	SMQueue2 *q = check_sm_queue2(L);
	sm_event *top_e = sm_queue2_get(q->native);	
	if(top_e == NULL) {
		lua_pushnil(L);
		return 1;
	}	
	if(this(top_e) != NULL)
		lua_pushlightuserdata(L, this(top_e));
	else {
		SMEvent *lua_e = (SMEvent *)lua_newuserdata(L, sizeof(SMEvent));
		luaL_getmetatable(L, "sm.event");
		lua_setmetatable(L, -2);
		lua_e->native = top_e;
		lua_e->linked = 1;
		this(top_e) = lua_e;
	}
	return 1;
}

static int queue2gethigh(lua_State *L) {
	SMQueue2 *q = check_sm_queue2(L);
	sm_event *top_e = sm_queue2_get_high(q->native);	
	if(top_e == NULL) {
		lua_pushnil(L);
		return 1;
	}	
	if(this(top_e) != NULL)
		lua_pushlightuserdata(L, this(top_e));
	else {
		SMEvent *lua_e = (SMEvent *)lua_newuserdata(L, sizeof(SMEvent));
		luaL_getmetatable(L, "sm.event");
		lua_setmetatable(L, -2);
		lua_e->native = top_e;
		lua_e->linked = 1;
		this(top_e) = lua_e;
	}
	return 1;
}

static int dequeue2(lua_State *L) {
	SMQueue2 *q = check_sm_queue2(L);
	sm_event *top_e = sm_dequeue2(q->native);	
	if(top_e == NULL) {
		lua_pushnil(L);
		return 1;
	}
	if(this(top_e) != NULL) {
		this(top_e)->linked--;
		lua_pushlightuserdata(L, this(top_e));
	}
	else {
		SMEvent *lua_e = (SMEvent *)lua_newuserdata(L, sizeof(SMEvent));
		luaL_getmetatable(L, "sm.event");
		lua_setmetatable(L, -2);
		lua_e->native = top_e;
		lua_e->linked = 0;
		if(top_e->next != NULL && next(lua_e) != NULL)
			next(lua_e)->linked--; 
		this(top_e) = lua_e;
	}
	return 1;
}

static int lockdequeue2(lua_State *L) {
	SMQueue2 *q = check_sm_queue2(L);
	sm_event *top_e = sm_lock_dequeue2(q->native);	
	if(top_e == NULL) {
		lua_pushnil(L);
		return 1;
	}
	if(this(top_e) != NULL) {
		this(top_e)->linked--;
		lua_pushlightuserdata(L, this(top_e));
	}
	else {
		SMEvent *lua_e = (SMEvent *)lua_newuserdata(L, sizeof(SMEvent));
		luaL_getmetatable(L, "sm.event");
		lua_setmetatable(L, -2);
		lua_e->native = top_e;
		lua_e->linked = 0;
		if(top_e->next != NULL && next(lua_e) != NULL)
			next(lua_e)->linked--; 
		this(top_e) = lua_e;
	}
	return 1;
}

#define check_sm_event2(L) (SMEvent *)luaL_checkudata(L, 2, "sm.event")

static int enqueue2(lua_State *L) {
	SMQueue2 *q = check_sm_queue2(L);
	if(q == NULL) {
		return 0;
	}
	SMEvent *e = check_sm_event2(L);	
	if(e == NULL) {
		return 0;
	}
	sm_enqueue2(e->native, q->native);
	e->linked++;
	return 0;
}

static int enqueue2high(lua_State *L) {
	SMQueue2 *q = check_sm_queue2(L);
	if(q == NULL) {
		return 0;
	}
	SMEvent *e = check_sm_event2(L);	
	if(e == NULL) {
		return 0;
	}
	sm_enqueue2_high(e->native, q->native);
	e->linked++;
	return 0;
}

static int lockenqueue2(lua_State *L) {
	SMQueue2 *q = check_sm_queue2(L);
	if(q == NULL) {
		return 0;
	}
	SMEvent *e = check_sm_event2(L);	
	if(e == NULL) {
		return 0;
	}
	sm_lock_enqueue2(e->native, q->native);
	e->linked++;
	return 0;
}

static int lockenqueue2high(lua_State *L) {
	SMQueue2 *q = check_sm_queue2(L);
	if(q == NULL) {
		return 0;
	}
	SMEvent *e = check_sm_event2(L);	
	if(e == NULL) {
		return 0;
	}
	sm_lock_enqueue2_high(e->native, q->native);
	e->linked++;
	return 0;
}

static const struct luaL_Reg smqueue2_m [] = {
	{"get", queue2get},
	{"gethigh", queue2gethigh},
	{"enqueue", enqueue2},
	{"lockenqueue", lockenqueue2},
	{"enqueuehigh", enqueue2high},
	{"lockenqueuehigh", lockenqueue2high},
	{"dequeue", dequeue2},
	{"lockdequeue", lockdequeue2},
	{"__len", queue2_size},
	{"__tostring", printqueue2},
	{"__gc", collectqueue2},
	{NULL, NULL}
};


/************************
 ******* SM.APP *********
 ************************/

typedef struct SMApp {
	sm_app native;
} SMApp;

#define check_sm_app(L) (SMApp *)luaL_checkudata(L, 1, "sm.app")
#define check_sm_app2(L) (SMApp *)luaL_checkudata(L, 2, "sm.app")

static int lookup_app(lua_State *L) {
	void *handle;
	if((handle = lua_touserdata(L, 1)) == NULL)
		return luaL_error(L, "@lookup_app: lua_touserdata() returns NULL");
	const char *name = luaL_checkstring(L, 2);
	SMApp *a = (SMApp *)lua_newuserdata(L, sizeof(SMApp));
	luaL_getmetatable(L, "sm.app");
	lua_setmetatable(L, -2);
	a->native = dlsym(handle, name);
	return 1;
}

static int print_app(lua_State *L) {	
	SMApp *a = check_sm_app(L);
	lua_pushfstring(L, "sm_app @ %p", a->native);
	return 1;
}

static int call_app(lua_State *L) {	
	SMApp *a = check_sm_app(L);
	SMEvent *e = check_sm_event2(L);
	(*(a->native))(e->native);
	return 0;
}

static const struct luaL_Reg smapp_m [] = {
	{"__tostring", print_app},
	{"__call", call_app},
	{NULL, NULL}
};

/************************
 **** SM.APP_TABLE ******
 ************************/

static int load_applib(lua_State *L) {
	const char *fn = luaL_checkstring(L, 1);
	lua_pushlightuserdata(L, dlopen(fn, SM_RTLD_FLAG));
	return 1;
}

typedef struct SMAppTable {
	sm_app_table *native;
} SMAppTable;

#define check_sm_apptab(L) (SMAppTable *)luaL_checkudata(L, 1, "sm.apptable")

static int new_apptab(lua_State *L) {
	SMAppTable *at = (SMAppTable *)lua_newuserdata(L, sizeof(SMAppTable));
	at->native = sm_app_table_create();
//	if(AT->native == NULL)
//		return luaL_error(L, "@malloc()");
	luaL_getmetatable(L, "sm.apptable");
	lua_setmetatable(L, -2);
	return 1;
}	

static int apptab_set(lua_State *L) {
	SMAppTable *at = check_sm_apptab(L);
	SMApp *a = check_sm_app2(L);
	const char *name = luaL_checkstring(L, 3);
	at->native = sm_app_table_set(at->native, name, a->native);
	return 0;
}	

static int apptab_get(lua_State *L) {
	SMAppTable *at = check_sm_apptab(L);
	const char *name = luaL_checkstring(L, 2);
	SMApp *a = (SMApp *)lua_newuserdata(L, sizeof(SMApp));
	a->native = *(sm_app_table_get_ref(at->native, name));
	return 1;
}

static int apptab_remove(lua_State *L) {
	SMAppTable *at = check_sm_apptab(L);
	const char *name = luaL_checkstring(L, 2);
	sm_app_table_remove(at->native, (char *)name);
	return 0;
}	

static int apptab_size(lua_State *L) {	
	SMAppTable *lat = check_sm_apptab(L);
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

static int print_apptab(lua_State *L) {	
	SMAppTable *ac = check_sm_apptab(L);
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

static int collect_apptab(lua_State *L) {	
	SMAppTable *at = check_sm_apptab(L);
	if(at == NULL)
		return 0;
	sm_app_table *ac = at->native;
	sm_app_table *acn;
	while(ac != NULL) {
		acn = ac;
		ac = ac->next;
		free(ac);
		ac = acn;
	}
	return 0;
}	

static const struct luaL_Reg smapptable_m [] = {
	{"set", apptab_set},
	{"get", apptab_get},
	{"remove", apptab_remove},
	{"__len", apptab_size},
	{"__tostring", print_apptab},
	{"__gc", collect_apptab},
	{NULL, NULL}
};
	

/**********************
 ******* SM.FSM *******
 **********************/

typedef struct SMFSM {
	sm_fsm *native;
} SMFSM;


#define check_sm_fsm(L, POS) (SMFSM *)luaL_checkudata(L, POS, "sm.fsm")

static int new_fsm(lua_State *L) {
	SMAppTable *at = check_sm_apptab(L);
	const char *s = luaL_checkstring(L, 2);
	const char *mm = luaL_checkstring(L, 3);
	SMFSM *f = (SMFSM *)lua_newuserdata(L, sizeof(SMFSM));
	sm_fsm_type t;
	if(!strcmp(mm, "mealy")) 
		t = SM_MEALY; 
	else { 
		if (!strcmp(mm, "moore")) 
			t = SM_MOORE;
		else 
			return luaL_error(L, "wrong automata type: %s", mm);
	}
	f->native = sm_fsm_create(s, at->native, t);
	if(f->native == NULL)
		return luaL_error(L, "@malloc()");
	luaL_getmetatable(L, "sm.fsm");
	lua_setmetatable(L, -2);
	return 1;
}	

static int collect_fsm(lua_State *L) {	
	SMFSM *f = check_sm_fsm(L, 1);
	if(f == NULL)
		return 0;
	sm_fsm_free(f->native);
	return 0;
}	

static int print_fsm(lua_State *L) {	
	SMFSM *f = check_sm_fsm(L, 1);
	lua_pushfstring(L, "sm_fsm %p -> %p:\n%s\n", f, f->native, sm_fsm_to_string(f->native));
	return 1;
}	

static const struct luaL_Reg smfsm_m [] = {
	{"__tostring", print_fsm},
	{"__gc", collect_fsm},
	{NULL, NULL}
};

/**********************
 ****** SM.STATE ******
 **********************/

typedef struct SMState {
	sm_state *native;
	bool standalone;
} SMState;

#define check_sm_state(L, POS) (SMState *)luaL_checkudata(L, POS, "sm.state")

static int new_state(lua_State *L) {
	SMFSM *fsm = check_sm_fsm(L, 1);
	size_t plsize = (size_t)lua_tointeger(L, 2);
	SMState *s = (SMState *)lua_newuserdata(L, sizeof(SMState));
	s->native = sm_state_create(fsm->native, plsize);
	if(s->native == NULL)
		return luaL_error(L, "@malloc()");
	s->standalone = true;
	luaL_getmetatable(L, "sm.state");
	lua_setmetatable(L, -2);
	return 1;
}	

// stk: state event
static int state_trace_add(lua_State *L) {	
	SMEvent *e = check_sm_event2(L);
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
	if(this(top_e) != NULL) {
		lua_pushlightuserdata(L, this(top_e));
		//luaL_getmetatable(L, "sm.event");
		//lua_setmetatable(L, -2);
	}
	else {
		SMEvent *lua_e = (SMEvent *)lua_newuserdata(L, sizeof(SMEvent));
		luaL_getmetatable(L, "sm.event");
		lua_setmetatable(L, -2);
		lua_e->native = top_e;
		lua_e->linked = 1;
		this(top_e) = lua_e;
	}
	return 1;
}

// stk: state keystring keylen
static int state_setkey(lua_State *L) {	
	SMState *s = check_sm_state(L, 1);
	if(s == NULL || s->native == NULL) // empty or childless
		return luaL_error(L, "empty state handler");
	const char *key = luaL_checkstring(L, 2);
	//size_t keylen = luaL_checkinteger(L, 3);
	sm_state_set_key(s->native, key/*, keylen*/);
	return 0;
}

// stk: state
static int state_getkey(lua_State *L) {	
	SMState *s = check_sm_state(L, 1);
	if(s == NULL || s->native == NULL) // empty or childless
		return luaL_error(L, "empty state handler");
	//lua_pushlstring(L, (char *)s->native->key, s->native->key_length);
	//lua_pushinteger(L, s->native->key_length);
	//return 2;
	lua_pushfstring(L, (char *)s->native->key);
	return 1;
}

// stk: state data
static int state_setstate(lua_State *L) {	
	SMState *s = check_sm_state(L, 1);
	if(s == NULL || s->native == NULL) // empty or childless
		return luaL_error(L, "empty state handler");
	const char *data = luaL_checkstring(L, 2);
	char *d = (char *)s->native->data;
	memcpy(d, data, MIN(strlen(data), s->native->data_size));
	d[strlen(data)] = '\0';
	return 0;
}

// stk: state SM_STATE_ID
static int state_setstateid(lua_State *L) {	
	SMState *s = check_sm_state(L, 1);
	if(s == NULL || s->native == NULL) // empty or childless
		return luaL_error(L, "empty state handler");
	s->native->id = luaL_checkinteger(L, 2);
	return 0;
}

// stk: state
static int state_getstate(lua_State *L) {	
	SMState *s = check_sm_state(L, 1);
	if(s == NULL || s->native == NULL) // empty or childless
		return luaL_error(L, "empty state handler");
	lua_pushfstring(L, (char *)s->native->data);
	return 1;
}

// stk: state
static int state_getstateid(lua_State *L) {	
	SMState *s = check_sm_state(L, 1);
	if(s == NULL || s->native == NULL) // empty or childless
		return luaL_error(L, "empty state handler");
	lua_pushinteger(L, s->native->id);
	return 1;
}

static int collect_state(lua_State *L) {	
	SMState *s = check_sm_state(L, 1);
	if(s == NULL || s->native == NULL) // empty or childless
		return 0;
	if(s->standalone)
		sm_state_free(s->native);
	s->native = NULL;
	return 0;
}

static int purge_state(lua_State *L) {	
	SMState *f = check_sm_state(L, 1);
	if(f == NULL)
		return 0;
	sm_state_purge(f->native);
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

static int print_state(lua_State *L) {	
	SMState *s = check_sm_state(L, 1);
	luaL_Buffer b;
	luaL_buffinit(L, &b);
	lua_pushfstring(L, "sm_state id %I @ %p -> %p:\n", s->native->id, s, s->native);
	luaL_addvalue(&b);
	lua_pushfstring(L, "fsm %I @ %p, key = ");
	luaL_addvalue(&b);
	if(s->native->key_length != 0)
		lua_pushfstring(L, "%s", (char *)s->native->key);
	else
		lua_pushfstring(L, "");
	luaL_addvalue(&b);
	lua_pushfstring(L, ", data = ");
	luaL_addvalue(&b);
	if(s->native->data_size != 0)
		lua_pushfstring(L, (const char *)s->native->data);
	else
		lua_pushfstring(L, "");
	luaL_addvalue(&b);
	lua_pushfstring(L, "\n");
	luaL_addvalue(&b);
	luaL_pushresult(&b);
	return 1;
}

static const struct luaL_Reg smstate_m [] = {
	{"apply", apply_event},
	{"purge", purge_state},
	{"trace_add", state_trace_add},
	{"trace_get", state_trace_get},
	{"get_key", state_getkey},
	{"set_key", state_setkey},
	{"get", state_getstate},
	{"set", state_setstate},
	{"getid", state_getstateid},
	{"setid", state_setstateid},
	{"purge", purge_state},
	{"__tostring", print_state},
	{"__gc", collect_state},
	{NULL, NULL}
};

/**********************
 ****** SM.ARRAY ******
 **********************/

typedef struct SMArray {
	sm_array *native;
} SMArray;

#define check_sm_array(L, POS) (SMArray *)luaL_checkudata(L, POS, "sm.array")

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
	a->native = sm_array_create(stsize, plsize, fsm->native, sync);
	if(a->native == NULL)
		return luaL_error(L, "@malloc(%I)", (lua_Integer)plsize);
	luaL_getmetatable(L, "sm.array");
	lua_setmetatable(L, -2);
	return 1;
}

static int collect_array(lua_State *L) {
	SMArray *a = check_sm_array(L,1);
	sm_array_free(a->native);
	return 0;
}

static int array_size(lua_State *L) {	
	SMArray *a = check_sm_array(L,1);
	lua_pushinteger(L, (lua_Integer)a->native->stack_size);
	return 1;
}

static int array_find_state(lua_State *L) {	
	SMArray *a = check_sm_array(L, 1);
	void * key;
	if(lua_islightuserdata(L, 2))
		key = lua_touserdata(L, 2);
	else
		return luaL_error(L, "ligh userdata expected");
	size_t keylen = (size_t)luaL_checkinteger(L, 3);	
	SMState *s = (SMState *)lua_newuserdata(L, sizeof(SMState));
	s->native = sm_array_find_state(a->native, key, keylen);
	s->standalone = false;
	return 1;
}

static int array_get_state(lua_State *L) {	
	SMArray *a = check_sm_array(L, 1);
	void * key;
	if(lua_islightuserdata(L, 2))
		key = lua_touserdata(L, 2);
	else
		return luaL_error(L, "ligh userdata expected");
	size_t keylen = (size_t)luaL_checkinteger(L, 3);	
	SMState *s = (SMState *)lua_newuserdata(L, sizeof(SMState));
	s->native = sm_array_get_state(a->native, key, keylen);
	s->standalone = false;
	return 1;
}

static int array_release_state(lua_State *L) {	
	SMArray *a = check_sm_array(L, 1);
	SMState *s = check_sm_state(L, 2);
	sm_array_release_state(a->native, s->native);
	return 0;
}

static int print_array(lua_State *L) {
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
	{"__tostring", print_array},
	{"__gc", collect_array},
	{NULL, NULL}
};

/**********************
 ******* SM.LIB *******
 **********************/

static const struct luaL_Reg smlib_f [] = {
	{"new_event", newevent},
	{"new_queue", newqueue},
	{"new_queue2", newqueue2},
	{"loadlib", load_applib},
	{"lookup", lookup_app}, 
	{"new_apptab", new_apptab},
	{"new_fsm", new_fsm},
	{"new_state", new_state},
	{"new_array", new_array},
	{NULL, NULL}
};

int luaopen_sm (lua_State *L) {
	luaL_newmetatable(L, "sm.event");
	//mt
	lua_pushvalue(L, -1);
	//mt mt
	lua_setfield(L, -2, "__index");
	//mt
	luaL_setfuncs(L, smevent_m, 0);
	//mt
	lua_newtable(L);
	//mt new
	lua_setfield(L, -2, "inventory");
	//mt
	
	luaL_newmetatable(L, "sm.queue");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, smqueue_m, 0);

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
