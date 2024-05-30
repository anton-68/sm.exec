/* SM.EXEC
   Logger functions
   anton.bondarenko@gmail.com */

//#include <stdio.h>
#include <errno.h>
#include <unistd.h>			// gethostname()
#include <string.h>			// strerror()
#include <stdlib.h>			// EXIT_SUCCESS
#include "sm_sys.h"
#include "sm_logger.h"

#ifdef SM_LOG
int sm_severity_old_type(int event_type) {
	int severity_log[] = {LOG_ERR, LOG_INFO};	
	return severity_log[event_type];
}
#endif

/* report */
int report(sm_log_entity  entity,		// SM_{CORE, FSM, LUA} => sm_log_entity_name
		             int  severity,		// 0-7 (rfc5424)
		   			 int  error,		// errno
		             int  line, 		// __LINE__
		     const char  *file, 		// __FILE__
		     const char  *function,		// __func__
		     const char  *description){	
	
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

	openlog("SM.EXEC", LOG_PID | SM_LOG_PERROR, LOG_USER);	

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
	
	