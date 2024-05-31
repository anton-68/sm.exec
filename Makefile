# SM.EXEC
# General Makefile
# anton.bondarenko@gmail.com 

# SM_DEBUG 		- Detailed log
# SM_CONSOLE	- Print erors also to console
# SM_RFC5424	- [Experimental] RFC 5424 Syslog format

SM_LUA_NAME = sm
SM_APP = sm_test_apps
SM_ROBOT = sm_robot_demo_apps
SM_TEST = test_sm_2

CC = gcc
CFLAGS = -fPIC -O2 -g -Wall -pthread
DFLAGS = -fPIC -O0 -ggdb -Wall -Werror -pthread -DSM_DEBUG 
LFLAGS = -pthread -shared
LIBS = -lpthread -ljsmn -lm

LUA = lua/
SRC = src/
JSMN = lib/jsmn/
LUAINC = /usr/local/src/lua-5.3.5/
INCLUDE	= /usr/local/include


#		$(SRC)sm_array.o \
#		$(SRC)sm_exec.o \
#		$(SRC)sm_fsm.o \
#		$(SRC)sm_state.o \

OBJS = 	$(SRC)sm_app.o \
		$(SRC)sm_directory.o \
		$(SRC)sm_event.o \
		$(SRC)sm_hash.o \
		$(SRC)sm_logger.o \
		$(SRC)sm_partition.o \
		$(SRC)sm_pqueue.o \
		$(SRC)sm_queue.o \
		$(SRC)sm_queue2.o \
		$(SRC)sm_sys.o \
		$(SRC)sm_tape.o \
		$(JSMN)jsmn.o

comp : $(OBJS)		

%.o : %.c
	$(CC) -c $< -o $@ $(DFLAGS) -I$(INCLUDE)

LUA_OBJS = $(OBJS) $(LUA)sm.o

lua : $(SM_NAME).so

$(SM_NAME).so : $(LUA_OBJS)
	$(CC) -shared $(LUA_OBJS) $(LIBS) -o $(LUA)$(SM_NAME).so

%.o : %.c
	$(CC) -c $< -o $@ $(DFLAGS) -I$(LUAINC) -I$(INCLUDE)
	
.PHONY : clean
clean :
	rm -f $(OBJS)
	rm -f $(LUA)sm.o

all	: lua clean
