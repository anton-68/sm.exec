-- SM TrafficSM7
-- anton.bondarenko@gmail.com
--[[----------------------------------------------------------------- 

Guide

Spatial states (Golly):
# States:
#  0 - Free / dead
#  1 - Heading to North
#  2 - Heading to East
#  3 - Heading to South
#  4 - Heading to West
#  5 - Still
#  6 - Fense or wall
#  7 - Still (reached goal)
#  8 - Blocked (unhappy)
#  9 - Behind the wall
# 10 - [reserved]
# 11 - Heading to North (stealth)
# 12 - Heading to East (stealth)
# 13 - Heading to South (stealth)
# 14 - Heading to West (stealth)
# 15 - [reserved]

Lua (main process) data:
========================

SM.EXEC Exec data block:
------------------------
App id = "10.255.255.128" (Pattern)
// Pattern:
Grid {
    int rows                        -- height 
    int cols                        -- width
    int rows x sizeof(int *)        -- rows
    int rows x cols x sizeof(int)    -- matrix
}
// Grid values: "0" - free | "6" - fense

SM.EXEC State data block:
-------------------------
App id = "10.255.255.129" (Heat-map)
// Heat-map:
Grid { 
    int rows                        -- height 
    int cols                        -- width
    int rows x sizeof(int *)        -- rows
    int rows x cols x sizeof(int)    -- matrix
}
// Grid values: int (distance to the goal)
Goal {int X, int Y}

Bot Control Message Event data block:
-------------------------------------
App id = "10.255.255.130"
event->id = SM_TRSIM_BCME
Position {int X, int Y}
int Heading
Neighbourhood {int N, int NE, int E, int SE, int S, int SW, int W, int NW}
Goal {int X, int Y}

Other event types: no data is conveyed
--------------------------------------

--]]-----------------------------------------------------------------

-- Load libs
g = golly()
gp = require "gplus"
package.cpath = package.cpath..";"..g.getdir("app").."Scripts/Lua/TrafficSM/?.so"
g.show(package.cpath)

sm = require("sm")
sm_exec = sm.exec_init(16*1024*1024)

-- math.randomseed(os.time())

-- position calculating
function sm_pos(i, s)
    return i-s//2
end

-- next position
function sm_next(i, w, h)
    return (i + 1) % ((w + h)//4)
end

-- Load Golly pattern (and rule)
g.open("../../../Patterns/TrafficSM/TrafficSM7.mc")


-- Read map into exec data block
local execData = _G.sm_exec:get_DCB("10.255.255.128")
local h = g.getheight()
execData:write_int(h)
local w = g.getwidth()
execData:write_int(w)
local m = execData:allocate_2d_int_array(h, w)
for i = 0, h-1 do
    for j = 0, w-1 do
        execData:int2d_set(m, i, j, g.getcell(sm_pos(j, w), sm_pos(i, h))) 
    end
end

-- Create shuffled array of goals    
   
goals = {}
for i = 0 , (w + h)//4 - 1 do
    goals[i] = i
end
    
function sm_swap(a, b)
    c = a
    a = b
    b = c
    return
end
        
function sm_shuffle(a)
    for i = 0, #a - 2 do
        sm_swap(a[i], a[i + math.random() % (#a - i)])
    end                
    return
end
    
sm_shuffle(goals)
    
-- Allocate bots
-- allocate N bot descriptor within Lua (pos = (X, Y))
-- calculate goal positions (next to bot start positions unshuffled)

bots = {}
goal_pos = {}

for i = 0, 47 do
    bots[i] = {}
    bots[i].X = sm_pos(3 + i * 8, w)
    bots[i].Y = sm_pos(1, h)
    bots[i].Goal = goals[i]
    bots[i].Heading = 5
    g.setcell(bots[i].X, bots[i].Y, bots[i].Heading)
    goal_pos[i]= {}
    goal_pos[i].X = bots[i].X + 1
    goal_pos[i].Y = bots[i].Y
end

for i = 0, 23 do
    k = i + 24
    bots[k] = {}
    bots[k].X = sm_pos(382, w)
    bots[k].Y = sm_pos(3 + i * 8, h)
    bots[k].Goal = goals[k]
    bots[k].Heading = 5
    g.setcell(bots[k].X, bots[k].Y, bots[k].Heading)
    goal_pos[k]= {}
    goal_pos[k].X = bots[k].X
    goal_pos[k].Y = bots[k].Y + 1
end

for i = 47, 0, -1 do
    k = i + 72
    bots[k] = {}
    bots[k].X = sm_pos(4 + i * 8, w)
    bots[k].Y = sm_pos(190, h)
    bots[k].Goal = goals[k]
    bots[k].Heading = 5
    g.setcell(bots[k].X, bots[k].Y, bots[k].Heading)
    goal_pos[k]= {}
    goal_pos[k].X = bots[k].X - 1
    goal_pos[k].Y = bots[k].Y
end

for i = 23, 0, -1 do
    k = i + 120
    bots[k] = {}
    bots[k].X = sm_pos(1, w)
    bots[k].Y = sm_pos(4 + i * 8, h)
    bots[k].Goal = goals[k]
    bots[k].Heading = 5
    g.setcell(bots[k].X, bots[k].Y, bots[k].Heading)
    goal_pos[k]= {}
    goal_pos[k].X = bots[k].X
    goal_pos[k].Y = bots[k].Y - 1
end

g.show('...')
        
-- Load apps 
lib = sm.loadlib("sm_test_apps.so")
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

planning_fsm = smnew_fsm(planning_fsm_str, "mealy")
planning_fsm:dir_set("planning_fsm")

-- Create BCM queue
q = sm.new_queue2()
q:dir_set("main_queue")

-- Create, populate and enqueue BCM events

-- SM.EXEC Event types:
SM_TRSIM_OMEGA = 0
SM_TRSIM_BCME = 1
SM_TRSIM_STEP1 = 2
SM_TRSIM_STEP2 = 3
SM_TRSIM_STEP2 = 4
SM_TRSIM_PLAN = 5

do_step1 = sm.new_event(SM_TRSIM_STEP1, 0)
do_step2 = sm.new_event(SM_TRSIM_STEP2, 0)
do_step3 = sm.new_event(SM_TRSIM_STEP3, 0)
do_plan = sm.new_event(SM_TRSIM_plan, 0)

-- Another helper function
function write_frame(db)
    db:rewind()
    _x = db:read_int()
    _y = db:read_int()
    db:write_int(g.getcell(_x, _y))
    db:write_int(g.getcell(_x, _y + 1))       -- N
    db:write_int(g.getcell(_x + 1, _y + 1))   -- NE
    db:write_int(g.getcell(_x + 1, _y))       -- E
    db:write_int(g.getcell(_x + 1, _y - 1))   -- SE
    db:write_int(g.getcell(_x, _y - 1))       -- S 
    db:write_int(g.getcell(_x - 1, _y - 1))   -- SW 
    db:write_int(g.getcell(_x - 1, _y))       -- W
    db:write_int(g.getcell(_x - 1, _y + 1))   -- NW 
    return
end

-- int Heading
-- Position {int X, int Y}
-- Neighbourhood {int N, int NE, int E, int SE, int S, int SW, int W, int NW}
-- Goal {int X, int Y}
for i,b in ipairs(bots) do
    e = sm.new_event(128)
    e:set_id(SM_TRSIM_BCME)
    e_data = e:get_DCB("10.255.255.130")
    e_data:write_int(b.X)
    e_data:write_int(b.Y)
    write_frame(e_data)
    e_data:write_int(goal_pos[goals[i].X])
    e_data:write_int(goal_pos[goals[i].Y])
    q:enqueue(e)
end
q:enqueue(do_step1)

-- Create BrushFire pool and queue - used by native apps
-- Do we need to hide it within the app??
-- One per Tx will be required
bf_pool = sm.new_queue(w * h // 2, 3 * sizeof(int), false)    
bf_pool:dir_set("bf_pool")
bf_queue = sm.new_queue(0, 0, false)    
bf_queue:dir_set("bf_queue")

-- Create TX / default state
tx = sm:new_tx("default_fsm", "main_queue", true, w * h * 4 + h * 8 + 256)
s = tx:get_state()

-- Create state array
states = sm.new_array((w + h)/4, 1024*1024);
state:dir_set("states")

-- Main cycle
while true do
    -- Quarter-step 1: Set heading and decide
    repeat
        e = q:dequeue()
        e_data = e:get_DCB("10.255.255.130")
        g.setcell(e_data:read_int(), e_data:read_int(), e_data:read_int())
        q:enqueue(e)
    until(e:get_id() ~= SM_TRSIM_STEP1)
    q:enqueue(do_step2)    
    g.step()
    -- Quarter-step 2: Read move and proceed
    repeat
        e = q:dequeue()
        e_data = e:get_DCB("10.255.255.130")
        _x = e_data:read_int()
        _y = e_data:read_int()
        _h = e_data:read_int()
        if _h == 11 then _y = _y - 1 end -- Heading to North (stealth)
        if _h == 12 then _x = _x + 1 end -- Heading to East (stealth)
        if _h == 13 then _y = _y + 1 end -- Heading to South (stealth)
        if _h == 14 then _x = _x - 1 end -- Heading to West (stealth) 
        e_data:rewind()
        e_data:write_int(_x)
        e_data:write_int(_y)
        q:enqueue(e)
    until(e:get_id() ~= SM_TRSIM_STEP2)
    q:enqueue(do_step3)    
    g.step()
    -- Quarter-step 3: Read frame
    repeat
        e = q:dequeue()
        e_data = e:get_DCB("10.255.255.130")
        write_frame(e_data)
        q:enqueue(e)
    until(e:get_id() ~= SM_TRSIM_STEP3)
    q:enqueue(do_plan)
    -- Quarter-step 4: Plan
    repeat
        e = q:dequeue()
        s:apply_event(e)
    until(e:get_id() ~= SM_TRSIM_PLAN)
    q:enqueue(do_step1)    
end

-- That's all, folks!