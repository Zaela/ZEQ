
local Class         = require "Class"
local ConvObject    = require "ConvObject"

local table = table

local ConvBone = Class("ConvBone", ConvObject)

function ConvBone.new(name)
    local b = ConvObject()
    
    b._children = {}
    b._index    = 0
    
    b:setName(name)
    
    return ConvBone:instance(b)
end

function ConvBone:pos()
    return self._pos
end

function ConvBone:rot()
    return self._rot
end

function ConvBone:addChild(bone)
    table.insert(self._children, bone)
end

function ConvBone:getChildCount()
    return #self._children
end

function ConvBone:children()
    local i         = 0
    local children  = self._children
    return function()
        i = i + 1
        return children[i]
    end
end

function ConvBone:setIndex(index)
    self._index = index
end

function ConvBone:index()
    return self._index
end

return ConvBone
