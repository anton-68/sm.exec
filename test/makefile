# SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
# Event class tests
# -------------------------------------------------------------------------------
# Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
# -------------------------------------------------------------------------------
# SPDX-License-Identifier: LGPL-3.0-only */

CC = gcc
INCLUDE	= /usr/local/include
CFLAGS = -fPIC -O2 -g -Wall
DFLAGS = -fPIC -O0 -ggdb -Wall -Werror

SRC = ../src/
OBJS = 	$(SRC)sm_event.o \
		$(SRC)sm_sys.o \
		test_event.o

test_event : $(OBJS)
	$(CC) $(OBJS) -o test_event

%.o : %.c
	$(CC) -c $< -o $@ $(DFLAGS) -I$(INCLUDE)

.PHONY : clean
clean :
	rm -f $(OBJS)