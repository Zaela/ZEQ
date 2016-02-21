
local Class         = require "Class"
local BoneEntry     = require "BoneEntry"
local ConvSkele     = require "ConvSkeleton"
local AttachPoint   = require "AttachPoint"

local SkeletonWLD = Class("SkeletonWLD", ConvSkele)

local pointValues = {
    head    = AttachPoint.Camera,
    r       = AttachPoint.RightHand,
    l       = AttachPoint.LeftHand,
    shield  = AttachPoint.Shield,
}

function SkeletonWLD.new(root, count, attachBones)
    local entries       = BoneEntry.Array(count)
    local index         = 0
    local indexMap      = {}
    local byName        = {}
    local indexToParent = {}
    
    local function getAttachType(bone)
        if not attachBones[bone] then return -1 end
        
        local name = bone:getName():sub(4)
        name = name:match("^%a+"):lower()

        return pointValues[name] or -1
    end
    
    local function recurse(bone)
        local childCount    = bone:getChildCount()
        local entry         = entries[index]
        
        indexMap[bone:index()] = index
        byName[bone:getName()] = index
        
        local thisIndex = index
        index = index + 1
        
        local pos = bone:pos()
        local rot = bone:rot()
        
        entry.childCount        = childCount
        entry.attachPointType   = getAttachType(bone)
        entry.x                 = pos.x
        entry.y                 = pos.y
        entry.z                 = pos.z
        entry.rot.x             = rot.x
        entry.rot.y             = rot.y
        entry.rot.z             = rot.z
        entry.rot.w             = rot.w
        entry.scale.x           = 1
        entry.scale.y           = 1
        entry.scale.z           = 1
        
        for child in bone:children() do
            indexToParent[index] = thisIndex
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
        _indexToParent  = indexToParent,
    }
    
    return SkeletonWLD:instance(s)
end

function SkeletonWLD:getParentIndexByBoneIndex(index)
    return self._indexToParent[index]
end

return SkeletonWLD
