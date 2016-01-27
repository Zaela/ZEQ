
local Vertex    = require "Vertex"
local Class     = require "Class"
local Array     = require "Array"

local VertexBuffer = Class("VertexBuffer", Array)

function VertexBuffer.new()
    local vb = Array(Vertex)
    return VertexBuffer:instance(vb)
end

function VertexBuffer:getMaterial()
    return self._material
end

function VertexBuffer:setMaterial(mat)
    self._material = mat
end

function VertexBuffer:addVertex()
    return self:pushBack()
end

function VertexBuffer:addVertexCopy(vert)
    self:pushBack(vert)
end

return VertexBuffer
