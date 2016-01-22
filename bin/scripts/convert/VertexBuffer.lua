
local Vertex        = require "Vertex"
local Class         = require "Class"
local ffi           = require "ffi"
local ConvObject    = require "ConvObject"

local VertexBuffer = Class("VertexBuffer", ConvObject)

function VertexBuffer.new()
    local vb = ConvObject()
    
    vb._count       = 0
    vb._capacity    = 32
    vb._array       = Vertex.Array(32)
    
    return VertexBuffer:instance(vb)
end

function VertexBuffer:data()
    return self._array
end

function VertexBuffer:length()
    return self._count * Vertex:sizeof()
end

function VertexBuffer:count()
    return self._count
end

function VertexBuffer:isEmpty()
    return self._count == 0
end

function VertexBuffer:getMaterial()
    return self._material
end

function VertexBuffer:setMaterial(mat)
    self._material = mat
end

local function grow(self)
	local cap   = self._capacity * 2
	local array = Vertex.Array(cap)

	ffi.copy(array, self._array, Vertex:sizeof() * self._capacity)

	self._array     = array
	self._capacity  = cap
end

local function checkSize(self)
	local index = self._count
	self._count = index + 1
	if self._count == self._capacity then
		grow(self)
	end
	return self._array[index]
end

function VertexBuffer:addVertex()
	return checkSize(self)
end

function VertexBuffer:addVertexCopy(vert)
    ffi.copy(checkSize(self), vert, Vertex:sizeof())
end

return VertexBuffer
