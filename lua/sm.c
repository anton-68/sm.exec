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

/************************
 ******* SM.EVENT *******
 ************************/

typedef struct SMEvent {
	sm_event *native;
	size_t linked;
} SMEvent;

#define check_sm_event(L) (SMEvent *)luaL_checkudata(L, 1, "sm.event")

static int create_event(lua_State *L) {
	size_t plsize = (size_t)lua_tointeger(L, 1);
	SMEvent *lua_e = (SMEvent *)lua_touserdata(L, 2);
	sm_event *ne = sm_event_create(plsize + sizeof(SMEvent *));
	if(ne == NULL)
		return luaL_error(L, "@malloc(%I)", (lua_Integer)plsize);
	memset(ne->data, '\0', plsize);	
	*((SMEvent **)(ne->data + plsize)) = lua_e;
	lua_e->native = ne;
	lua_e->linked = 0;
	return 0;
}

static int newevent(lua_State *L) {
	size_t plsize = (size_t)luaL_checkinteger(L, 1); 
	luaL_argcheck(L, plsize >= 0, 1, "wrong event payload size");
	SMEvent *e = (SMEvent *)lua_newuserdata(L, sizeof(SMEvent)); (void)e;
	luaL_getmetatable(L, "sm.event");
	lua_setmetatable(L, -2);
	lua_pushcfunction(L, create_event);
	lua_pushinteger(L, plsize);
	lua_pushlightuserdata(L, e);
	if(lua_pcall(L, 2, 0, 0) == LUA_OK)
		return 1;
	else {
		lua_error(L);
		lua_pushnil(L);
		return 1;
	}
}

static int setevent(lua_State *L) {	
	SMEvent *e = check_sm_event(L);
	const char *s = luaL_checkstring(L, 2);
	strncpy((char *)e->native->data, s, e->native->data_size - sizeof(SMEvent *) - 1);
	return 0;
}
	
static int getevent(lua_State *L) {	
	SMEvent *e = check_sm_event(L);
	lua_pushlstring(L, (char *)e->native->data, e->native->data_size - sizeof(SMEvent *));
	return 1;
}
	
static int event_plsize(lua_State *L) {	
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

static int linkevent(lua_State *L) {	
	SMEvent *e1 = check_sm_event(L);
	SMEvent *e2 = (SMEvent *)luaL_checkudata(L, 2, "sm.event");
	e1->native->next = e2->native;
	next(e1)->linked++;
	lua_pop(L, 1);
	return 1;
}

static int unlinkevent(lua_State *L) {	
	SMEvent *e1 = check_sm_event(L);
	if(e1->native->next == NULL)
		return 0;
	next(e1)->linked--;
	e1->native->next = NULL;
	lua_pop(L, 1);
	return 0;
}

static int printevent(lua_State *L) {	
	SMEvent *e = check_sm_event(L);
	lua_pushfstring(L, "sm_event @ %p : %p -> %p, linked = %I", e,  e->native,
				   (e->native == NULL) ? NULL : e->native->next, e->linked);
	return 1;
}

static int nextevent(lua_State *L) {
	SMEvent *e1 = check_sm_event(L);
	if(e1->native->next == NULL) {
		lua_pushnil(L);
		return 1;
	}
	if(next(e1) != NULL)
		lua_pushlightuserdata(L, next(e1));
	else {
		SMEvent *lua_e = (SMEvent *)lua_newuserdata(L, sizeof(SMEvent));
		luaL_getmetatable(L, "sm.event");
		lua_setmetatable(L, -2);
		lua_e->native = e1->native->next;
		lua_e->linked = 1;
		next(e1) = lua_e;
	}
	return 1;
}

static int collectevent(lua_State *L) {
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
	{"set", setevent},
	{"get", getevent},
	{"__concat", linkevent},
	{"__unm", unlinkevent},
	{"next", nextevent},
	{"__len", event_plsize},
	{"__tostring", printevent},
	{"__gc", collectevent},
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

static int load_applib(lua_State *L) {
	const char *fn = luaL_checkstring(L, 1);
	lua_pushlightuserdata(L, dlopen(fn, SM_RTLD_FLAG));
	return 1;
}

static int lookup_app(lua_State *L) {
	void *handle;
	if((handle = lua_touserdata(L, 1)) == NULL)
		return luaL_error(L, "@lookup_app: lua_touserdata() returns NULL");
	const char *name = luaL_checkstring(L, 2);
	lua_pushlightuserdata(L, dlsym(handle, name));
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
	void *app;
	if((app = lua_touserdata(L, 2)) == NULL)
		return luaL_error(L, "@apptab_set: lua_touserdata() returns NULL");
	const char *name = luaL_checkstring(L, 3);
	at->native = sm_app_table_set(at->native, name, (sm_app)app);
	return 0;
}	

static int apptab_get(lua_State *L) {
	SMAppTable *at = check_sm_apptab(L);
	const char *name = luaL_checkstring(L, 2);
	lua_pushlightuserdata(L, (void *)(sm_app_table_get_ref(at->native, (char *)name)));
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

#define check_sm_fsm(L) (SMFSM *)luaL_checkudata(L, 1, "sm.fsm")

static int new_fsm(lua_State *L) {
	SMAppTable *at = check_sm_apptab(L);
	const char *s = luaL_checkstring(L, 2);
	SMFSM *f = (SMFSM *)lua_newuserdata(L, sizeof(SMFSM));
	f->native = sm_fsm_create(s, at->native);
	if(f->native == NULL)
		return luaL_error(L, "@malloc()");
	luaL_getmetatable(L, "sm.apptable");
	lua_setmetatable(L, -2);
	return 1;
}	

static int collect_fsm(lua_State *L) {	
	SMFSM *f = check_sm_fsm(L);
	if(f == NULL)
		return 0;
	sm_fsm_free(f->native);
	return 0;
}	

static const struct luaL_Reg smfsm_m [] = {
	{"__gc", collect_fsm},
	{NULL, NULL}
};

/**********************
 ****** SM.STATE ******
 **********************/
//..


/**********************
 ****** SM.ARRAY ******
 **********************/
//..


/**********************
 ******* SM.LIB *******
 **********************/

static const struct luaL_Reg smlib_f [] = {
	{"new_event", newevent},
	{"ne", newevent},
	{"new_queue", newqueue},
	{"nq", newqueue},
	{"new_queue2", newqueue2},
	{"nq2", newqueue2},
	{"loadlib", load_applib},
	{"lookup", lookup_app}, 
	{"new_apptab", new_apptab},
	{"nat", new_apptab},
	{"new_fsm", new_fsm},
	{"nf", new_fsm},
	{NULL, NULL}
};

int luaopen_sm (lua_State *L) {
	luaL_newmetatable(L, "sm.event");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, smevent_m, 0);
	
	luaL_newmetatable(L, "sm.queue");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, smqueue_m, 0);

	luaL_newmetatable(L, "sm.queue2");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, smqueue2_m, 0);
	
	luaL_newmetatable(L, "sm.apptable");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, smapptable_m, 0);
	
	luaL_newmetatable(L, "sm.fsm");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, smfsm_m, 0);
	
	luaL_newlib(L, smlib_f);
	return 1;
};