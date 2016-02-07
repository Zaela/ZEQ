
local Class     = require "Class"
local BoneEntry = require "BoneEntry"
local ConvSkele = require "ConvSkeleton"

local SkeletonWLD = Class("SkeletonWLD", ConvSkele)

function SkeletonWLD.new(root, count)
    local entries   = BoneEntry.Array(count)
    local index     = 0
    local indexMap  = {}
    local byName    = {}
    
    local function recurse(bone)
        local childCount    = bone:getChildCount()
        local entry         = entries[index]
        
        indexMap[bone:index()] = index
        byName[bone:getName()] = index
        
        index = index + 1
        
        local pos = bone:pos()
        local rot = bone:rot()
        
        entry.childCount    = childCount
        entry.x             = pos.x
        entry.y             = pos.y
        entry.z             = pos.z
        entry.rot.x         = rot.x
        entry.rot.y         = rot.y
        entry.rot.z         = rot.z
        entry.rot.w         = rot.w
        entry.scale.x       = 1
        entry.scale.y       = 1
        entry.scale.z       = 1
        
        for child in bone:children() do
            recurse(child)
        end
    end
    
    recurse(root)
    
    local s = {
        _data           = entries,
        _bytes          = count * BoneEntry:sizeof(),
        _count          = count,
        _byName         = byName,
        _indexMap       = indexMap,
    }
    
    return SkeletonWLD:instance(s)
end

return SkeletonWLD
