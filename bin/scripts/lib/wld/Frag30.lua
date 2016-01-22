
local Struct        = require "Struct"
local Class            = require "Class"
local FragHeader    = require "wld/FragHeader"

local Frag = Class("Frag30", FragHeader)

function Frag:addChildren(vw)
    self.ref = vw:addByRef(self.ref)
end

return Struct([[
    WLDFragHeader    header;
    uint32_t        flag;
    uint32_t        visibilityFlag;
    uint32_t        unknown[3];
    int                ref;
    int                unknownB[2];
]], Frag)
