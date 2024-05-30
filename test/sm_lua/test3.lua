require "sm"
local h = 17
local w = 10
db = sm_exec:get_DCB('10.0.0.1')
db:write_int(12)
db:write_int(14)
m = db:allocate_2d_int_array(h, w)
for i = 0, h-1 do
	print("i = ", i)
	for j = 0, w-1 do
		print("j = ", j)
		print("(i * h + j) = ", (i * h + j))
		print("m = ", m)
		db:int2d_set(m, i, j, i * h + j)
		print("\nm[i][j] = ", db:int2d_get(m, i, j))
	end
end
