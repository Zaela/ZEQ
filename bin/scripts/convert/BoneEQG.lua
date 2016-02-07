
local Class     = require "Class"
local ConvBone  = require "ConvBone"

local table = table

local BoneEQG = Class("BoneEQG", ConvBone)

function BoneEQG.new(name, pos, rot, scale)
    local b = ConvBone(name)
    
    b._pos      = pos
    b._rot      = rot
    b._scale    = scale

    return BoneEQG:instance(b)
end

function BoneEQG:scale()
    return self._scale
end

return BoneEQG
