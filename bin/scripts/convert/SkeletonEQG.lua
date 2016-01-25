
local Struct    = require "Struct"
local Class     = require "Class"

local BoneEntry = Struct.packed[[
    uint32_t    childCount;
    float       x, y, z;
    struct { float x, y, z, w; } rot;
    struct { float x, y, z; } scale;
]]

local SkeletonEQG = Class("SkeletonEQG")

function SkeletonEQG.new(root, count, byName)
    local entries   = BoneEntry.Array(count)
    local index     = 0
    
    local function recurse(bone)
        local childCount    = bone:getChildCount()
        local entry         = entries[index]
        
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
    
    local s = {
        _data   = entries,
        _bytes  = count * BoneEntry:sizeof(),
        _count  = count,
        _byName = byName,
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

return SkeletonEQG
