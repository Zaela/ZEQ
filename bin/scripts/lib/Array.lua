
local ffi           = require "ffi"
local Class         = require "Class"
local ConvObject    = require "ConvObject"
local BinUtil       = require "BinUtil"

local Array = Class("Array", ConvObject)

function Array.new(struct)
    local a = ConvObject()
    
    a._arrayType        = struct
    a._arrayData        = struct.Array(32)
    a._arrayCount       = 0
    a._arrayCapacity    = 32
    
    return Array:instance(a)
end

local function grow(self)
    local type      = self._arrayType
    local oldCap    = self._arrayCapacity
    local cap       = oldCap * 2
    local array     = type.Array(cap)
    
    ffi.copy(array, self._arrayData, oldCap * type:sizeof())
    
    self._arrayData     = array
    self._arrayCapacity = cap
end

local function checkSize(self)
    local index = self._arrayCount
    self._arrayCount = index + 1
    if self._arrayCount == self._arrayCapacity then
        grow(self)
    end
    return self._arrayData[index]
end

function Array:pushBack(v)
    local o = checkSize(self)
    
    if v then
        ffi.copy(o, v, self._arrayType:sizeof())
    end
    
    return o
end

function Array:back()
    return self._arrayData[self._arrayCount - 1]
end

function Array:bytes()
    return self._arrayCount * self._arrayType:sizeof()
end

Array.length = Array.bytes

function Array:data()
    return self._arrayData
end

function Array:isEmpty()
    return self._arrayCount == 0
end

function Array:count()
    return self._arrayCount
end

function Array:sort(sorter)
    BinUtil.sortArray(self._arrayData, self._arrayCount, sorter, self._arrayType.Type)
end

function Array:iterator()
    local i     = 0
    local n     = self._arrayCount
    local array = self._arrayData
    return function()
        i = i + 1
        if i < n then return array[i] end
    end
end

function Array:foreach(func)
    local data = self._arrayData
    for i = 0, self._arrayCount - 1 do
        func(data[i], i, data)
    end
end

return Array
