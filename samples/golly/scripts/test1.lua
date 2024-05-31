require "sm"
local h = 12
local w = 14
db = sm_exec:get_DCB('10.0.0.1')
db:write_int(12)
db:write_int(14)
m = db:allocate_2d_int_array(h, w)
for i = 0, h-1 do
	for j = 0, w-1 do
		execData:int2d_set(m, i, j, i * h + j) 
	end
end
