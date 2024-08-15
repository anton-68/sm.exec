/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Apply module tests apps
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#ifndef APPLY_APPS_H
#define APPLY_APPS_H

#include "../include/sm.h"

// Console
#define COLLECTOR "CollectorQueue"
#define POOL "pool"

// Router
#define ROUTER "Router"
#define ROUTER_QUEUE "RouterQueue"
#define ROUTER_SM_NAME "RouterSM"
#define FLOW_PROCESSOR_SM_NAME "FlowProcessor"
#define FLOW_PROCESSOR_ARRAY "FlowProcessorArray"

// User Equipment
#define CLIENT "SmartPhone_1"
#define CLIENT_QUEUE "SmartPhone_1_q"
#define CLIENT_SM_NAME "ClientSM"

// 42-Server
#define SERVER "Server"
#define SERVER_QUEUE "ServerQueue"
#define SERVER_SM_NAME "ServerSM"
/*
// SIP Server
#define SIP_SERVER "SIPServer"
#define SIP_SERVER_QUEUE "SIPServerQueue"
#define SIP_SERVER_SM_NAME "SIPServerSM"
#define SIP_SESSION_SM_NAME "SIPSessionSM"
#define SIP_SESSION_ARRAY "SIPSessionArray"
*/
// Home Network Controller
#define CONTROLLER "Controller"
#define CONTROLLER_QUEUE "ControllerQueue"
#define CONTROLLER_SM_NAME "ControllerSM"
#define RULE_ENGINE_SM_NAME "RuleEngineSM"
#define RULE_ENGINE_ARRAY "RuleEngineArray"

// Executors
#define TX0 "tx0"
#define TX1 "tx1"
#define TX2 "tx2"

// Defaults
#define EVENT_SIZE 1024
#define STATE_SIZE 4096
#define QUEUE_SIZE 4096
#define ARRAY_SIZE 12 // this is power of 2 hense for 5 it is 32 elements
#define DEFAULT_REPORTING_FLAG 1;

// Vocabularies
#define PROTO_SPACE 1024
#define COMMAND_SPACE 1024
#define STATUS_SPACE 1024
#define SIP_SPACE 1024
#define NUM_OF_PROTOCOLS 6
#define NUM_OF_COMMANDS 11
#define NUM_OF_STATUS_VALUES 5
#define SIP_NUM_OF_METHODS 7
#define NUM_OF_END_NODES 15

// Apps
int send(sm_event *e, sm_state *s);
int rout(sm_event *e, sm_state *s);
int recv(sm_event *e, sm_state *s);
int orig(sm_event *e, sm_state *s); 
int ssel(sm_event *e, sm_state *s);
int dtmp(sm_event *e, sm_state *s);
int appl(sm_event *e, sm_state *s); // DEPRECATED
//int drop(sm_event *e, sm_state *s); 
//int wait(sm_event *e, sm_state *s); // DEPRECATED
int noap(sm_event *e, sm_state *s);
int serv(sm_event *e, sm_state *s);
//int fltr(sm_event *e, sm_state *s); // DEPRECATED
//int setk(sm_event *e, sm_state *s);
//int setf(sm_event *e, sm_state *s);
//int reqf(sm_event *e, sm_state *s);
//int reqr(sm_event *e, sm_state *s);
//int chck(sm_event *e, sm_state *s);
//int getr(sm_event *e, sm_state *s);
//int addr(sm_event *e, sm_state *s);
//int xtrr(sm_event *e, sm_state *s);
//int askf(sm_event *e, sm_state *s);
//int askr(sm_event *e, sm_state *s);
//int sips(sm_event *e, sm_state *s);
//int sinv(sm_event *e, sm_state *s);
//int s100(sm_event *e, sm_state *s);
//int wrrt(sm_event *e, sm_state *s);

#endif //APPLY_APPS_H
