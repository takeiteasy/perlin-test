function preframe() -- Called at start of every frame
	local x = Setting("xoff") -- Get setting "xoff"
	local y = Setting("yoff")
	local delta = Delta() * 10.0 -- Get delta time from progam
	Setting("xoff", x + delta) -- Set setting "xoff"
	Setting("yoff", y + delta)
end

function callback(v, x, y, w, h) -- Called pre-render (when settings have updated) Return the new height value (v is the original height)
	return v - math.sqrt((w / 2 - x) ^ 2 + (h / 2 - y) ^ 2) -- Apply circular gradient
end

function postframe(bitmap) -- Called pre-render and after biome colouring (when settings have updated)
	local w = heightmap:width()
	local h = heightmap:height()
	-- NOTE: Despire Lua's array's starting from 1, C's don't so start from 0
	for x=0, w do
		for y=0, h do
			local c = heightmap:pget(x, y) -- Original RGB value (ABGR formatted 32bit integer)
			local v = heightmap:get(x, y) -- Height value (0-255, 8bit unsigned char)
			
			-- Set the pixel color
			heightmap:pset(x, y, c)
		end
	end
end
