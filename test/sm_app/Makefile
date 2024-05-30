# SM.EXEC
# FSM module unit tests makefile
# anton.bondarenko@gmail.com 

NAME = sm_test_apps
CC = gcc
CFLAGS = -fPIC -O2 -g -Wall -pthread
DFLAGS = -fPIC -Og -ggdb -Wall -Werror -pthread
LFLAGS = -pthread -shared
LIBS = -lpthread
OAM = ../../oam
SRC = ../../src

OBJECTS = 	$(SRC)/sm_sys.o  \
			$(SRC)/sm_event.o \
			$(SRC)/sm_queue.o \
			$(SRC)/sm_app.o \
			$(OAM)/logger.o \
			sm_test_apps.o

all : $(NAME).so

$(NAME).so : $(OBJECTS)
	$(CC) $(LFLAGS) $(OBJECTS) $(LIBS) -o $(NAME).so

%.o : %.c
	$(CC) $(DFLAGS) -c $< -o $@
	
.PHONY : clean
clean :
	rm -f $(OBJECTS)
