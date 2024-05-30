/* SM.EXEC
   Logger functions
   anton.bondarenko@gmail.com */

#ifndef LOGGER_H
#define LOGGER_H

#include <syslog.h>

#ifdef SM_DEBUG
    #define SM_LOG_TERM LOG_PERROR | LOG_CONS
    #define SM_LOG_TERM LOG_PERROR | LOG_CONS
#else
    #define SM_LOG_PERROR 0x0
#endif
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

// DEPRECTATED
int sm_severity_old_type(int event_type);
#define REPORT(severity, message) sm_logger_report(SM_CORE, sm_severity_old_type(severity), __LINE__, __FILE__, __func__, (message), "-") 
// END OF DEPRECTATED 

#define SM_LOG(entity, severity, description) sm_logger_report((entity), (severity), errno, __LINE__, __FILE__, __func__, (description))

int sm_logger_init();

int sm_logger_report(sm_log_entity  entity,		// SM_{CORE, FSM, LUA} => sm_log_entity_name
		             		   	int severity,   // 0-7 (rfc5424)
		   			 		   	int error,		// errno
		             		   	int line, 		// __LINE__
		     			const char* file, 		// __FILE__
		     			const char* function,	// __func__
		     			const char* description);

#endif //LOGGER_H
