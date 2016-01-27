
local ffi           = require "ffi"
local Struct        = require "Struct"
local Class         = require "Class"
local Array         = require "Array"

local BA = Struct.packed[[
    uint32_t    vertIndex;
    uint32_t    boneIndex;
    float       weight;
]]

local BAEQG = Class("BoneAssignmentEQG", Array)

function BAEQG.new(vb)
    local b = Array(BA)

    b._vertexBuffer = vb
    
    return BAEQG:instance(b)
end

function BAEQG:add(vertIndex, boneIndex, weight)
    local o = self:pushBack()
    
    o.vertIndex = vertIndex
    o.boneIndex = boneIndex
    o.weight    = weight
end

function BAEQG:sort()
    return Array.sort(self, function(a, b) return a.vertIndex < b.vertIndex end)
end

function BAEQG:vertexBuffer()
    return self._vertexBuffer
end

function BAEQG:isWeighted()
    return true
end

return BAEQG
