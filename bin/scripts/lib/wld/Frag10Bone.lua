
local Struct    = require "Struct"
local Class     = require "Class"
local BinUtil   = require "BinUtil"

local Frag10Bone
local Bone = Class("Frag10Bone")

function Bone:next()
    return Frag10Bone:cast(BinUtil.Byte:cast(self) + Frag10Bone:sizeof() + self.size * 4)
end

function Bone:indexList()
    return BinUtil.Int:cast(BinUtil.Byte:cast(self) + Frag10Bone:sizeof())
end

Frag10Bone = Struct([[
    int         nameref;
    uint32_t    flag;
    int         refA;
    int         refB;
    int         size;
]], Bone)

return Frag10Bone
