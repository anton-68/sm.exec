# SM.EXEC
# Makefile for Lua wrapper
# anton.bondarenko@gmail.com 

LUAINC= /home/anton/lab/sandbox/lua/lua-5.3.4/src
TOP= /usr/local
LIB= $(TOP)/lib
INC= $(TOP)/include
OAM= /home/anton/lab/sm.exec/v5/oam/
LUA= /home/anton/lab/sm.exec/v5/lua/
SRC= /home/anton/lab/sm.exec/v5/src/
CC= gcc
LD= gcc
CFLAGS= -g -O2 -fPIC -Wall -I$(INC) -I$(LUAINC)
LDFLAGS= -shared
#LDFLAGS= -Wl,-E
LDLIBS= -lm -pthread
#MYLIBS= -lm -Wl,-E -ldl -lreadline -lhistory -lncurses
RM= rm -f

SRCS = $(LUA)sm.c $(SRC)sm_event.c $(SRC)sm_queue.c $(SRC)sm_sys.c $(OAM)logger.c
OBJS = $(SRCS:.c=.o)

.PHONY:	all
all: sm.so

sm.so: $(OBJS)
	$(CC) $(LDFLAGS) $(LDLIBS) -o $@ $^

$(SRCS:.c=.d):%.d:%.c
	$(CC) $(CFLAGS) -MM $< >$@
	
include $(SRCS:.c=.d)

.PHONY: clean
clean:
	-$(RM) $(OBJS) $(SRCS:.c=.d)
	