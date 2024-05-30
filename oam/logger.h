/* SM.EXEC
   Logger functions
   anton.bondarenko@gmail.com */

#ifndef LOGGER_H
#define LOGGER_H

#include <syslog.h>

typedef enum sm_log_entity {CORE, FSM, LUA} sm_log_entity;

#define SM_LOG			//undef this to switch off logging
#define SM_CONSOLE_LOG 	//undef this to switch off console logging

#define SM_DEBUG
#define SM_SYSLOG_VERSION "1"
#define SM_SYSLOG_MSGID "-"
#define SM_SYSLOG_ENTID "99999"

#ifdef SM_LOG
#define REPORT(severity, message) report(CORE, sm_severity_old_type(severity), __LINE__, __FILE__, __func__, (message), "-") // Deprecated
#define SM_SYSLOG(entity, severity, description, cause) report((entity), (severity), __LINE__, __FILE__, __func__, (description), (cause))
#else
#define REPORT(severity, message)
#define SM_SYSLOG(entity, severity, description, cause)
#endif

/*
LOG_EMERG	0	// system is unusable
LOG_ALERT	1	// action must be taken immediately
LOG_CRIT	2	// critical conditions
LOG_ERR		3	// error conditions
LOG_WARNING	4	// warning conditions
LOG_NOTICE	5	// normal but significant condition
LOG_INFO	6	// informational
LOG_DEBUG	7	// debug-level messages
*/

enum sm_severity {
    ERROR,
    EVENT
};

int report(sm_log_entity  entity,		// {CORE, FSM} => sm_log_entity_name
		             int  severity,		// 0-7 (rfc5424)
		             int  line, 		// __LINE__
		     const char  *file, 		// __FILE__
		     const char  *function,		// __func__
		     const char  *description,
		  	 const char  *cause);

#ifdef SM_LOG
int sm_severity_old_type(int event_type);
#endif

#endif //LOGGER_H
