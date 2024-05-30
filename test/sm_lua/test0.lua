function readAll(file)
    local f = assert(io.open(file, "rb"))
    local content = f:read("*all")
    f:close()
    return content
end

sm = require("sm")

at = sm.new_apptab()

lib = sm.loadlib("/home/anton/lab/sm.exec/v6/test/sm_app/sm_test_apps.so")

print("\n")
print("Applications:\n")
a1 = sm.lookup(lib, "sm_test_app1")
print(a1)
a2 = sm.lookup(lib, "sm_test_app2")
print(a2)
a3 = sm.lookup(lib, "sm_test_app3")
print(a3)
noop = sm.lookup(lib, "sm_nope")
print(noop)
print("\n")

at:set(a1, "APP1")
at:set(a2, "APP2")
at:set(a3, "APP3")
at:set(noop, "NOAP")

fsm_str = readAll("test_sm_fsm_0.json")
fsm = sm.new_fsm(fsm_str, at, "moore")
print("FSM:\n")
print(fsm)

ft = sm.new_fsmtab()
ft:set(fsm, "fsm0")
print(ft)

s = sm.new_state(fsm, 16)
print("State:\n")
print(s)

print("Events:\n")
e1 = sm.new_event(1, 16)
e1:set("event1")
print(e1)
e2 = sm.new_event(2, 16)
e2:set("event2")
print(e2)
e3 = sm.new_event(3, 16)
e3:set("event3")
print(e3)
e4 = sm.new_event(4, 16)
e4:set("event4")
print(e4)
print("\n")

print("Trace:\n")

print(s:id())
s:apply(e1)
print(s:id())
s:apply(e1)
print(s:id())
s:apply(e2)
print(s:id())
s:apply(e4)
print(s:id())
s:apply(e1)
print(s:id())
s:apply(e2)
print(s:id())
s:apply(e1)
print(s:id())
s:apply(e3)
print(s:id())

print("Set key for s:\n")
s:setkey("k9", 2)
print(s)
print(s)
