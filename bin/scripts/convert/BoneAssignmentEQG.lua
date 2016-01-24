
local ffi           = require "ffi"
local Struct        = require "Struct"
local Class         = require "Class"
local BinUtil       = require "BinUtil"
local ConvObject    = require "ConvObject"

local BA = Struct.packed[[
    uint32_t    vertIndex;
    uint32_t    boneIndex;
    float       weight;
]]

local BAEQG = Class("BoneAssignmentEQG")

function BAEQG.new(vb)
    local b = ConvObject()
    
    local raw = BinUtil.Byte.Array(4 + BA:sizeof() * 32)
    
    b._raw          = raw
    b._count        = 0
    b._capacity     = 32
    b._array        = BA:cast(raw + 4)
    b._vertexBuffer = vb
    
    BinUtil.Uint32:cast(raw)[0] = 1
    
    return BAEQG:instance(b)
end

local function grow(self)
    local cap = self._capacity * 2
    local raw = BinUtil.Byte.Array(4 + cap * BA:sizeof())

    ffi.copy(raw, self._raw, 4 + BA:sizeof() * self._capacity)

    self._raw       = raw
    self._array     = BA:cast(raw + 4)
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

function BAEQG:add(vertIndex, boneIndex, weight)
    local o = checkSize(self)
    
    o.vertIndex = vertIndex
    o.boneIndex = boneIndex
    o.weight    = weight
end

function BAEQG:bytes()
    return 4 + self._count * BA:sizeof()
end

function BAEQG:data()
    return self._raw
end

local function sorter(a, b)
    return a.vertIndex < b.vertIndex
end

function BAEQG:sort()
    BinUtil.sortArray(self._array, self._count, sorter, BA.Type)
end

function BAEQG:isEmpty()
    return self._count == 0
end

function BAEQG:vertexBuffer()
    return self._vertexBuffer
end

return BAEQG
