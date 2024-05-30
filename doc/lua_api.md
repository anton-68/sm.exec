**** SM.EXEC

Singleton, created on init, exposed to Lua as global name "sm_exec". Not expected to be used explicitly.


**** SM.DIRECTORY

Not accessible in Lua other than through global sm_exec. Initialized on load of SM module. 


**** SM.EVENT

Create event:
e = sm.new_event(size)

Set event data:
e:set(string)

Get event data:
e:get() -> string

Set event id:
e:setid(int)

Get event id:
e:getid() -> int

Set event priority (two values):
e:setpriority(int, int)

Get event priority:
e:getpriority -> [int, int]

Get event data by app id:
e[string] -> string

Get event data size:
#e -> int

Chain two events:
e0..e1 -> e1

Unchain event (drop tail):
-e -> e

Print event:
print(e)

Collect event:
Supprted with analysis of dependencies (queueving and linking)


**** SM.QUEUE


// stk: qsize, plsize, sync
static int new_queue(lua_State *L) {
// stk: q
static int queue_size(lua_State *L) {	
// stk: q
static int queue_tostring(lua_State *L) {
// stk: q
static int collect_queue(lua_State *L) {
// stk: q
static int queuetop(lua_State *L) {
// stk: q
static int dequeue(lua_State *L) {
// stk: q, e
static int enqueue(lua_State *L) {
// stk: queue, name
static int dir_set_queue(lua_State *L) {

	{"top", queuetop},
	{"enqueue", enqueue},
	{"dequeue", dequeue},
	{"set", dir_set_queue},
	{"__len", queue_size},
	{"__tostring", queue_tostring},
	{"__gc", collect_queue},
	{NULL, NULL}


**** SM.PQUEUE



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

// stk: pqueue, name
static int dir_set_pqueue(lua_State *L) {
	SMPQueue *pq = check_sm_pqueue(L);
	const char *name = luaL_checkstring(L, 2);
	lua_getglobal(L, "sm_exec");
	SMExec *e = (SMExec *)lua_touserdata(L, -1);
	e->native->dir = sm_directory_set(e->native->dir, name, (void *)pq->native);
	return 0;
}

static const struct luaL_Reg smpqueue_m [] = {
	{"top", pqueuetop},
	{"enqueue", penqueue},
	{"dequeue", pdequeue},
	{"set", dir_set_pqueue},
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

// stk: queue2, name
static int dir_set_queue2(lua_State *L) {
	SMQueue2 *q = check_sm_queue2(L, 1);
	const char *name = luaL_checkstring(L, 2);
	lua_getglobal(L, "sm_exec");
	SMExec *e = (SMExec *)lua_touserdata(L, -1);
	e->native->dir = sm_directory_set(e->native->dir, name, (void *)q->native);
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
	{"set", dir_set_queue2},
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

// Defining also SMState ahead of the actual class definition
typedef struct SMState {
	sm_state *native;
	bool standalone;
} SMState;

#define check_sm_state(L, POS) (SMState *)luaL_checkudata((L), (POS), "sm.state")
#define test_sm_state(L, POS) (SMState *)luaL_testudata((L), (POS), "sm.state")


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

// stk: app, name
static int dir_set_app(lua_State *L) {
	SMApp *a = check_sm_app(L, 1);
	const char *name = luaL_checkstring(L, 2);
	lua_getglobal(L, "sm_exec");
	SMExec *e = (SMExec *)lua_touserdata(L, -1);
	e->native->dir = sm_directory_set(e->native->dir, name, (void *)a->native);
	return 0;
}	

// stk: name
static int dir_get_app(lua_State *L) {
	const char *name = luaL_checkstring(L, 1);
	lua_getglobal(L, "sm_exec");
	SMExec *e = (SMExec *)lua_touserdata(L, -1);
	sm_app na = *((sm_app *)sm_directory_get_ref(e->native->dir, name));
	if(push_app_handler(L, na) != EXIT_SUCCESS) 
		lua_pushnil(L);
	return 1;
}

static const struct luaL_Reg smapp_m [] = {
	{"set", dir_set_app},
	{"__tostring", app_tostring},
	{"__call", call_app},
	{"__gc", collect_app},
	{NULL, NULL}
};

// stk: file
static int load_applib(lua_State *L) {
	const char *fn = luaL_checkstring(L, 1);
	lua_pushlightuserdata(L, dlopen(fn, SM_LUA_RTLD_FLAG));
	return 1;
}



//DEPRECATED[
/************************
 **** SM.APP_TABLE ******
 ************************/

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
//]


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
	SMDirectory *dir = check_sm_directory(L, 2);
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
	sm_fsm *f = sm_fsm_create(fsmj, dir->native, t);
	if(f == NULL)
		return luaL_error(L, "@sm_fsm_create()");
	if(push_fsm_handler(L, f) != EXIT_SUCCESS)
		lua_pushnil(L);
	return 1;
}	

// stk: name
static int dir_get_fsm(lua_State *L) {
	const char *name = luaL_checkstring(L, 1);
	lua_getglobal(L, "sm_exec");
	SMExec *e = (SMExec *)lua_touserdata(L, -1);
	sm_fsm *f = *(sm_fsm **)sm_directory_get_ref(e->native->dir, name);
	if(push_fsm_handler(L, f) != EXIT_SUCCESS) 
		lua_pushnil(L);
	return 1;
}

// stk: fsm, name
static int dir_set_fsm(lua_State *L) {
	SMFSM *f = check_sm_fsm(L, 1);
	const char *name = luaL_checkstring(L, 2);
	lua_getglobal(L, "sm_exec");
	SMExec *e = (SMExec *)lua_touserdata(L, -1);
	e->native->dir = sm_directory_set(e->native->dir, name, (void *)f->native);
	f->lock = true;
	return 0;
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
	{"set", dir_set_fsm},
	{"__tostring", fsm_tostring},
	{"free", unlock_fsm},
	{"__gc", collect_fsm},
	{NULL, NULL}
};

//DEPRECATED[
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
//]

/**********************
 ****** SM.STATE ******
 **********************/

/* Defined ahead before SM.APP
typedef struct SMState {
	sm_state *native;
	bool standalone;
} SMState;


#define check_sm_state(L, POS) (SMState *)luaL_checkudata((L), (POS), "sm.state")
#define test_sm_state(L, POS) (SMState *)luaL_testudata((L), (POS), "sm.state")
*/

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
	sm_state *s;
	SMFSM *fsm = check_sm_fsm(L, 1);
	size_t plsize = (size_t)lua_tointeger(L, 2);
	if(fsm != NULL) {
		s = sm_state_create(fsm->native->ref, plsize);
		fsm->lock = true;
	}
	else {
		const char *fsm_name = luaL_checkstring(L, 1);
		lua_getglobal(L, "sm_exec");
		SMExec *e = (SMExec *)lua_touserdata(L, -1);
		sm_fsm ** fsm_ref = (sm_fsm **)sm_directory_get_ref(e->native->dir, fsm_name);
		s = sm_state_create(fsm_ref, plsize);	
	}
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

// stk: s, key
static int get_state_appdata(lua_State *L) {	
	SMState *s = check_sm_state(L, 1);
	const char *key = luaL_checkstring(L, 2);
	char *c0 = (char *)sm_chunk_find(s->native->data, sm_ipstr_to_id(key));
	if(c0 == NULL)
		lua_pushnil(L);
	else
		lua_pushlstring(L, c0, SM_CHUNK_LENGTH((sm_chunk *)(c0 - sizeof(sm_chunk))));
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
	{"__index", get_state_appdata},
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


/*******************
 ****** SM.TX ******
 *******************/

typedef struct SMTx {
	sm_tx *native;
} SMTx;

#define check_sm_tx(L, POS) (SMTx *)luaL_checkudata((L), (POS), "sm.tx")

//stk: queue, fsm, sync, size
static int new_tx(lua_State *L) {
	const char *queue = luaL_checkstring(L, 1);
	const char *fsm = luaL_checkstring(L, 2);
	bool sync;
	if(lua_isboolean(L, 3))
		sync = (bool)lua_toboolean(L, 3);
	else
		return luaL_error(L, "wrong synchronized flag value");
	size_t txsize = (size_t)luaL_checkinteger(L, 4);
	luaL_argcheck(L, txsize >= 0, 1, "wrong tx descriptor data size");
	if (txsize == 0)
		txsize = SM_TX_DATA_SIZE;
	size_t stsize = (size_t)luaL_checkinteger(L, 5);
	luaL_argcheck(L, stsize >= 0, 1, "wrong tx descriptor data size");
	if (stsize == 0)
		stsize = SM_STATE_DATA_SIZE;
	
	lua_getglobal(L, "sm_exec");
	SMExec *e = (SMExec *)lua_touserdata(L, -1);
	SMTx *tx = (SMTx *)lua_newuserdata(L, sizeof(SMTx));
	sm_fsm **fsm_ref = (sm_fsm **)sm_directory_get_ref(e->native->dir, fsm);
	sm_queue2 **queue_ref = (sm_queue2 **)sm_directory_get_ref(e->native->dir, queue);
	tx->native = sm_tx_create(e->native, fsm_ref, txsize, stsize, queue_ref, sync);
	if(tx->native == NULL)
		return luaL_error(L, "@new_tx()");
	tx->native->state->exec = e->native;
	luaL_getmetatable(L, "sm.exec");
	lua_setmetatable(L, -2);
	return 1;
}

//stk: tx
static int run_tx(lua_State *L) {
	SMTx *tx = check_sm_tx(L, 1);
	pthread_t t0;
	pthread_create(&t0, NULL, &sm_tx_runner, (void *)tx->native);
	return 0;
}

// stk: tx, key
static int get_tx_appdata(lua_State *L) {	
	SMTx *tx = check_sm_tx(L, 1);
	const char *key = luaL_checkstring(L, 2);
	char *c0 = (char *)sm_chunk_find(tx->native->data, sm_ipstr_to_id(key));
	if(c0 == NULL)
		lua_pushnil(L);
	else
		lua_pushlstring(L, c0, SM_CHUNK_LENGTH((sm_chunk *)(c0 - sizeof(sm_chunk))));
	return 1;
}

// stk: tx
static int tx_tostring(lua_State *L) {
	SMTx *tx = check_sm_tx(L, 1);
	lua_pushfstring(L, "sm_tx @ %p, size = %I\n", 
					tx, (lua_Integer)tx->native->data_size);
	return 1;
}

// stk: tx
static int collect_tx(lua_State *L) {
#ifdef SM_DEBUG	
	printf("collect_tx attempt ...");
#endif		
	return 0;
}	

static const struct luaL_Reg smtx_m [] = {
	{"__index", get_tx_appdata},
	{"run", run_tx},
	{"__tostring", tx_tostring},
	{"__gc", collect_tx},
	{NULL, NULL}
};


/**********************
 ******* SM.LIB *******
 **********************/

static const struct luaL_Reg smlib_f [] = {
	{"new_event", new_event},
	{"new_directory", new_directory},  // deprecated
	{"new_queue", new_queue},
	{"new_pqueue", new_pqueue},
	{"new_queue2", new_queue2},
	{"loadlib", load_applib},
	{"lookup", lookup_app},
	{"get_app", dir_get_app},
	{"new_apptab", new_apptab}, // deprecated	
	{"new_fsm", new_fsm},
	{"new_fsmtab", new_fsmtab}, // deprecated
	{"get_fsm", dir_get_fsm},	
	{"new_state", new_state},	
	{"new_array", new_array},	
	{"nex_tx", new_tx},	
	{NULL, NULL}
};

int luaopen_sm (lua_State *L) {

	lua_pushinteger(L, (lua_Integer)0);
	exec_init(L);
	lua_setglobal(L, "sm_exec");
	
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
	
	luaL_newmetatable(L, "sm.directory");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, smdirectory_m, 0);
	
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
	
	luaL_newmetatable(L, "sm.exec");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, smexec_m, 0);
	
	luaL_newmetatable(L, "sm.tx");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, smtx_m, 0);
	
	luaL_newlib(L, smlib_f);
	
	return 1;
};
