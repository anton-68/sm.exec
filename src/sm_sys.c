/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
System-цшву utilities and definintions
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#include "sm_sys.h"


/* thread id */

#ifdef __linux__
void get_tid_str(char *strID){
    sprintf(strID, "%i:%i", getpid(), (int)syscall(__NR_gettid));
}
#else
unsigned long gettid(){
    pthread_t ptid = pthread_self();
    unsigned long utid = 0;
    memcpy(&utid, &ptid, sizeof(utid)<sizeof(ptid)?sizeof(utid):sizeof(ptid));
    return utid;
}
void get_tid_str(char *strID){
    sprintf(strID, "%i:%lu", getpid(), (unsigned long)gettid());
}
#endif

#ifdef __linux__
unsigned long get_tid(){
    return (unsigned long)syscall(__NR_gettid);
}
#else
unsigned long get_tid(){
    pthread_t ptid = pthread_self();
    unsigned long utid = 0;
    memcpy(&utid, &ptid, sizeof(utid)<sizeof(ptid)?sizeof(utid):sizeof(ptid));
    return utid;
}
#endif

void sm_id_to_ipstr(SM_ID id, char* const ip) {
	sprintf(ip, "%d.%d.%d.%d", (id >> 24) & 0xFF, (id >> 16) & 0xFF, (id >>  8) & 0xFF, 
			(id) & 0xFF);
}

SM_ID sm_ipstr_to_id(const char* const ip) {
	unsigned byte3, byte2, byte1, byte0;
   	char dummyString[2];	
   	if (sscanf (ip, "%u.%u.%u.%u%1s",
			   &byte3, &byte2, &byte1, &byte0, dummyString) == 4){
    	if (byte3 < 256 && byte2 < 256 && byte1 < 256 && byte0 < 256) {
         	SM_ID id = (byte3 << 24) + (byte2 << 16) + (byte1 << 8) +  byte0;
         	return id;
      	}
   	}
   	return 0;
}

sm_timestamp sm_get_timestamp() {
    char   timebuffer[20]     = {0};
    struct tm      *tmval     = NULL;
    struct tm       gmtval    = {0};
    struct timespec curtime   = {0};
    sm_timestamp timestamp;
    clock_gettime(CLOCK_REALTIME, &curtime);
    timestamp.seconds = curtime.tv_sec;
    timestamp.nanoseconds = curtime.tv_nsec;
    if((tmval = gmtime_r(&timestamp.seconds, &gmtval)) != NULL)
    {
        strftime(timebuffer, sizeof timebuffer, "%Y-%m-%dT%H:%M:%S", &gmtval);
        snprintf(timestamp.timestring, sizeof timestamp.timestring, "%s.%06ldZ", timebuffer, timestamp.nanoseconds); 
    }
    return timestamp;
}
