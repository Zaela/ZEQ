
local Class     = require "Class"
local ConvModel = require "ConvModel"
local BAEQG     = require "BoneAssignmentEQG"

local ModelEQG = Class("ModelEQG", ConvModel)

function ModelEQG.new()
    local m = ConvModel()
    return ModelEQG:instance(m)
end

function ModelEQG:initWeightBuffers()
    local vbs   = self:getVertexBuffers()
    local cvb   = self:getNoCollideVertexBuffers()
    local count = #vbs
    
    local wt, nwt = {}, {}
    
    for i = 1, count do
        wt[i]   = BAEQG(vbs[i])
        nwt[i]  = BAEQG(cvb[i])
    end
    
    self._weightBuffers             = wt
    self._noCollideWeightBuffers    = nwt
end

function ModelEQG:getWeightBuffer(index)
    return self._weightBuffers[index], self._noCollideWeightBuffers[index]
end

local function iter(tbl)
    local i = 0
    return function()
        i = i + 1
        return tbl[i]
    end
end

function ModelEQG:weightBuffers()
    return iter(self._weightBuffers)
end

function ModelEQG:noCollideWeightBuffers()
    return iter(self._noCollideWeightBuffers)
end

function ModelEQG:sortWeights()
    local function sort(iter)
        for wt in iter do
            if not wt:isEmpty() then
                wt:sort()
            end
        end
    end
    
    sort(self:weightBuffers())
    sort(self:noCollideWeightBuffers())
end

function ModelEQG:setSkeleton(skele)
    self._skele = skele
end

function ModelEQG:getSkeleton()
    return self._skele
end

return ModelEQG
