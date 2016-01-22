
local Struct        = require "Struct"
local Class            = require "Class"
local FragHeader    = require "wld/FragHeader"
local BinUtil        = require "BinUtil"
local bit            = require "bit"

local Frag14
local Frag = Class("Frag14", FragHeader)

function Frag:hasMeshRefs()
    return self.size[1] > 0
end

local function skipToRefList(self)
    local ptr = BinUtil.Byte:cast(self) + Frag14:sizeof()
    ptr = BinUtil.Int:cast(ptr)

    -- skip optional fields, if they are indicated to exist
    if bit.band(self.flag, 1) ~= 0 then
        ptr = ptr + 1
    end
    if bit.band(self.flag, 2) ~= 0 then
        ptr = ptr + 1
    end
    -- skip variable size portions
    for i = 0, self.size[0] - 1 do
        ptr = ptr + (ptr[0] * 2 + 1)
    end

    return ptr
end

function Frag:getFirstRefPtr()
    local ptr = skipToRefList(self)
    return ptr
end

function Frag:getFirstRef()
    return self:getFirstRefPtr()[0]
end

function Frag:getMeshRefCount()
    return self.size[1]
end

function Frag:addChildren(vw)
    self.refA = vw:addByRef(self.refA)
    self.refB = vw:addByRef(self.refB)

    if not self:hasMeshRefs() then return end

    local ptr = self:getFirstRefPtr()
    for i = 0, self:getMeshRefCount() - 1 do
        ptr[i] = vw:addByRef(ptr[i])
    end
end

Frag14 = Struct([[
    WLDFragHeader    header;
    uint32_t        flag;
    int                refA;
    int                size[2];
    int                refB;
]], Frag)

return Frag14
