/* SM.EXEC
   Logger functions
   anton.bondarenko@gmail.com */

#ifndef LOGGER_H
#define LOGGER_H

#include <syslog.h>

#define SM_LOG			//undef this to switch off logging
#define SM_CONSOLE_LOG 	//undef this to switch off console logging

//#define SM_DEBUG
//#define SM_RFC5424
#define SM_LOG_PERROR 0 //LOG_PERROR
#define SM_SYSLOG_VERSION "1"
#define SM_SYSLOG_MSGID "-"
#define SM_SYSLOG_ENTID "99999"


typedef enum sm_log_entity {
	SM_CORE, 
	SM_FSM, 
	SM_LUA
} sm_log_entity;

typedef enum sm_syslog_severity {
	SM_LOG_EMERG,	// system is unusable
	SM_LOG_ALERT,	// action must be taken immediately
	SM_LOG_CRIT,	// critical conditions
	SM_LOG_ERR,		// error conditions
	SM_LOG_WARNING,	// warning conditions
	SM_LOG_NOTICE,	// normal but significant condition
	SM_LOG_INFO,	// informational
	SM_LOG_DEBUG,	
} sm_syslog_severity;

enum sm_severity {
    ERROR,
    EVENT
};

#ifdef SM_LOG
#define REPORT(severity, message) report(CORE, sm_severity_old_type(severity), __LINE__, __FILE__, __func__, (message), "-") // Deprecated
#define SM_SYSLOG(entity, severity, description) report((entity), (severity), errno, __LINE__, __FILE__, __func__, (description))
#else
#define REPORT(severity, message)
#define SM_SYSLOG(entity, severity, description)
#endif

int report(sm_log_entity  entity,		// SM_{CORE, FSM, LUA} => sm_log_entity_name
		             int  severity,		// 0-7 (rfc5424)
		   			 int  error,		// errno
		             int  line, 		// __LINE__
		     const char  *file, 		// __FILE__
		     const char  *function,		// __func__
		     const char  *description);

#ifdef SM_LOG
int sm_severity_old_type(int event_type);
#endif

#endif //LOGGER_H
