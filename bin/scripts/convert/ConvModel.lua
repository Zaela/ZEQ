
local Class         = require "Class"
local ConvObject    = require "ConvObject"

local ConvModel = Class("ConvModel", ConvObject)

function ConvModel.new()
    local m = ConvObject()
    
    m._materials                = {}
    m._textures                 = {}
    m._texturesByName           = {}
    m._maskedTextures           = {}
    m._maskedTexturesByName     = {}
    m._vertexBuffers            = {}
    m._noCollideVertexBuffers   = {}
    
    return ConvModel:instance(m)
end

function ConvModel:getVertexBuffer(index)
    return self._vertexBuffers[index], self._noCollideVertexBuffers[index]
end

function ConvModel:addMaterial(mat)
    table.insert(self._materials, mat)
end

function ConvModel:addTexture(tex, isMasked)
    local textures, byName
    
    if isMasked then
        textures    = self._maskedTextures
        byName      = self._maskedTexturesByName
    else
        textures    = self._textures
        byName      = self._texturesByName
    end
    
    table.insert(textures, tex)
    byName[tex:getName()] = tex
end

function ConvModel:getTextureByName(name, isMasked)
    local textures = isMasked and self._maskedTexturesByName or self._texturesByName
    return textures[name]
end

function ConvModel:getVertexBuffers()
    return self._vertexBuffers
end

function ConvModel:getNoCollideVertexBuffers()
    return self._noCollideVertexBuffers
end

local function iterator(tbl)
    local i = 0
    return function()
        i = i + 1
        return tbl[i]
    end
end

function ConvModel:materials()
    return iterator(self._materials)
end

function ConvModel:textures()
    return iterator(self._textures)
end

function ConvModel:maskedTextures()
    return iterator(self._maskedTextures)
end

function ConvModel:vertexBuffers()
    return iterator(self._vertexBuffers)
end

function ConvModel:noCollideVertexBuffers()
    return iterator(self._noCollideVertexBuffers)
end

function ConvModel:resetVertexBuffers()
    self._vertexBuffers             = {}
    self._noCollideVertexBuffers    = {}
end

return ConvModel
