# SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
# Test classes makefile
# -------------------------------------------------------------------------------
# Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
# -------------------------------------------------------------------------------
# SPDX-License-Identifier: LGPL-3.0-only */

CC = gcc
INCLUDE	= -I/usr/local/include -I../include
CFLAGS = -std=gnu11 -fPIC -O2 -g -Wall -pthread
DFLAGS = -std=gnu11 -fPIC -O0 -ggdb -Wall -Werror -pthread -fno-stack-protector
LFLAGS = -pthread
LIBS = -lpthread -lm

SRC = ../src/

OBJS = 	$(SRC)sm_sys.o \
		$(SRC)sm_logger.o \
		$(SRC)sm_hash.o \
		$(SRC)sm_event.o \
		$(SRC)sm_queue.o \
		$(SRC)sm_queue2.o \
		$(SRC)sm_pqueue.o \
		$(SRC)sm_state.o \
		$(SRC)sm_array.o \
		$(SRC)sm_directory.o \
		$(SRC)sm_fsm.o \
		$(SRC)sm_tx.o \
		$(SRC)sm_exec.o \
		$(SRC)sm_apply.o \
		../lib/bj_hash/bj_hash.o \
		../lib/jsmn/jsmn.o \
		../lib/parson/parson.o \
		test_utils.o

TEST_OBJS =	test_sys.o \
			test_logger.o \
			test_hash.o \
			test_event.o \
			test_queue.o \
			test_queue2.o \
			test_pqueue.o \
			test_state.o \
			test_array.o \
			test_directory.o \
			test_fsm.o \
			test_tx.o \
			test_exec.o \
			test_apply.o

EXES = 	test_sys \
		test_logger \
		test_hash \
		test_event \
		test_queue \
		test_queue2 \
		test_pqueue \
		test_state \
		test_array \
		test_directory \
		test_fsm \
		test_tx \
		test_exec \
		test_apply

all : $(EXES) apply_apps.so chmod

APP_OBJS =	$(OBJS) apply_apps.o

apply_apps.so : $(APP_OBJS)
	$(CC) $(LFLAGS) -shared $(APP_OBJS) $(LIBS) -o apply_apps.so

test_sys : $(OBJS) test_sys.o
	$(CC) $(LFLAGS) $(OBJS) test_sys.o -o test_sys

test_logger : $(OBJS) test_logger.o
	$(CC) $(LFLAGS) $(OBJS) test_logger.o -o test_logger

test_hash : $(OBJS) test_hash.o
	$(CC) $(LFLAGS) $(OBJS) test_hash.o -o test_hash

test_event : $(OBJS) test_event.o
	$(CC) $(LFLAGS) $(OBJS) test_event.o -o test_event

test_queue : $(OBJS) test_queue.o
	$(CC) $(LFLAGS) $(OBJS) test_queue.o $(LIBS) -o test_queue

test_queue2 : $(OBJS) test_queue2.o
	$(CC) $(LFLAGS) $(OBJS) test_queue2.o $(LIBS) -o test_queue2

test_pqueue : $(OBJS) test_pqueue.o
	$(CC) $(LFLAGS) $(OBJS) test_pqueue.o $(LIBS) -o test_pqueue

test_state : $(OBJS) test_state.o
	$(CC) $(LFLAGS) $(OBJS) test_state.o $(LIBS) -o test_state

test_array : $(OBJS) test_array.o
	$(CC) $(LFLAGS) $(OBJS) test_array.o $(LIBS) -o test_array

test_directory : $(OBJS) test_directory.o
	$(CC) $(LFLAGS) $(OBJS) test_directory.o $(LIBS) -o test_directory

test_fsm : $(OBJS) test_fsm.o
	$(CC) $(LFLAGS) $(OBJS) test_fsm.o $(LIBS) -o test_fsm

test_tx : $(OBJS) test_tx.o
	$(CC) $(LFLAGS) $(OBJS) test_tx.o $(LIBS) -o test_tx

test_exec : $(OBJS) test_exec.o
	$(CC) $(LFLAGS) $(OBJS) test_exec.o $(LIBS) -o test_exec

test_apply : $(OBJS) test_apply.o
	$(CC) $(LFLAGS) $(OBJS) test_apply.o $(LIBS) -ldl -o test_apply

%.o : %.c
	$(CC) $(LFLAGS) -c $< -o $@ $(DFLAGS) $(INCLUDE)

.PHONY : chmod
chmod :
	chmod -x *.so

.PHONY : clean
clean :
	rm -f $(OBJS)
	rm -f $(TEST_OBJS)
	rm -f $(EXES)
	rm -f apply_apps.o apply_apps.so
