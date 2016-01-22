
local Struct		= require "Struct"
local Class			= require "Class"
local FragHeader	= require "wld/FragHeader"

local Simple = Class("SimpleFrag", FragHeader)

function Simple:addChildren(vw)
	self.ref = vw:addByRef(self.ref)
end

return Struct([[
	WLDFragHeader	header;
	int				ref;
	uint32_t		flag;
]], Simple)
