/* SM.EXEC
   Core functions
   anton.bondarenko@gmail.com */
   
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>

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
    char   timebuffer[32]     = {0};
    struct timeval  tv        = {0};
    struct tm      *tmval     = NULL;
    struct tm       gmtval    = {0};
    struct timespec curtime   = {0};
    sm_timestamp timestamp;
    int i = 0;
    clock_gettime(CLOCK_REALTIME, &curtime);
    timestamp.seconds      = curtime.tv_sec;
    timestamp.microseconds = round(curtime.tv_nsec/1.0e3);
    if((tmval = gmtime_r(&timestamp.seconds, &gmtval)) != NULL)
    {
        strftime(timebuffer, sizeof timebuffer, "%Y-%m-%dT%H:%M:%S", &gmtval);
        snprintf(timestamp.timestring, sizeof timestamp.timestring, "%s.%06ldZ", timebuffer, timestamp.microseconds); 
    }
    return timestamp;
}
