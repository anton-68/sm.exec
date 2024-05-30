function pinv(obj)
	mt = getmetatable(obj)
	inv = mt['inventory']
	for k, v in pairs(inv) do
		print(k, v)
	end
end

sm = require("sm")

e1 = sm.new_event(1,16)
e2 = sm.new_event(2,16)

print(e1..e2)

e3 = e1:next()

e4 = e1:next()

print(e3 == e4)

print(pinv(e1))
