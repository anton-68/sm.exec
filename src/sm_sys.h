/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
System-wide utilities and definintions
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#ifndef SM_SYS_H
#define SM_SYS_H

#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

/* Log & debug flags*/
#define SM_LOG
#define SM_DEBUG

/* Timestamp */
typedef struct sm_timestamp
{
    time_t seconds;
    long nanoseconds;
    char timestring[32];
} sm_timestamp;

sm_timestamp sm_get_timestamp();

/* MIN & MAX - do we still need them? */
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) >= (Y)) ? (X) : (Y))

/* Branching hints */
#define SM_LIKELY(X) __builtin_expect(!!(X), 1)
#define SM_UNLIKELY(X) __builtin_expect(!!(X), 0)

/* Mutex type */
#define SM_MUTEX_TYPE PTHREAD_MUTEX_NORMAL
                   /* PTHREAD_MUTEX_DEFAULT */
                   /* PTHREAD_MUTEX_ERRORCHECK */

/* Thread ID */
unsigned long get_tid();
void get_tid_str(char *);

/* System word (ILP64 and LP32) */
#define SM_WORD sizeof(void *)

#endif //SM_SYS_H
