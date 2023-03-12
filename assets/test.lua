function preframe()
	local x = Setting("xoff") -- Get setting "xoff"
	local y = Setting("yoff")
	local delta = Delta() * 10.0 -- Get delta time from progam
	Setting("xoff", x + delta) -- Set setting "xoff"
	Setting("yoff", y + delta)
end

function callback(v, x, y, w, h)
	return v - math.sqrt((w / 2 - x) ^ 2 + (h / 2 - y) ^ 2) -- Apply circular gradient
end
