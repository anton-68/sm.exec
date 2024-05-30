/* SM.EXEC
   Logger functions
   anton.bondarenko@gmail.com */

#include <unistd.h>	
#include <string.h>	
#include <stdlib.h>	
#include "sm_sys.h"
#include "sm_logger.h"

// DEPRECTATED
int sm_severity_old_type(int event_type) {
	int severity_log[] = {SM_LOG_ERR, SM_LOG_INFO};	
	return severity_log[event_type];
}
// END OF DEPRECTATED

int sm_logger_init() {
	openlog(SM_NAME, LOG_PID | SM_LOG_TERM, LOG_USER);	
#ifdef SM_DEBUG
	return setlogmask(LOG_UPTO(LOG_DEBUG));
#else
	return setlogmask(LOG_UPTO(LOG_ERR));
#endif	
}

int sm_logger_report(sm_log_entity entity, int severity, int  error, int line, 
					 const char *file, const char  *function, const char  *description){	

	char hostname[256] = "\0";
	gethostname(hostname, 256);	
#ifdef SM_RFC5424	
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
	sm_timestamp timestamp = sm_get_timestamp();
#endif	
	char procid[64] = "\0";
	get_tid_str(procid);
#ifdef SM_DEBUG
#ifdef SM_RFC5424
	syslog(severity /*% 8 | LOG_USER*/, "%s %s %s %s %s %s Desc.: %s errno: %s File : %s Func: %s Line: %d",
	   timestamp.timestring,
	   hostname,
	   sm_log_entity_name[entity],
	   procid,
	   SM_SYSLOG_MSGID,
	   "-", // Structured data
	   description,
	   strerror(errno),
	   file,
	   function,
	   line);
#else // ~SM_RFC5424
	syslog(severity % 8 | LOG_USER, "%s, errno: %s, File : %s, Func: %s, Line: %d",
	   description,
	   strerror(errno),
	   file,
	   function,
	   line);
#endif // SM_RFC5424
#else // ~SM_DEBUG
#ifdef SM_RFC5424
	syslog(severity % 8 | LOG_USER, "%s %s %s %s %s %s Desc.: %s errno: %s",
		   timestamp.timestring,
		   hostname,
		   sm_log_entity_name[entity],
		   procid,
		   SM_SYSLOG_MSGID,
		   "-", // Structured data
		   description,
		   strerror(errno));
#else // ~SM_RFC5424
	syslog(severity % 8 | LOG_USER, "%s, errno: %s",
		   description,
		   strerror(errno));
#endif // SM_RFC5424	
#endif // SM_DEBUG
	closelog();
	return EXIT_SUCCESS;
}
	
	