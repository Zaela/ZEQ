
local bit           = require "bit"
local Struct        = require "Struct"
local Class         = require "Class"
local BinUtil       = require "BinUtil"
local FragHeader    = require "wld/FragHeader"
local Bone          = require "wld/Frag10Bone"

local Frag10
local Frag = Class("Frag10", FragHeader)

function Frag:hasRefList()
    return bit.band(self.flag, 2^9) ~= 0
end

function Frag:boneCount()
    return self.numBones
end

local function skipFlags(self)
    local ptr = BinUtil.Byte:cast(self) + Frag10:sizeof()
    
    -- Skip optional fields if they exist
    if bit.band(self.flag, 1) ~= 0 then
        ptr = ptr + 12
    end
    if bit.band(self.flag, 2) ~= 0 then
        ptr = ptr + 4
    end
    
    return ptr
end

function Frag:boneList()
    return Bone:cast(skipFlags(self))
end

function Frag:refList()
    local ptr = skipFlags(self)
    
    -- Skip bones (var size)
    for i = 1, self:boneCount() do
        ptr = ptr + 16
        local count = BinUtil.Int:cast(ptr)
        ptr = ptr + (count[0] + 1) * 4
    end
    
    local count = BinUtil.Int:cast(ptr)
    return count + 1, count[0]
end

Frag10 = Struct([[
    WLDFragHeader   header;
    uint32_t        flag;
    int             numBones;
    int             ref;
]], Frag)

return Frag10
