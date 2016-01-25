
local PFS       = require "PFS"
local MOD       = require "MOD"
local Config    = require "Config"

local pcall = pcall

local MobEQG = {}

function MobEQG.convert(path)
    local pfs = PFS(path)
    
    local data, len = pfs:getEntryByExtension("mds")
    
    if data then
        return MobEQG.convertType(MDS, pfs, data, len)
    end
    
    data, len = pfs:getEntryByExtension("mod")
    
    if data then
        return MobEQG.convertType(MOD, pfs, data, len)
    end
end

function MobEQG.convertType(type, pfs, data, len)
    local data = type(pfs, data, len)
    return data:model()
end

return MobEQG
