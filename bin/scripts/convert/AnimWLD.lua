
local Class     = require "Class"
local Bone      = require "BoneWLD"
local AnimFrame = require "AnimFrame"
local Util      = require "Util"
local Matrix    = require "Matrix"

local AnimWLD = Class("AnimWLD")

function AnimWLD.new(code, count)
    local a = {
        _animData   = {},
        _count      = count,
    }
    
    return AnimWLD:instance(a)
end

function AnimWLD:dataByBoneIndex()
    return self._animData
end

function AnimWLD:count()
    return self._count
end

function AnimWLD:addFrames(index, f12)
    local frames    = AnimFrame()
    local entries   = f12.entry
    local len       = self:count()
    local count     = f12.count
    
    local function add(ent, i)
        local ms        = i * 0.1
        local pos, rot  = Bone.f12EntryToPosRot(ent)
        frames:addWLD(ms, pos, rot)
    end
    
    add(entries[0], 0)
    
    if count > 2 then
        local first = 0
        for i = 1, count - 2 do
            local a, b, c = entries[first], entries[i], entries[i + 1]
            
            if a.rotW ~= b.rotW or a.rotX ~= b.rotX or a.rotY ~= b.rotY or a.rotZ ~= b.rotZ or
               a.shiftDenom ~= b.shiftDenom or a.shiftX ~= b.shiftX or a.shiftY ~= b.shiftY or a.shiftZ ~= b.shiftZ or
               c.rotW ~= b.rotW or c.rotX ~= b.rotX or c.rotY ~= b.rotY or c.rotZ ~= b.rotZ or
               c.shiftDenom ~= b.shiftDenom or c.shiftX ~= b.shiftX or c.shiftY ~= b.shiftY or c.shiftZ ~= b.shiftZ then
                add(b, i)
                first = first + 1
            end
        end
    end
    
    if count > 1 then
        add(entries[count - 1], count - 1)
    end
    
    if count < len then
        add(entries[count - 1], len - 1)
    end
    
    self._animData[index] = frames
end

return AnimWLD
