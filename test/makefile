# SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
# Event class tests
# -------------------------------------------------------------------------------
# Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
# -------------------------------------------------------------------------------
# SPDX-License-Identifier: LGPL-3.0-only */

CC = gcc
INCLUDE	= /usr/local/include
CFLAGS = -std=gnu11 -fPIC -O2 -g -Wall 
DFLAGS = -std=gnu11 -fPIC -O0 -ggdb -Wall -Werror

SRC = ../src/
OBJS = 	$(SRC)sm_event.o \
		$(SRC)sm_logger.o \
		$(SRC)sm_sys.o
TEST_OBJS =	test_event.o \
			test_sys.o

all : test_event test_sys clean

test_event : $(OBJS) test_event.o
	$(CC) $(OBJS) test_event.o -o test_event

test_sys : $(OBJS) test_sys.o
	$(CC) $(OBJS) test_sys.o -o test_sys

%.o : %.c
	$(CC) -c $< -o $@ $(DFLAGS) -I$(INCLUDE)

.PHONY : clean
clean :
	rm -f $(OBJS)
	rm -f $(TEST_OBJS)