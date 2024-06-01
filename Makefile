# SM.EXEC
# General Makefile
# anton.bondarenko@gmail.com 

SM_NAME = sm
SM_APP = dtag_tc_apps
SM_TEST_FSM = test_sm_fsm
SM_TEST_JSON = test_sm_json
SM_TEST_APPLY = test_sm_apply
DT_USE_CASES = dt_use_cases

CC = gcc
CFLAGS = -fPIC -O2 -g -Wall -pthread
DFLAGS = -fPIC -O0 -ggdb -Wall -Werror -pthread
LFLAGS = -pthread -shared
LIBS = -lpthread -lm

LUA = lua/
SRC = src/
APP	= app/
TEST_FSM = test/fsm/
TEST_JSON = test/json/
TEST_APPLY = test/apply/
DTMP = test/dtmp/
BJHASH = lib/bj_hash/
JSMN = lib/jsmn/
PARSON = lib/parson/
LUAINC = /usr/local/src/lua-5.3.5/
INCLUDE	= /usr/local/include

OBJS = 	$(SRC)sm_app.o \
		$(SRC)sm_apply.o \
		$(SRC)sm_array.o \
		$(SRC)sm_directory.o \
		$(SRC)sm_event.o \
		$(SRC)sm_exec.o \
		$(SRC)sm_fsm.o \
		$(SRC)sm_logger.o \
		$(SRC)sm_pqueue.o \
		$(SRC)sm_queue.o \
		$(SRC)sm_queue2.o \
		$(SRC)sm_state.o \
		$(SRC)sm_sys.o \
		$(SRC)sm_tx.o \
		$(BJHASH)bj_hash.o \
		$(JSMN)jsmn.o \
		$(PARSON)parson.o \
		
LUA_OBJS = 	$(OBJS) $(LUA)sm.o
APP_OBJS =	$(OBJS) $(APP)dtag_tc_apps.o
TEST_FSM_OBJS = $(OBJS) $(TEST_FSM)test_sm_fsm.o
TEST_JSON_OBJS = $(OBJS) $(TEST_JSON)test_sm_json.o
TEST_APPLY_OBJS = $(APP_OBJS) $(TEST_APPLY)test_sm_apply.o
DT_USE_CASES_OBJS = $(APP_OBJS) $(DTMP)dt_use_cases.o

lua : $(SM_NAME).so

$(SM_NAME).so : $(LUA_OBJS)
	$(CC) -shared $(LUA_OBJS) $(LIBS) -o $(LUA)$(SM_NAME).so

app : $(SM_APP).so
	
$(SM_APP).so : $(APP_OBJS)
	$(CC) $(LFLAGS) $(APP_OBJS) $(LIBS) -o $(APP)$(SM_APP).so
	
test : $(SM_TEST_FSM) $(SM_TEST_JSON) $(SM_TEST_APPLY) $(DT_USE_CASES) app clean
	
$(SM_TEST_FSM) : $(TEST_FSM_OBJS)
	$(CC) -lpthread $(TEST_FSM_OBJS) $(LIBS) -ldl -o $(TEST_FSM)$(SM_TEST_FSM)

$(SM_TEST_JSON) : $(TEST_JSON_OBJS)
	$(CC) -lpthread $(TEST_JSON_OBJS) $(LIBS) -ldl -o $(TEST_JSON)$(SM_TEST_JSON)

$(SM_TEST_APPLY) : $(TEST_APPLY_OBJS)
	$(CC) -lpthread $(TEST_APPLY_OBJS) $(LIBS) -ldl -o $(TEST_APPLY)$(SM_TEST_APPLY)

$(DT_USE_CASES) : $(DT_USE_CASES_OBJS)
	$(CC) -lpthread $(DT_USE_CASES_OBJS) $(LIBS) -ldl -o $(DTMP)$(DT_USE_CASES)

%.o : %.c
	$(CC) -c $< -o $@ $(DFLAGS) -I$(LUAINC) -I$(INCLUDE)
	
.PHONY : clean
clean :
	rm -f $(OBJS)
	rm -f $(LUA)sm.o
	rm -f $(APP)dtag_tc_apps.o
	rm -f $(TEST_FSM)test_sm_fsm.o
	rm -f $(TEST_JSON)test_sm_json.o
	rm -f $(TEST_APPLY)test_sm_apply.o
	rm -f $(DTMP)dt_use_cases.o

all	: test lua clean
