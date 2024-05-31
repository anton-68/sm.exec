local citylist = {}
for line in io.lines("citys.csv") do
    local city, region, coalition, coordinate_x, coordinate_y = line:match("%s*(.-),%s*(.-),%s*(.-),%s*(.-),%s*(.-)")
    citylist[#citylist + 1] = { city = city, region = region, coalition = coalition, coordinate_x = coordinate_x, coordinate_y = coordinate_y }
end
