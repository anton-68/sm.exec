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

#define SM_LOGGER_PRINT_BUFFER 160

typedef enum sm_log_entity
{
	SM_EXEC,
	SM_CORE,
	SM_FSM,
	SM_LUA,
	SM_JSON,
	SM_OAM,
} sm_log_entity;

typedef enum sm_syslog_severity
{
	//  Log level          Description							Usage
	SM_LOG_EMERG,	// system is unusable					For applications usage
	SM_LOG_ALERT,	// action must be taken immediately		For applications usage
	SM_LOG_CRIT,	// critical conditions					For applications usage
	SM_LOG_ERR,		// error conditions						API function unable to complete
	SM_LOG_WARNING, // warning conditions					Potentially amendable error
	SM_LOG_NOTICE,	// normal but significant condition		Resources utilization, amended error 
	SM_LOG_INFO,	// informational						System objects lifecycle 
	SM_LOG_DEBUG,	// excessive debug information			Operations on events
} sm_syslog_severity;

#ifdef SM_LOG
#define SM_REPORT_MESSAGE(severity, message) \
	report(SM_EXEC, severity, __LINE__, __FILE__, __func__, (message), "-")
#define SM_REPORT_WARNING(...)                                                                        \
	{                                                                                                \
		char __sm__message__[SM_LOGGER_PRINT_BUFFER];                                                \
		sprintf(__sm__message__, __VA_ARGS__);                                                       \
		if (strlen(__sm__message__) > SM_LOGGER_PRINT_BUFFER)                                        \
		{                                                                                            \
			report(SM_OAM, SM_LOG_ERR, __LINE__, __FILE__, __func__, "SYSLOG buffer exceeded", "-"); \
		}                                                                                            \
		report(SM_EXEC, SM_LOG_WARNING, __LINE__, __FILE__, __func__, (__sm__message__), "-");         \
	}
#define SM_REPORT_CODE(severity, code) \
	report(SM_EXEC, severity, __LINE__, __FILE__, __func__, "-", strerror(code))
#define SM_SYSLOG(entity, severity, description, code) \
	report((entity), (severity), __LINE__, __FILE__, __func__, (description), strerror(code))
#else
#define SM_REPORT_MESSAGE(severity, message)
#define SM_REPORT_CODE(severity, code)
#define SM_SYSLOG(entity, severity, description, cause)
#endif

#ifdef SM_DEBUG
#define SM_DEBUG_MESSAGE(...)                                                                  		\
	{                                                                                          		\
		char __sm__message__[SM_LOGGER_PRINT_BUFFER];                                          		\
		sprintf(__sm__message__, __VA_ARGS__);                                                 		\
		if (strlen(__sm__message__) > SM_LOGGER_PRINT_BUFFER)                                 		\
		{																							\
			report(SM_OAM, SM_LOG_ERR, __LINE__, __FILE__, __func__, "SYSLOG buffer exceeded", "-");\
		}																							\
		report(SM_EXEC, SM_LOG_DEBUG, __LINE__, __FILE__, __func__, (__sm__message__), "-");   		\
	}
#define SM_DEBUG_CODE(code) \
	report(SM_EXEC, SM_LOG_DEBUG, __LINE__, __FILE__, __func__, "-", strerror(code))
#else
#define SM_DEBUG_MESSAGE(...)
#define SM_DEBUG_CODE(code)
#endif

int report(sm_log_entity  entity,		// {CORE, FSM, ...} => sm_log_entity_name
		             int  severity,		// 0-7 (rfc5424)
		             int  line, 		// __LINE__
		      const char *file, 		// __FILE__
		      const char *function,		// __func__
		      const char *description,	// aka symptoms
		  	  const char *cause);		// aka (diff) diagnosys

#endif // SM_LOGGER_H
