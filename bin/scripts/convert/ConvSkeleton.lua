
local Class = require "Class"

local ConvSkeleton = Class("ConvSkeleton")

function ConvSkeleton:data()
    return self._data
end

function ConvSkeleton:bytes()
    return self._bytes
end

function ConvSkeleton:count()
    return self._count
end

function ConvSkeleton:boneIndicesByName()
    return self._byName
end

function ConvSkeleton:getBoneIndexMap()
    return self._indexMap
end

return ConvSkeleton
