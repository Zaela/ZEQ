
local BlendTypes    = require "BlendTypes"
local ConvMaterial  = require "ConvMaterial"
local Class         = require "Class"
local Texture       = require "Texture"

local table = table

local MaterialEQG = Class("MaterialEQG", ConvMaterial)

function MaterialEQG.new(name)
    local mat = ConvMaterial(name)
    
    mat._normalMaps = {}
    
    return MaterialEQG:instance(mat)
end

function MaterialEQG:addNormalMap(tex)
    table.insert(self._normalMaps, tex)
end

local function makeTextureHandler(func)
    return function(self, prop, strings, pfs)
        local name      = prop:getValue(strings):lower()
        local data, len = pfs:getEntryByName(name)
        local tex
        
        if not data then
            io.write("Could not find texture '", name, "'\n")
            tex = Texture.NULL
        else
            tex = Texture(name, data, len)
        end
        
        func(self, tex)
    end
end

local propHandlers = {
    e_TextureDiffuse0   = makeTextureHandler(MaterialEQG.addTexture),
    e_TextureNormal0    = makeTextureHandler(MaterialEQG.addNormalMap),
}

function MaterialEQG:addProperty(prop, strings, pfs)
    local name      = prop:getName(strings)
    local handler   = propHandlers[name]
    
    if not handler then
        io.write("Unhandled EQG Property '", name, "'\n")
        return
    end
    
    handler(self, prop, strings, pfs)
end

function MaterialEQG:textures()
    local i = 0
    local a = self:getTextures()
    local b = self._normalMaps
    return function()
        i = i + 1
        return a[i], b[i]
    end
end

return MaterialEQG
