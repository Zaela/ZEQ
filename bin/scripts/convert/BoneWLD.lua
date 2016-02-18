
local Class     = require "Class"
local ConvBone  = require "ConvBone"

local BoneWLD = Class("BoneWLD", ConvBone)

function BoneWLD.new(name, entry, index)
    local b = ConvBone(name)
    
    b:setIndex(index)
    
    local pos, rot = BoneWLD.f12EntryToPosRot(entry)
    
    b._children = {}
    b._pos      = pos
    b._rot      = rot
    
    return BoneWLD:instance(b)
end

function BoneWLD.f12EntryToPosRot(entry)
    local pos = {
        x = 0,
        y = 0,
        z = 0,
    }
    
    local rot = {
        x = 0,
        y = 0,
        z = 0,
        w = 1,
    }

    if entry.shiftDenom ~= 0 then
        local denom = entry.shiftDenom
        
        pos.x = entry.shiftX / denom
        pos.y = entry.shiftY / denom
        pos.z = entry.shiftZ / denom
    end
    
    if entry.rotW ~= 0 then
        rot.x = -entry.rotX / 16384
        rot.y = -entry.rotY / 16384
        rot.z = -entry.rotZ / 16384
        rot.w =  entry.rotW / 16384
    end
    
    return pos, rot
end

return BoneWLD
