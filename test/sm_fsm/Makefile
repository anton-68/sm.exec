# SM.EXEC
# FSM module unit tests makefile
# anton.bondarenko@gmail.com 

CC		    := gcc
LD			:= gcc
INCLUDE		:= /usr/local/include
CFLAGS		:= -g -I$(INCLUDE)
UFLAGS		:= -g -U__linux__
LIBDIR		:= /usr/local/lib
LFLAGS		:= -L$(LIBDIR)
LIBS		:= -lpthread -ljsmn
SRC			:= ../../src/
OAM			:= ../../oam/
WRAP		:= ../../wrap/
BJHASH		:= ../../lib/bj_hash/

.PHONY					:	all
all						:	
							$(CC) $(CFLAGS) \
							$(SRC)sm_sys.c \
							$(SRC)sm_event.c \
							$(SRC)sm_queue.c \
							$(SRC)sm_queue2.c \
							$(SRC)sm_app.c \
							$(SRC)sm_fsm.c \
							$(SRC)sm_state.c \
							$(SRC)sm_array.c \
							$(OAM)logger.c \
							$(BJHASH)bj_hash.c \
							test_sm_fsm.c \
							$(LFLAGS) \
							$(LIBS)

.PHONY					:	clean
clean:
							rm -f *.o 

.PHONY					:	rebuild
rebuil d:					clean all
