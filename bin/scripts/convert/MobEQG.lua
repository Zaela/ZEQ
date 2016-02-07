
local PFS = require "PFS"
local MOD = require "MOD"
local MDS = require "MDS"

local pcall = pcall

local MobEQG = {}

function MobEQG.convert(path)
    local obj
    local s, err = pcall(function()
        local time  = os.clock()
        local pfs   = PFS(path)
        
        local function wrap(func)
            io.write("Loading raw mob data from EQG... ")
            func()
            io.write("done in ", os.clock() - time, " seconds\n")
        end
        
        local data, len = pfs:getEntryByExtension("mds")
        
        if data then
            return wrap(function() obj = MobEQG.convertType(MDS, pfs, data, len) end)
        end
        
        data, len = pfs:getEntryByExtension("mod")
        
        if data then
            return wrap(function() obj = MobEQG.convertType(MOD, pfs, data, len) end)
        end
    end)
    
    return obj
end

function MobEQG.convertType(type, pfs, data, len)
    local data = type(pfs, data, len)
    return data:model()
end

return MobEQG
