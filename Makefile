# SM.EXEC
# General Makefile
# anton.bondarenko@gmail.com 

SM_NAME = sm
SM_APP = sm_test_apps
SM_TEST = test_sm_2

CC = gcc
CFLAGS = -fPIC -O2 -g -Wall -pthread
DFLAGS = -fPIC -Og -ggdb -Wall -Werror -pthread
LFLAGS = -pthread -shared
LIBS = -lpthread -ljsmn

OAM = oam/
LUA = lua/
SRC = src/
APP	= app/
TEST = test/sm_thread0/
BJHASH = lib/bj_hash/
JSMN = lib/jsmn/
LUAINC = /usr/local/src/lua-5.3.5/
INCLUDE	= /usr/local/include

OBJS = 	$(SRC)sm_app.o \
		$(SRC)sm_array.o \
		$(SRC)sm_directory.o \
		$(SRC)sm_event.o \
		$(SRC)sm_exec.o \
		$(SRC)sm_fsm.o \
		$(SRC)sm_memory.o \
		$(SRC)sm_pqueue.o \
		$(SRC)sm_queue.o \
		$(SRC)sm_queue2.o \
		$(SRC)sm_state.o \
		$(SRC)sm_sys.o \
		$(BJHASH)bj_hash.o \
		$(JSMN)jsmn.o \
		$(OAM)logger.o
		
LUA_OBJS = 	$(OBJS) $(LUA)sm.o
APP_OBJS =	$(OBJS) $(APP)sm_test_apps.o
TEST_OBJS = $(OBJS) $(TEST)test_sm_2.o

lua : $(SM_NAME).so

$(SM_NAME).so : $(LUA_OBJS)
	$(CC) -shared $(LUA_OBJS) $(LIBS) -o $(LUA)$(SM_NAME).so

app : $(SM_APP).so
	
$(SM_APP).so : $(APP_OBJS)
	$(CC) $(LFLAGS) $(APP_OBJS) $(LIBS) -o $(APP)$(SM_APP).so

test : $(SM_TEST)
	
$(SM_TEST) : $(TEST_OBJS)
	$(CC) -lpthread $(TEST_OBJS) $(LIBS) -ldl -o $(TEST)$(SM_TEST)

%.o : %.c
	$(CC) -c $< -o $@ $(DFLAGS) -I$(LUAINC) -I$(INCLUDE)
	
.PHONY : clean
clean :
	rm -f $(OBJS)
	rm -f $(LUA)sm.o
	rm -f $(APP)sm_test_apps.o
	rm -f $(TEST)test_sm_2.o

all	: lua app test clean
