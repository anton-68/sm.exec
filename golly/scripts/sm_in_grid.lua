--[[
function sm_in_grid(W, H, X, Y)
    res = {}
    if X < 0 then res.X = 0 end
    if X >= W then res.X = W-1 end
    if Y < 0 then res.Y = 0 end
    if X >= W then res.X = W-1 end               
    return res
end

    if sm_in_grid(w, h, b.X, b.Y + 1) then e_data:write_int(g.getcell(b.X, b.Y + 1)) 
    else e_data:write_int(6) end
    -- NE
    if sm_in_grid(w, h, b.X + 1, b.Y + 1) then e_data:write_int(g.getcell(b.X + 1, b.Y + 1)) 
    else e_data:write_int(6) end    
    -- E
    if sm_in_grid(w, h, b.X + 1, b.Y) then e_data:write_int(g.getcell(b.X + 1, b.Y)) 
    else e_data:write_int(6) end
    -- SE
    if sm_in_grid(w, h, b.X + 1, b.Y - 1) then e_data:write_int(g.getcell(b.X + 1, b.Y - !)) 
    else e_data:write_int(6) end
    -- S
    if sm_in_grid(w, h, b.X, b.Y - 1) then e_data:write_int(g.getcell(b.X, b.Y - 1)) 
    else e_data:write_int(6) end
    -- SW
    if sm_in_grid(w, h, b.X - 1, b.Y - 1) then e_data:write_int(g.getcell(b.X - 1, b.Y - 1)) 
    else e_data:write_int(6) end
    -- W
    if sm_in_grid(w, h, b.X - 1, b.Y) then e_data:write_int(g.getcell(b.X - 1, b.Y)) 
    else e_data:write_int(6) end
    -- NW
    if sm_in_grid(w, h, b.X - 1, b.Y + 1) then e_data:write_int(g.getcell(b.X - 1, b.Y + 1)) 
    else e_data:write_int(6) end 
]]--