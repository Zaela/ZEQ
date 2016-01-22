
local Class         = require "Class"
local Image         = require "Image"
local ConvObject    = require "ConvObject"

local Texture = Class("Texture", ConvObject)

function Texture.new(name, data, len)
    local tex = ConvObject()
    
    tex:setName(name)
    
    tex._data   = data
    tex._len    = len
    
    return Texture:instance(tex)
end

function Texture:open()
    self._image = Image(self._data, self._len)
    self._image:open()
end

function Texture:close()
    self._image:close()
end

function Texture:normalize()
    local data, len, w, h = self._image:normalize()
    
    self._data      = data
    self._len       = len
    self._width     = w
    self._height    = h
end

function Texture:data()
    return self._data
end

function Texture:length()
    return self._len
end

function Texture:width()
    return self._width
end

function Texture:height()
    return self._height
end

function Texture:mask()
    local data, len = self._image:mask()
    
    if not data then return end
    
    self._data  = data
    self._len   = len
end

return Texture
