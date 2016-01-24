
local Class         = require "Class"
local ConvObject    = require "ConvObject"

local table = table

local BoneEQG = Class("EQGBone", ConvObject)

function BoneEQG.new(name, pos, rot, scale)
    local b = ConvObject()
    
    b._children = {}
    b._pos      = pos
    b._rot      = rot
    b._scale    = scale
    
    b:setName(name)

    return BoneEQG:instance(b)
end

function BoneEQG:addChild(bone)
    table.insert(self._children, bone)
end

return BoneEQG
