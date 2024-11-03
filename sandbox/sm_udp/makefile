# SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
# SM.EXEC Simple UDP RA
# -------------------------------------------------------------------------------
# Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
# -------------------------------------------------------------------------------
# SPDX-License-Identifier: LGPL-3.0-only */

CC = gcc
INCLUDE	= -I/usr/local/include -I../include
CFLAGS = -std=gnu11 -fPIC -O2 -g -Wall -pthread
DFLAGS = -std=gnu11 -fPIC -O0 -ggdb -Wall -Werror -pthread -fno-stack-protector -fdump-rtl-expand
LFLAGS = -pthread
BUILD = build/
LIBS = -lpthread -lm
SM_EXEC = ../build/sm.so
LIB = ../lib/

SRC = ""

OBJS = 	$(SRC)udp_apps.o \
		$(SRC)bj_hash/bj_hash.o \
		$(LIB)jsmn/jsmn.o

all : sm_udp_server sm_udp_client

sm_udp_server : $(OBJS)
	$(CC) $(LFLAGS) -shared $(OBJS) $(LIBS) $(SM_EXEC) -o $(BUILD)sm_udp_server

sm_udp_client : $(OBJS)
	$(CC) $(LFLAGS) -shared $(OBJS) $(LIBS) $(SM_EXEC) -o $(BUILD)sm_udp_client

%.o : %.c
	$(CC) $(LFLAGS) -c $< -o $@ $(DFLAGS) $(INCLUDE)

.PHONY : clean
clean :
	rm -f $(OBJS)