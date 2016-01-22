
local Struct        = require "Struct"
local Class            = require "Class"
local FragHeader    = require "wld/FragHeader"

local Frag = Class("Frag31", FragHeader)

function Frag:addChildren(vw)
    for i = 0, self.refCount - 1 do
        self.refList[i] = vw:addByRef(self.refList[i])
    end
end

return Struct([[
    WLDFragHeader    header;
    uint32_t        flag;
    uint32_t        refCount;
    int                refList[0];
]], Frag)
