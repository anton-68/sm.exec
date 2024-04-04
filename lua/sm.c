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

typedef struct SMEvent {
	sm_event *native;
} SMEvent;

static sm_event *create_event(size_t plsize, SMEvent *e) {
	sm_event *ne = sm_event_create(plsize + sizeof(* SMEvent));
	if(ne == NULL)
		return NULL;
	memset(ne->data, '\0', plsize);
	*(ne->data + plsize) = (void *)e;  // Wrapper address
	return ne;
}

static int newevent(lua_State *L) {
	size_t plsize = (size_t)luaL_checkinteger(L, 1); 
	luaL_argcheck(L, plsize >= 0, 1, "wrong event payload size");
	SMEvent *e = (SMEvent *)lua_nuewuserdata(L, sizeof(SMEvent));
	e->native = create_event(plsize, e);
	if(e->native == NULL)
		return luaL_error(L, "@malloc(%I)", (lua_Integer)plsize));
	luaL_getmetatable(L, "sm.event");
	lua_setmetatable(L, -2);
	return 1;
}

static const struct luaL_Reg smlib_f [] = {
	{"new_event", newevent},
	{NULL, NULL}
};

#define check_sm_event(L) (SMEvent *)luaL_checkudata(L, 1, "sm.event")

static int setevent(lua_State *L) {	
	SMEvent *e = check_sm_event(L);
	const char *s = luaL_checkstring(L, 2);
	strncpy((char *)e->native->data, s, e->native->data_size - sizeof(SMEvent *) - 1);
	return 0;
}
	
static int getevent(lua_State *L) {	
	SMEvent *e = check_sm_event(L);
	lua_pushstring(L, (const char *)e->native->data);
	return 1;
}
	
static int event_plsize(lua_State *L) {	
	SMEvent *e = check_sm_event(L);
	lua_pushinteger(L, (lua_Integer)e->native->data_size);
	return 1;
}	

static int printevent(lua_State *L) {	
	SMEvent *e = check_sm_event(L);
	lua_pushfstring(L, "sm_event @ %p:%p : %s", e, e->native, (const char *)e->native->data);
	return 1;
}

static int freeevent(lua_State *L) {
	SMEvent *e = check_sm_event(L);
	sm_event_free(e->native);
	return 0;
}
	
static const struct luaL_Reg smevent_m = {
	{"set", setevent},
	{"get", getevent},
	{"__len", event_plsize},
	{"__tostring", printevent},
	{"__gc", freeevent},
	{NULL, NULL}
};
	
int luaopen_sm (lua_State *L) {
	luaL_newmetatable(L, "sm.event");
	lua_pushvalue(L, 1);
	lua_setfileld(L, -2, "__index");
	luaL_setfucns(L, smevent_m, 0);
	luaL_newlib(L, smlib_f);
	return 1;
};