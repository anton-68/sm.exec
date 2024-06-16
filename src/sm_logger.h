/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Logger functions
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#ifndef SM_LOGGER_H
#define SM_LOGGER_H

#include <syslog.h>
#include <errno.h>
#include "sm_sys.h" // timestamp

#define SM_LOG

//#define SM_SYSLOG_STRING_LEN 4096

typedef enum sm_log_entity
{
	SM_EXEC,
	SM_CORE,
	SM_FSM,
	SM_LUA,
	SM_JSON,
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

#ifdef SM_LOG
#define SM_REPORT(severity, message) report(SM_EXEC, severity, __LINE__, __FILE__, __func__, (message), "-") // Deprecated
#define SM_SYSLOG(entity, severity, description, cause) report((entity), (severity), __LINE__, __FILE__, __func__, (description), (cause))
#else
#define REPORT(severity, message)
#define SM_SYSLOG(entity, severity, description, cause)
#endif

int report(sm_log_entity  entity,		// {CORE, FSM, ...} => sm_log_entity_name
		             int  severity,		// 0-7 (rfc5424)
		             int  line, 		// __LINE__
		     const char  *file, 		// __FILE__
		     const char  *function,		// __func__
		     const char  *description,	// aka symptoms
		  	 const char  *cause);		// aka (diff) diagnosys

#endif // SM_LOGGER_H
