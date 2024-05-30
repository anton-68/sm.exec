sm = require("sm")
lib = sm.loadlib("/home/anton/lab/sm.exec/v5/test/sm_app/sm_test_apps.so")
at = sm.nat()
a1 = sm.lookup(lib, "sm_test_app1")
a2 = sm.lookup(lib, "sm_test_app2")
a3 = sm.lookup(lib, "sm_test_app3")
noop = sm.lookup(lib, "sm_nope")
at:set(a1, "APP1")
at:set(a2, "APP2")
at:set(a3, "APP3")
at:set(noop, "NOOP")
print(at)