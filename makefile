# SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
# SM.EXEC. so makefile
# -------------------------------------------------------------------------------
# Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
# -------------------------------------------------------------------------------
# SPDX-License-Identifier: LGPL-3.0-only */

CC = gcc
INCLUDE	= -I/usr/local/include -Iinclude
CFLAGS = -std=gnu11 -fPIC -O2 -g -Wall -pthread
DFLAGS = -std=gnu11 -fPIC -O0 -ggdb -Wall -Werror -pthread -fno-stack-protector
LFLAGS = -pthread
BUILD = build/
LIBS = -lpthread -lm

SRC = src/

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
		lib/bj_hash/bj_hash.o \
		lib/jsmn/jsmn.o

all : sm.so

sm.so : $(OBJS)
	$(CC) $(LFLAGS) -shared $(OBJS) $(LIBS) -o $(BUILD)sm.so

%.o : %.c
	$(CC) $(LFLAGS) -c $< -o $@ $(DFLAGS) $(INCLUDE)

.PHONY : clean
clean :
	rm -f $(OBJS)