-- Uses height and circular gradient to create an island from perlin noise

function preframe(heightmap) -- optional, called at start of frame
	local x = Setting("xoff") -- Get setting "xoff"
	local y = Setting("yoff")
	local delta = Delta() * 10.0 -- Get delta time from progam
	Setting("xoff", x + delta) -- Set setting "xoff"
	Setting("yoff", y + delta)
end

function frame(heightmap) -- called pre-render when canvas updates
	local w = heightmap:width()
	local h = heightmap:height()
	-- NOTE: Despire Lua's array's starting from 1, C's don't so start from 0
	for x=0, w do
		for y=0, h do
			local c = heightmap:pget(x, y) -- Original RGB value (ABGR formatted 32bit integer)
			local v = heightmap:get(x, y) - math.sqrt((w / 2 - x) ^ 2 + (h / 2 - y) ^ 2) -- Height value (0-255, 8bit unsigned char)
			
			-- Decide the biome based on the height
			if v >= 26 and v < 37 then
				c = RGB(11, 95, 230) -- Shallow water, dark blue, 0xFFE65F0B
			elseif v >= 37 and v < 70 then
				c = RGB(212, 180, 63)  -- Sand/beach, yellow, 0xFF3FB4D4
			elseif v >= 70 and v < 100 then
				c = RGB(83, 168, 37) -- Grass, green, 0xFF25A853
			elseif v >= 100 and v < 113 then
				c = RGB(34, 92, 18) -- Forest, dark green, 0xFF125C22
			else
				c = RGB(66, 135, 245) -- Ocean, blue, 0xFFF58742
			end
			
			-- Set the pixel color
			heightmap:pset(x, y, c)
		end
	end
end
