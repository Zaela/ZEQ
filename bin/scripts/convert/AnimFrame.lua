
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

return AnimFrame
