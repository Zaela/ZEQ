
local Struct		= require "Struct"
local FragHeader	= require "wld/FragHeader"

return Struct.packed([[
	WLDFragHeader	header;
	int				count;
	uint16_t		stringLen;
	uint8_t			string[0];
]], FragHeader)
