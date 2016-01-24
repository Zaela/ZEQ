
local PFS       = require "PFS"
local MOD       = require "MOD"
local Config    = require "Config"

local pcall = pcall

local MobEQG = {}

function MobEQG.convert(path)
    local pfs = PFS(path)
    
    local data, len = pfs:getEntryByExtension("mds")
    
    if data then
        return MobEQG.convertMDS(pfs, data, len)
    end
    
    data, len = pfs:getEntryByExtension("mod")
    
    if data then
        return MobEQG.convertMOD(pfs, data, len)
    end
end

function MobEQG.convertMOD(pfs, data, len)
    local mod = MOD(pfs, data, len)
end

function MobEQG.convertMDS(pfs, data, len)

end

return MobEQG
