function list_mt(obj)
	mt = getmetatable(obj)
	for k, v in pairs(mt) do
		print(k, v)
	end
end


sm = require("sm")
sm_exec = sm.exec_init(16*1024*1024)

local lib = sm.loadlib("sm_robot_demo_apps.so")
noap = sm.lookup(lib, "trsim_noap")
noap:dir_set("noap")
check_neighbours = sm.lookup(lib, "trsim_check_neighbours")
check_neighbours:dir_set("check_neighbours")
switch_fsm = sm.lookup(lib, "trsim_switch_fsm")
switch_fsm:dir_set("switch_fsm")
plan = sm.lookup(lib, "trsim_plan")
plan:dir_set("plan")

-- Create FSM-s
default_fsm_str = [[{
    "nodes": [
        {"id": 1,"type": "initial"},
        {"id": 2,"type": "joint"},
        {"id": 3,"type": "final"}
    ],
    "events": [
        {"id": 1,"type": "regular"},
        {"id": 2,"type": "regular"},
        {"id": 0,"type": "default"}
    ],
    "transitions": [
        {"state": 1,"event": 0,"new_state": 2,"action": "check_neighbours"},
        {"state": 2,"event": 0,"new_state": 3,"action": "noap"},
        {"state": 2,"event": 2,"new_state": 2,"action": "switch_fsm"}
    ]
}]]
-- Transtion to completion was on 1 not by 0!

default_fsm = sm.new_fsm(default_fsm_str, "mealy")
default_fsm:dir_set("default_fsm")

planning_fsm_str = [[{
    "nodes": [
        {"id": 1,"type": "initial"},
        {"id": 2,"type": "final"}
    ],
    "events": [
        {"id": 1,"type": "regular"}
        {"id": 0,"type": "default"}
    ],
    "transitions": [
        {"state": 1,"event": 0,"new_state": 2,"action": "plan"}
    ]
}]]

planning_fsm = sm.new_fsm(planning_fsm_str, "mealy")
planning_fsm:dir_set("planning_fsm")

-- Create BCM queue
q = sm.new_queue2()
q:dir_set("main_queue")




-- Create TX / default state
tx = sm.new_tx("main_queue", "default_fsm", true, 256, 256)
-- s = tx:get_state()

-- Create state array
-- states = sm.new_array((w + h)/4, 1024*1024);
-- state:dir_set("states")
