
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
    b._index    = 0
    
    b:setName(name)

    return BoneEQG:instance(b)
end

function BoneEQG:pos()
    return self._pos
end

function BoneEQG:rot()
    return self._rot
end

function BoneEQG:scale()
    return self._scale
end

function BoneEQG:addChild(bone)
    table.insert(self._children, bone)
end

function BoneEQG:getChildCount()
    return #self._children
end

function BoneEQG:children()
    local i         = 0
    local children  = self._children
    return function()
        i = i + 1
        return children[i]
    end
end

function BoneEQG:setIndex(index)
    self._index = index
end

function BoneEQG:index()
    return self._index
end

return BoneEQG
