# SM.EXEC
# Makefile for Lua wrapper
# anton.bondarenko@gmail.com 

NAME = sm
CC = gcc
CFLAGS = -fPIC -O2 -g -Wall -pthread
DFLAGS = -fPIC -Og -ggdb -Wall -Werror -pthread
LFLAGS = -pthread -shared
LIBS = -lpthread -ljsmn

OAM = ../../oam/
LUA = ../../lua/
SRC = ../../src/
BJHASH = ../../lib/bj_hash/
JSMN = ../../lib/jsmn/
LUAINC = /usr/local/src/lua-5.3.4/

OBJS = 	$(LUA)sm.o \
		$(SRC)sm_event.o \
		$(SRC)sm_queue.o \
		$(SRC)sm_pqueue.o \
		$(SRC)sm_queue2.o \
		$(SRC)sm_app.o \
		$(SRC)sm_sys.o \
		$(SRC)sm_fsm.o \
		$(SRC)sm_state.o \
		$(SRC)sm_array.o \
		$(SRC)sm_thread.o \
		$(BJHASH)bj_hash.o \
		$(JSMN)jsmn.o \
		$(OAM)logger.o

all : $(NAME).so

$(NAME).so : $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) $(LIBS) -o $(NAME).so

%.o : %.c
	$(CC) $(DFLAGS) -I$(LUAINC) -c $< -o $@
	
.PHONY : clean
clean :
	rm -f $(OBJS) $(NAME).so
