
local Struct        = require "Struct"
local Class         = require "Class"
local FragHeader    = require "wld/FragHeader"

local Frag = Class("Frag13", FragHeader)

function Frag:addChildren(vw)
    self.ref = vw:addByRef(self.ref)
end

return Struct([[
    WLDFragHeader   header;
    int             ref;
    uint32_t        flag;
    uint32_t        param;
]], Frag)
