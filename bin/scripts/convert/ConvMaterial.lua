
local Class         = require "Class"
local BlendTypes    = require "BlendTypes"
local ConvObject    = require "ConvObject"

local table = table

local ConvMaterial = Class("ConvMaterial", ConvObject)

function ConvMaterial.new(name)
    local mat = ConvObject()
    
    mat:setName(name)
    
    mat._animDelayMs    = 0
    mat._blendType      = BlendTypes.Solid
    mat._textures       = {}
    
    return ConvMaterial:instance(mat)
end

function ConvMaterial:isMasked()
    local bt = self._blendType
    return bt == BlendTypes.Masked or bt == BlendTypes.Particle
end

function ConvMaterial:isInvisible()
    return self._blendType == BlendTypes.Invisible
end

function ConvMaterial:getAnimDelay()
    return self._animDelayMs
end

function ConvMaterial:setAnimDelay(ms)
    self._animDelayMs = ms
end

function ConvMaterial:getTextureSetId()
    return self._texSetId
end

function ConvMaterial:setTextureSetId(id)
    self._texSetId = id
end

function ConvMaterial:getTextureCount()
    return #self._textures
end

function ConvMaterial:getTextureAt(index)
    return self._textures[index]
end

function ConvMaterial:addTexture(tex)
    table.insert(self._textures, tex)
end

function ConvMaterial:textures()
    local i         = 0
    local textures  = self._textures
    return function()
        i = i + 1
        return textures[i]
    end
end

function ConvMaterial:getBlendType()
    return self._blendType
end

function ConvMaterial:setBlendType(t)
    self._blendType = t
end

ConvMaterial.NULL = ConvMaterial("NULL")
ConvMaterial.NULL:setBlendType(BlendTypes.Invisible)

return ConvMaterial
