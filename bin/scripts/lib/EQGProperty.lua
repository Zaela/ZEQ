
local Struct    = require "Struct"
local Class     = require "Class"

local Property = Class("EQGProperty")

function Property:getName(strings)
    return strings[self.typeNameIndex]
end

function Property:getValueType()
    local t = self.valueType
    
    if t == 0 then
        return "float"
    elseif t == 2 then
        return "string"
    elseif t == 3 then
        return "color"
    end
end

function Property:getValue(strings)
    local t = self.valueType
    
    if t == 0 then
        return self.asFloat
    elseif t == 2 then
        return strings[self.asIndex]
    elseif t == 3 then
        return self.asColor
    end
end

local EQGProperty = Struct.namedPacked("EQGProperty", [[
    uint32_t typeNameIndex; // Index of the property's type name in the file's string block
    uint32_t valueType;     // Type of the property's value: 0 = float, 2 = string index int, 3 = ARGB color value int
    union {
        uint32_t    asIndex;
        float       asFloat;
        struct { uint8_t a, r, g, b; } asColor;
    };
]], Property)

return EQGProperty
