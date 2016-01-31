
local Struct    = require "Struct"
local Class     = require "Class"

local BoneEntry = Struct.packed[[
    uint32_t    childCount;
    float       x, y, z;
    struct { float x, y, z, w; } rot;
    struct { float x, y, z; } scale;
]]

local SkeletonEQG = Class("SkeletonEQG")

function SkeletonEQG.new(root, count, byName, recurseOrder)
    local entries           = BoneEntry.Array(count)
    local index             = 0
    local indexMap          = {}
    local listOrderIndexMap = {}
    
    local function recurse(bone)
        local childCount    = bone:getChildCount()
        local entry         = entries[index]
        
        indexMap[bone:index()] = index
        --io.write(bone:index(), " -> ", index, " (", bone:getName(), ")\n")
        
        index = index + 1
        
        local pos   = bone:pos()
        local rot   = bone:rot()
        local scale = bone:scale()
        
        entry.childCount    = childCount
        entry.x             = pos.x
        entry.y             = pos.y
        entry.z             = pos.z
        entry.rot.x         = rot.x
        entry.rot.y         = rot.y
        entry.rot.z         = rot.z
        entry.rot.w         = rot.w
        entry.scale.x       = scale.x
        entry.scale.y       = scale.y
        entry.scale.z       = scale.z
        
        for child in bone:children() do
            recurse(child)
        end
    end
    
    recurse(root)
    
    --[[
    for i, bone in ipairs(listOrder) do
        listOrderIndexMap[i - 1] = indexMap[bone:index()]
        io.write(bone:index(), " -> ", i - 1, " (", bone:getName(), ")\n")
    end
    --]]
    
    local s = {
        _data           = entries,
        _bytes          = count * BoneEntry:sizeof(),
        _count          = count,
        _byName         = byName,
        _indexMap       = indexMap,
        _recurseOrder   = recurseOrder,
    }
    
    return SkeletonEQG:instance(s)
end

function SkeletonEQG:data()
    return self._data
end

function SkeletonEQG:bytes()
    return self._bytes
end

function SkeletonEQG:count()
    return self._count
end

function SkeletonEQG:boneIndicesByName()
    return self._byName
end

function SkeletonEQG:getBoneIndexMap()
    return self._indexMap
end

return SkeletonEQG
