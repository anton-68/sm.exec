sm = require("sm")

e = {}
pq = sm.new_pqueue(12, false)
for i=1, 3 do
	e[i] = sm.new_event(1, 16)
	e[i]:setpriority(0, -i)
	pq:enqueue(e[i])
end
for i=1, 3 do
	e[i] = sm.new_event(1, 16)
	e[i]:setpriority(1, i)
	pq:enqueue(e[i])
end

print(pq)

while(#pq > 0) do
	print(pq:dequeue())
end	

print(pq)
