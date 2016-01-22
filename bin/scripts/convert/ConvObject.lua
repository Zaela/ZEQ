
local Class = require "Class"

local ConvObject = Class("ConvObject")

function ConvObject.new()
    local obj = {
        _convName = "",
    }
    
    return ConvObject:instance(obj)
end

function ConvObject:getId()
    return self._convId
end

function ConvObject:setId(id)
    self._convId = id
end

function ConvObject:getName()
    return self._convName
end

function ConvObject:setName(name)
    self._convName = name
end

return ConvObject
