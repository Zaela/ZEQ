
local BlendTypes    = require "BlendTypes"
local ConvMaterial  = require "ConvMaterial"
local Class         = require "Class"

local table = table

local MaterialWLD = Class("MaterialWLD", ConvMaterial)

local blendMapping = {
    [0x00000000] = BlendTypes.Invisible,
    [0x00000553] = BlendTypes.Masked,
    [0x80000001] = BlendTypes.Solid,
    [0x80000005] = BlendTypes.Additive,
    [0x80000009] = BlendTypes.Particle,
    [0x8000000a] = BlendTypes.Additive,
    [0x8000000b] = BlendTypes.Particle,
    [0x80000013] = BlendTypes.Masked,
    [0x80000017] = BlendTypes.Particle,
}

function MaterialWLD.new(name)
    local mat = ConvMaterial(name)
    return MaterialWLD:instance(mat)
end

function MaterialWLD:setVisibility(flag)
    local bt = blendMapping[flag]
    
    if not bt then
        io.write(string.format("Warning: No MaterialWLD mapping for visibility flag %8x (material: %s)\n", flag, self:getName()))
    elseif bt ~= BlendTypes.Solid and bt ~= BlendTypes.Masked then
        io.write(string.format("Material flag %8x (%s)\n", flag, self:getName()))
    end
    
    self._blendType = bt or BlendTypes.Solid
end

function MaterialWLD:checkZoneSpecificFixes(shortname)

end

return MaterialWLD
