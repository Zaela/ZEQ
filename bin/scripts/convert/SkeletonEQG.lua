
local Class         = require "Class"
local BoneEntry     = require "BoneEntry"
local ConvSkele     = require "ConvSkeleton"
local AttachPoint   = require "AttachPoint"

local SkeletonEQG = Class("SkeletonEQG", ConvSkele)

function SkeletonEQG.new(root, count, byName)
    local entries           = BoneEntry.Array(count)
    local index             = 0
    local indexMap          = {}
    local listOrderIndexMap = {}
    
    local function recurse(bone)
        local childCount    = bone:getChildCount()
        local entry         = entries[index]
        
        indexMap[bone:index()] = index
        
        index = index + 1
        
        local pos   = bone:pos()
        local rot   = bone:rot()
        local scale = bone:scale()
        
        entry.childCount        = childCount
        entry.attachPointType   = AttachPoint.None --fixme
        entry.x                 = pos.x
        entry.y                 = pos.y
        entry.z                 = pos.z
        entry.rot.x             = rot.x
        entry.rot.y             = rot.y
        entry.rot.z             = rot.z
        entry.rot.w             = rot.w
        entry.scale.x           = scale.x
        entry.scale.y           = scale.y
        entry.scale.z           = scale.z
        
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
    
    return SkeletonEQG:instance(s)
end

return SkeletonEQG
