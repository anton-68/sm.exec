sm=require("sm")
sm.exec_init(16*1024*1024)

function string.tohex(str)
    return (str:gsub('.', function (c)
        return string.format('%02X', string.byte(c))
    end))
end

e = sm.new_event(7,128)
f = sm.new_event(8,128)

e_data = e:get_DCB("10.255.255.130")
e_data:write_int(7)
e_data:write_int(11)
e_data:write_int(13)
e_data:write_int(17)
e_data:write_int(19)
e_data:rewind()
e_data:read_int()
e_data:read_int()
e_data:read_int()
e_data:rewind()
e_data:write_int(23)
e_data:write_int(29)


