/* SM.EXEC 
   Lua wrapper
   anton.bondarenko@gmail.com */


#include "lprefix.h"

#include <stdlib.h>
#include <string.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "../src/sm_event.h"
#include "../src/sm_queue.h"

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
	lua_e->linked = 1;
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

// Wrapper for next from wrapper for this
#define next(E1) (*((SMEvent **)(E1->native->next->data \
                               + E1->native->next->data_size \
							   - sizeof(SMEvent *))))

// Wrapper for this from native for this
#define this(E1) (*((SMEvent **)(E1->data + E1->data_size - sizeof(SMEvent *))))

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

static int freeevent(lua_State *L) {
	SMEvent *e = check_sm_event(L);
	if(e->native == NULL)
		return 0;
	if(e->linked > 0)
		return 0;
	if(e->native->next != NULL)
		next(e)->linked--;
	sm_event_free(e->native);
	e->native = NULL;
	return 0;
}

static int collectevent(lua_State *L) {
	SMEvent *e = check_sm_event(L);
	if(e->linked)
		return 0;
	else
		return freeevent(L);
}
	
static const struct luaL_Reg smevent_m [] = {
	{"set", setevent},
	{"get", getevent},
	{"free", freeevent},
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
		lua_e = (*((SMEvent **)(e->data + e->data_size - sizeof(SMEvent *))));
		lua_pushfstring(L, "sm_event @ %p : %p -> %p, linked = %I\n", 
						lua_e, e, e->next, 
						(lua_e == NULL) ? -1 : lua_e->linked);
		luaL_addvalue(&b);		
		e = e->next;
	}
	luaL_pushresult(&b);
	return 1;
}

static int freequeue(lua_State *L) {
	SMQueue *q = check_sm_queue(L);
	sm_queue_free(q->native);
	return 0;
}

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
		lua_e->linked = 0;
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
	if(this(top_e) != NULL)
		lua_pushlightuserdata(L, this(top_e));
	else {
		SMEvent *lua_e = (SMEvent *)lua_newuserdata(L, sizeof(SMEvent));
		luaL_getmetatable(L, "sm.event");
		lua_setmetatable(L, -2);
		lua_e->native = top_e;
		lua_e->linked = 0;
		if(next(lua_e) != NULL)
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
	return 0;
}

static const struct luaL_Reg smqueue_m [] = {
	{"top", queuetop},
	{"enqueue", enqueue},
	{"dequeue", dequeue},
	//{"to_array", to_array},
	{"__len", queue_size},
	{"__tostring", printqueue},
	{"__gc", freequeue},
	{NULL, NULL}
};



/**********************
 ******* SM.LIB *******
 **********************/

static const struct luaL_Reg smlib_f [] = {
	{"new_event", newevent},
	{"ne", newevent},
	{"new_queue", newqueue},
	{"nq", newqueue},
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
	
	luaL_newlib(L, smlib_f);
	return 1;
};