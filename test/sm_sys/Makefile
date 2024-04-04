# TrafficLab
# Core module unit tests makefile
# anton.bondarenko@gmail.com 
CC		    := gcc
LD			:= gcc
INCLUDE		:= 
CFLAGS		:= -g
UFLAGS		:= -g -U__linux__
LFLAGS		:= 
LIBDIR		:= 
LIBS		:= -lpthread
LIBS_D		:=
OBJECTS		:=
SRC			:= ../../src/
OAM			:= ../../oam/
WRAP		:= ../../wrap/

.PHONY					:	all
all						:	
							$(CC) $(CFLAGS) \
							$(SRC)sm_sys.c \
							$(OAM)logger.c \
							test_sm_sys.c \
							-o linux \
							$(LIBS)
							$(CC) $(UFLAGS) \
							$(SRC)sm_sys.c \
							$(OAM)logger.c \
							test_sm_sys.c \
							-o posix \
							$(LIBS)

.PHONY					:	clean
clean:
							rm -f *.o 

.PHONY					:	rebuild
rebuil d:					clean all
