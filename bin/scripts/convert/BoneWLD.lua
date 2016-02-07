
local Class     = require "Class"
local ConvBone  = require "ConvBone"

local BoneWLD = Class("BoneWLD", ConvBone)

function BoneWLD.new(name, entry, index)
    local b = ConvBone(name)
    
    b:setIndex(index)
    
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
    
    b._children = {}
    b._pos      = pos
    b._rot      = rot

    if entry.shiftDenom ~= 0 then
        local denom = entry.shiftDenom
        
        pos.x = entry.shiftX / denom
        pos.y = entry.shiftZ / denom
        pos.z = entry.shiftY / denom
    end
    
    if entry.rotDenom ~= 0 then
        local denom = entry.rotDenom
    
        local x = entry.rotX / denom * 90.0
        local y = entry.rotZ / denom * 90.0
        local z = entry.rotY / denom * 90.0
        local w
        
        -- Convert euler angles to quaternion
        x = x * 0.5
        local sr = math.sin(x)
        local cr = math.cos(x)
        
        y = y * 0.5
        local sp = math.sin(y)
        local cp = math.cos(y)
        
        z = z * 0.5
        local sy = math.sin(z)
        local cy = math.cos(z)
        
        local cpcy = cp * cy
        local spcy = sp * cy
        local cpsy = cp * sy
        local spsy = sp * sy
        
        x = sr * cpcy - cr * spsy
        y = cr * spcy + sr * cpsy
        z = cr * cpsy - sr * spcy
        w = cr * cpcy + sr * spsy
        
        -- Normalize
        local n = x*x + y*y + z*z + w*w
        if n ~= 1.0 then
            n = 1.0 / math.sqrt(n)
            
            x = x * n
            y = y * n
            z = z * n
            w = w * n
        end
        
        rot.x = x
        rot.y = y
        rot.z = z
        rot.w = w
    end
    
    return BoneWLD:instance(b)
end

return BoneWLD
