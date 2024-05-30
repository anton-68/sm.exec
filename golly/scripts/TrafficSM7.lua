-- SM TrafficSM7
-- anton.bondarenko@gmail.com
--[[----------------------------------------------------------------- 

Guide

Used SM App ID-s:
10.255.255.128 Move		Move bot to position X, Y
10.255.255.129 Set		Set bot state to S
10.255.255.130 Plan		Process queued Move events
10.255.255.131 Step		Make another spatial iteration

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
#  9 - [reserved]
# 10 - [reserved]
# 11 - Heading to North (stealth)
# 12 - Heading to East (stealth)
# 13 - Heading to South (stealth)
# 14 - Heading to West (stealth)
# 15 - [reserved]

Lua (main process) data:
Bot descriptor bidirectional mapping:
   Id (Integer) <-> Position (Integer, Integer)	

SM.EXEC Exec data block:
App id = "10.255.255.130" (Plan)
Spatial grid (pattern):
int rows, cols					-- size
int rows x sizeof(int *)		-- rows
int rows x cols x sizeof(int)	-- matrix
"0" - free						-- values
"6" - fense						-- values

SM.EXEC State data block:
App id = "10.255.255.130" (Plan)
int BrushfireFlag = {			-- flag
	-1 = goal updated | 
  	 0 = planning in progress | 
	 1 = ready }
int X, Y						-- position
int X, Y						-- goal
int rows, cols					-- brushfire map size
int rows x sizeof(int *)		-- rows
int rows x cols x sizeof(int)	-- brushfire matrix

SM.EXEC Events:

Move event data block:
App id = "10.255.255.128" (Move)
int X, Y						-- target position
int NW, NE, SW, SE				-- neighbourhood

Set event data block:
App id = "10.255.255.129" (Set)
int								-- new spatial state			

Plan event data block:
App id = "10.255.255.130" (Plan)

Step event data block:
App id = "10.255.255.131" (Step)
--]]-----------------------------------------------------------------

-- Load libs
g = golly()
gp = require "gplus"
package.cpath = package.cpath..";"..g.getdir("app").."Scripts/Lua/TrafficSM/?.so"
g.show(package.cpath)

sm = require "sm"

-- Load Golly pattern (and rule)
g.open("../../../Patterns/TrafficSM/TrafficSM7.mc")

-- Read map into exec data block
local execData = _G.sm_exec:get_DCB("10.255.255.130")
local h = g.getheight()
execData:write_int(h)
local w = g.getwidth()
execData:write_int(w)
DCB_allocate_2d_int_array(h, w)
for i = 0, h-1 do
	for j = 0, w-1 do
		execData:int2d_set(i, j, g,getcell(i-h//2, j-w//2)) 
	end
end

-- Allocate bots
-- allocate N bot descriptor within Lua (pos = (X, Y))

-- Load bot apps and fsm

-- Create & register bots
-- for 1..N:
--		generate bot
--		set goal
--		place bot on pattern (state = steal)
--		init bot state(pos, goal)

-- Init thread-workers:
-- for 1..K:
--		create TX
--		run X

-- Main cycle
-- 		step
--		enqueue events
--		step
--		trigger TX
