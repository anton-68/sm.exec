# SM.EXEC
# General Makefile
# anton.bondarenko@gmail.com 

# SM_DEBUG 		- Detailed log
# SM_CONSOLE	- Print erors also to console
# SM_RFC5424	- [Experimental] RFC 5424 Syslog format

SM_TEST = test_eq

CC = gcc
CFLAGS = -fPIC -O2 -g -Wall -pthread
DFLAGS = -fPIC -O0 -ggdb -Wall -Werror -pthread -DSM_DEBUG 
LFLAGS = -pthread -shared
LIBS = -lpthread -lm

SRC = ../../src/
INCLUDE	= /usr/local/include

OBJS = 	$(SRC)sm_event.o \
		$(SRC)sm_hash.o \
		$(SRC)sm_logger.o \
		$(SRC)sm_queue.o \
		$(SRC)sm_sys.o \
		test_eq.o

comp : $(OBJS)		

%.o : %.c
	$(CC) -c $< -o $@ $(DFLAGS) -I$(INCLUDE)

test : $(SM_NAME)

$(SM_NAME) : $(OBJS)
	$(CC) -shared $(OBJS) $(LIBS) -o $(SM_NAME)

%.o : %.c
	$(CC) -c $< -o $@ $(DFLAGS) -I$(INCLUDE)
	
.PHONY : clean
clean :
	rm -f $(OBJS)

all	: test clean
