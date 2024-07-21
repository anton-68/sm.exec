# SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
# Event class tests
# -------------------------------------------------------------------------------
# Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
# -------------------------------------------------------------------------------
# SPDX-License-Identifier: LGPL-3.0-only */

CC = gcc
INCLUDE	= /usr/local/include
CFLAGS = -std=gnu11 -fPIC -O2 -g -Wall -pthread
DFLAGS = -std=gnu11 -fPIC -O0 -ggdb -Wall -Werror -pthread -fno-stack-protector
LFLAGS = -pthread
LIBS = -lpthread -lm

SRC = ../src/

OBJS = 	$(SRC)sm_sys.o \
		$(SRC)sm_logger.o \
		$(SRC)sm_event.o \
		$(SRC)sm_queue.o \
		$(SRC)sm_state.o \
		test_utils.o

TEST_OBJS =	test_sys.o \
			test_logger.o \
			test_event.o \
			test_queue.o \
			test_state.o

EXES = test_sys test_logger test_event test_queue test_state

all : $(EXES)

test_sys : $(OBJS) test_sys.o
	$(CC) $(LFLAGS) $(OBJS) test_sys.o -o test_sys

test_logger : $(OBJS) test_logger.o
	$(CC) $(LFLAGS) $(OBJS) test_logger.o -o test_logger

test_event : $(OBJS) test_event.o
	$(CC) $(LFLAGS) $(OBJS) test_event.o -o test_event

test_queue : $(OBJS) test_queue.o
	$(CC) $(LFLAGS) $(OBJS) test_queue.o $(LIBS) -o test_queue

test_state : $(OBJS) test_state.o
	$(CC) $(LFLAGS) $(OBJS) test_state.o $(LIBS) -o test_state

%.o : %.c
	$(CC) $(LFLAGS) -c $< -o $@ $(DFLAGS) -I$(INCLUDE)

.PHONY : clean
clean :
	rm -f $(OBJS)
	rm -f $(TEST_OBJS)
	rm -f $(EXES)