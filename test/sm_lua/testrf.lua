function readAll(file)
    local f = assert(io.open(file, "rb"))
    local content = f:read("*all")
    f:close()
    return content
end

s = readAll("test_sm_fsm_0.json")

print(s)