/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Logger functions
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#include "sm_logger.h"

int report(sm_log_entity  entity,		// {CORE, FSM} => sm_log_entity_name
		             int  severity,		// 0-7 (rfc5424)
		             int  line, 		// __LINE__
		     const char  *file, 		// __FILE__
		     const char  *function,		// __func__
		     const char  *description,
		  	 const char  *cause){
	//char message[SM_SYSLOG_STRING_LEN] = "\0";	
	
	char* sm_log_entity_name[] = {
		"SM_EXEC",
		"SM_EXEC.CORE",
		"SM_EXEC.FSM",
		"SM_EXEC.LUA",
		"SM_EXEC.JSON",
		"SM_EXEC.OAM"
	};

	char* prioritynames[] = { 
		"emerg",
    	"alert",
    	"crit",
    	"err",
    	"warning",
    	"notice",
    	"info",
    	"debug",
    	"none"
	};
	
	char hostname[256] = "\0";
	gethostname(hostname, 256);				 
	sm_timestamp timestamp = sm_get_timestamp();
	char procid[64] = "\0";
	get_tid_str(procid);
	
	syslog(severity % 8 | LOG_USER, "Time: %s, Priority: %s, Entity: %s, Process: %s, Desc.: %s, Cause: %s, File : %s, Func: %s, Line: %d, errno: %s",
		   timestamp.timestring,
		   prioritynames[severity],
		   sm_log_entity_name[entity],
		   procid,
		   description,
		   cause,
		   file,
		   function,
		   line,
		   strerror(errno));
	
	return EXIT_SUCCESS;
}
	
	
