/* SM.EXEC
   Logger functions
   anton.bondarenko@gmail.com */

//#include <stdio.h>
#include <errno.h>
#include <unistd.h>			// gethostname()
#include <string.h>			// strerror()
#include <stdlib.h>			// EXIT_SUCCESS
#include "../src/sm_sys.h"
#include "logger.h"

#ifdef SM_LOG
int sm_severity_old_type(int event_type) {
	int severity_log[] = {LOG_ERR, LOG_INFO};	
	return severity_log[event_type];
}
#endif

/* report */
/*
int report(severity_t severity, const char *message, const char *file, 
		   int line, const char *function) {
    if(severity == ERROR) 
        fprintf (stderr, "[ SEVERITY ] ERROR\n"); 
    else
		fprintf (stderr, "[ SEVERITY ] EVENT\n");
    fprintf (stderr, "[    errno ] %s\n", strerror(errno));
	char tid[32];
	get_tid_str(tid);
	fprintf (stderr, "[   THREAD ] %s\n", tid);
    fprintf (stderr, "[       IN ] %s\n", message);
    fprintf (stderr, "[     FILE ] %s\n", file);
    fprintf (stderr, "[     LINE ] %i\n", line - TL_REPORT_LN_ADJUSTMENT);
    fprintf (stderr, "[ FUNCTION ] %s\n\n", function);  
    return EXIT_SUCCESS;
}
*/

int report(sm_log_entity  entity,		// {CORE, FSM} => sm_log_entity_name
		             int  severity,		// 0-7 (rfc5424)
		             int  line, 		// __LINE__
		     const char  *file, 		// __FILE__
		     const char  *function,		// __func__
		     const char  *description,
		  	 const char  *cause){
	//char message[SM_SYSLOG_STRING_LEN] = "\0";	
	
	char* sm_log_entity_name[] = {
		"SM_EXEC.CORE",
		"SM_EXEC.FSM",
		"SM_EXEC.LUA"
	};

	char* prioritynames[] = { 
		"emerg",
    	"alert",
    	"crit",
    	"err",
    	"warning",
    	"notice",
    	"info",
    	"debug"
    	"none"
	};
	
	char hostname[256] = "\0";
	gethostname(hostname, 256);				 
	sm_timestamp timestamp = sm_get_timestamp();
	char procid[64] = "\0";
	get_tid_str(procid);
	
#ifdef SM_DEBUG // ---------------------------------	
//	syslog(severity % 8 | LOG_USER, "<%d>%s %s %s %s %s %s {\"severity\": \"%s\", \"problem\":\"%s\", \"cause\":\"%s\", \"file\":\"%s\", \"func\":\"%s\", \"line\":\"%d\", \"errno\":\"%s\" }",
//	syslog(severity % 8 | LOG_USER, "<%d>%s %s %s %s %s %s [severity:%s, problem:%s, cause:%s, file:%s, func:%s, line:%d, errno:%s]",
	syslog(severity % 8 | LOG_USER, "%s %s %s %s, \nDesc.: %s, \nCause: %s, \nFile : %s, Func: %s, Line: %d, \nerrno: %s",
		   //severity % 8 | LOG_USER,
		   //SM_SYSLOG_VERSION,
		   timestamp.timestring,
		   //hostname,
		   prioritynames[severity],
		   sm_log_entity_name[entity],
		   procid,
		   //SM_SYSLOG_MSGID,
		   description,
		   cause,
		   file,
		   function,
		   line,
		   strerror(errno));
#else // -------------------------------------------
//	syslog(severity % 8 | LOG_USER, "<%d>%s %s %s %s %s %s {\"severity\": \"%s\", \"problem\":\"%s\", \"cause\":\"%s\", \"errno\":\"%s\" }",
//	syslog(severity % 8 | LOG_USER, "<%d>%s %s %s %s %s %s [severity:%s, problem:%s, cause:%s, errno:%s]",		   
	syslog(severity % 8 | LOG_USER, "%s %s %s %s, \nDesc.: %s, \nCause: %s, \nerrno: %s",
		   //severity % 8 | LOG_USER,
		   //SM_SYSLOG_VERSION,
		   timestamp.timestring,
		   //hostname,
		   prioritynames[severity],
		   sm_log_entity_name[entity],
		   procid,
		   //SM_SYSLOG_MSGID,
		   description,
		   cause,
		   strerror(errno));	
#endif // ------------------------------------------
	
	return EXIT_SUCCESS;
}
	
	