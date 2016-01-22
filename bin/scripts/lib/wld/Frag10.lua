
local bit            = require "bit"
local Struct        = require "Struct"
local Class            = require "Class"
local BinUtil        = require "BinUtil"
local FragHeader    = require "wld/FragHeader"

local Bone
local Frag10
local Frag = Class("Frag10", FragHeader)

function Frag:addChildren(vw)
    self.ref = vw:addByRef(self.ref)

    local ptr = BinUtil.Byte:cast(self) + Frag10:sizeof()

    -- skip optional fields if they exist
    if bit.band(self.flag, 1) ~= 0 then
        ptr = ptr + 12
    end
    if bit.band(self.flag, 2) ~= 0 then
        ptr = ptr + 4
    end

    -- bones
    for i = 1, self.numBones do
        local bone = Bone:cast(ptr)

        bone.nameref    = vw:addByRef(bone.nameref)
        bone.refA        = vw:addByRef(bone.refA)
        bone.refB        = vw:addByRef(bone.refB)

        ptr = ptr + (bone.size * 4 + Bone:sizeof())
    end

    -- ref list
    local int = BinUtil.Int:cast(ptr)
    local n = int[0]
    for i = 1, n do
        int[i] = vw:addByRef(int[i])
    end
end

Frag10 = Struct([[
    WLDFragHeader    header;
    uint32_t        flag;
    int                numBones;
    int                ref;
]], Frag)

Bone = Struct[[
    int            nameref;
    uint32_t    flag;
    int            refA;
    int            refB;
    int            size;
]]

return Frag10
