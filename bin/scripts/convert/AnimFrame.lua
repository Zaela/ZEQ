
local Struct    = require "Struct"
local Class     = require "Class"
local Array     = require "Array"

local Frame = Struct.packed[[
    float   milliseconds;
    float   x, y, z;
    struct { float x, y, z, w; } rot;
    struct { float x, y, z; } scale;
]]

local AnimFrame = Class("AnimFrame", Array)

function AnimFrame.new()
    local af = Array(Frame)
    return AnimFrame:instance(af)
end

function AnimFrame:add(eqgFrame)
    local o = self:pushBack()
    
    o.milliseconds  = eqgFrame.milliseconds * 0.001
    o.x             = eqgFrame.x
    o.y             = eqgFrame.y
    o.z             = eqgFrame.z
    o.rot.x         = eqgFrame.rot.x
    o.rot.y         = eqgFrame.rot.y
    o.rot.z         = eqgFrame.rot.z
    o.rot.w         = eqgFrame.rot.w
    o.scale.x       = eqgFrame.scale.x
    o.scale.y       = eqgFrame.scale.y
    o.scale.z       = eqgFrame.scale.z
end

function AnimFrame:addWLD(ms, pos, rot)
    local o = self:pushBack()
    
    o.milliseconds  = ms
    o.x             = pos.x
    o.y             = pos.y
    o.z             = pos.z
    o.rot.x         = rot.x
    o.rot.y         = rot.y
    o.rot.z         = rot.z
    o.rot.w         = rot.w
    o.scale.x       = 1
    o.scale.y       = 1
    o.scale.z       = 1
end

return AnimFrame
