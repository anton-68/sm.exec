# SM.EXEC
# Makefile for Lua wrapper
# anton.bondarenko@gmail.com 

TOP= /usr/local
LIB= $(TOP)/lib
INC= $(TOP)/include
OAM= /home/anton/lab/sm.exec/v5/oam
LUA= /home/anton/lab/sm.exec/v5/lua
SRC= /home/anton/lab/sm.exec/v5/src
CC= gcc
LD= gcc
CFLAGS= -g -O2 -Wall -I$(INC)
LDFLAGS= -shared
#LDFLAGS= -Wl,-E
LDLIBS= -lm -lpthread
#MYLIBS= -lm -Wl,-E -ldl -lreadline -lhistory -lncurses
RM= rm -f

SRCS = $(LUA)sm.c $(SRC)sm_event.c $(OAM)logger.c
OBJS = $(SRCS:.c=.o)

.PHONY:	all
all: sm.so

sm.so: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

$(SRCS:.c=.d):%.d:%.c
	$(CC) $(CFLAGS) -MM $< >$@
	
include $(SRCS:.c=.d)

.PHONY: clean
clean:
	-$(RM) $(OBJS) $(SRCS:.c=.d)
	