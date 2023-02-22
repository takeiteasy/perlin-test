function heightmap_callback(heightmap)
	for i=0,512 do
		for j=0,512 do
			local c = heightmap:pget(i, j)
			local h = c & 0xFF
			if h >= 26 and h < 37 then
				c = 0xFFE65F0B
			elseif h >= 37 and h < 70 then
				c = 0xFF3FB4D4
			elseif h >= 70 and h < 100 then
				c = 0xFF25A853
			elseif h >= 100 and h < 113 then
				c = 0xFF125C22
			else
				c = 0xFFF58742
			end
			heightmap:pset(i, j, c)
		end
	end
end
