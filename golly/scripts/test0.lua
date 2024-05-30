-- Learning about golly scripting
-- (c) anton.bondarenko@gmail.com 

local g = golly()







local bbox = g.getrect()
if #bbox == 0 then g.exit("The pattern is empty.") end

local d = g.getpop() / (bbox[3] * bbox[4])
if d < 0.000001 then
    g.show(string.format("Density = %.1e", d))
else
    g.show(string.format("Density = %.6f", d))
end
