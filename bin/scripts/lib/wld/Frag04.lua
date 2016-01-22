
local Struct		= require "Struct"
local Class			= require "Class"
local FragHeader	= require "wld/FragHeader"

local Animated
local Frag = Class("Frag04", FragHeader)

function Frag:isAnimated()
	return self.count > 1
end

function Frag:toAnimated()
	return Animated:cast(self)
end

function Frag:addChildren(vw)
	if self:isAnimated() then
		local f = self:toAnimated()
		for i = 0, f.count - 1 do
			f.refList[i] = vw:addByRef(f.refList[i])
		end
		return
	end

	self.ref = vw:addByRef(self.ref)
end

local Frag04 = Struct([[
	WLDFragHeader	header;
	uint32_t		flag;
	int				count;
	int				ref;
]], Frag)

Animated = Struct[[
	WLDFragHeader	header;
	uint32_t		flag;
	int				count;
	uint32_t		milliseconds;
	int				refList[0];
]]

return Frag04
