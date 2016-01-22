
local Class         = require "Class"
local VertexBuffer  = require "VertexBuffer"
local ConvModel     = require "ConvModel"

local table     = table
local ipairs    = ipairs

local ModelWLD = Class("ModelWLD", ConvModel)

function ModelWLD.new(pfs, wld)
    local m = ConvModel()
    
    m._pfs                      = pfs
    m._wld                      = wld
    m._materialsByF30           = {}
    
    return ModelWLD:instance(m)
end

function ModelWLD:getPFS()
    return self._pfs
end

function ModelWLD:getWLD()
    return self._wld
end

function ModelWLD:addMaterialIndexByF30(addr, index)
    self._materialsByF30[addr] = index
end

function ModelWLD:getMaterialIndicesByF30()
    return self._materialsByF30
end

function ModelWLD:append(model)
    local function appendTbl(to, from)
        for i, val in ipairs(from) do
            table.insert(to, val)
        end
    end
    
    appendTbl(self._materials,              model._materials)
    appendTbl(self._textures,               model._textures)
    appendTbl(self._maskedTextures,         model._maskedTextures)
    appendTbl(self._vertexBuffers,          model._vertexBuffers)
    appendTbl(self._noCollideVertexBuffers, model._noCollideVertexBuffers)
end

return ModelWLD
