sm = require("sm")

function pinv(obj)
	mt = getmetatable(obj)
	inv = mt['inventory']
	for k, v in pairs(inv) do
		print(k, v)
	end
end

e1 = sm.new_event(1,16)
e2 = sm.new_event(2,16)
print(e1..e2)
print(e1:next())
