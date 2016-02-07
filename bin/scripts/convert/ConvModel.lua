
local Class         = require "Class"
local ConvObject    = require "ConvObject"
local VertexBuffer  = require "VertexBuffer"
local ConvMaterial  = require "ConvMaterial"

local pairs = pairs

local ConvModel = Class("ConvModel", ConvObject)

function ConvModel.new()
    local m = ConvObject()
    
    m._materials                = {}
    m._textures                 = {}
    m._texturesByName           = {}
    m._maskedTextures           = {}
    m._maskedTexturesByName     = {}
    m._normalMaps               = {}
    m._normalMapsByName         = {}
    m._vertexBuffers            = {}
    m._noCollideVertexBuffers   = {}
    m._headModels               = {}
    
    return ConvModel:instance(m)
end

function ConvModel:initVertexBuffers(count, bindMaterials)
    count       = count or #self._materials
    local vbs   = self._vertexBuffers
    local cvb   = self._noCollideVertexBuffers
    
    for i = 0, count do
        vbs[i] = VertexBuffer()
    end
    for i = 0, count do
        cvb[i] = VertexBuffer()
    end
    
    -- Zeroth buffers always use the NULL material (used by EQG models only)
    vbs[0]:setMaterial(ConvMaterial.NULL)
    cvb[0]:setMaterial(ConvMaterial.NULL)
    
    if bindMaterials then
        local mats = self._materials
        
        for i = 1, count do
            local mat       = mats[i]
            local vb, cb    = vbs[i], cvb[i]
            
            vb:setMaterial(mat)
            cb:setMaterial(mat)
        end
    end
end

function ConvModel:inheritTextures(from)
    self._textures              = from._textures
    self._texturesByName        = from._texturesByName
    self._maskedTextures        = from._maskedTextures
    self._maskedTexturesByName  = from._maskedTexturesByName
    self._normalMaps            = from._normalMaps
    self._normalMapsByName      = from._normalMapsByName
end

function ConvModel:getAllMaterials()
    return self._materials
end

function ConvModel:setAllMaterials(mats)
    self._materials = mats
end

function ConvModel:getMaterial(index)
    return self._materials[index]
end

function ConvModel:getVertexBuffer(index)
    return self._vertexBuffers[index], self._noCollideVertexBuffers[index]
end

function ConvModel:addMaterial(mat)
    table.insert(self._materials, mat)
end

function ConvModel:addHeadModel(model, i)
    local heads = self._headModels
    i = i or #heads + 1
    heads[i] = model
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

function ConvModel:addNormalMap(tex)
    table.insert(self._normalMaps, tex)
    self._normalMapsByName[tex:getName()] = tex
end

function ConvModel:getTextureByName(name, isMasked)
    local textures = isMasked and self._maskedTexturesByName or self._texturesByName
    return textures[name]
end

function ConvModel:addVertexBuffer(vb)
    table.insert(self._vertexBuffers, vb)
end

function ConvModel:addNoCollideVertexBuffer(vb)
    table.insert(self._noCollideVertexBuffers, vb)
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

local function iterator2(a, b)
    local i = 0
    return function()
        i = i + 1
        return a[i], b[i]
    end
end

function ConvModel:materials()
    return iterator(self._materials)
end

function ConvModel:textures()
    return iterator2(self._textures, self._normalMaps)
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

function ConvModel:headModels()
    return pairs(self._headModels)
end

function ConvModel:resetVertexBuffers()
    self._vertexBuffers             = {}
    self._noCollideVertexBuffers    = {}
end

function ConvModel:setSkeleton(skele)
    self._skele = skele
end

function ConvModel:skeleton()
    return self._skele
end

return ConvModel
