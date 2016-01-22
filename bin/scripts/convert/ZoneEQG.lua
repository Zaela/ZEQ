
local PFS   = require "PFS"
local TER   = require "TER"
local MOD   = require "MOD"

local pcall = pcall

local ZoneEQG = {}

function ZoneEQG.convert(shortname)
    local obj
    
    local s, err = pcall(function()
        local pfs       = PFS(shortname .. ".eqg")
        local data, len = pfs:getEntryByExtension("ter")
        
        if not data then return end
        
        local time = os.clock()
        io.write("Loading raw zone data from EQG... ")
        io.flush()
        
        local ter = TER(pfs, data, len)
        
        obj = {
            zone = ter:model(),
        }
        
        io.write("done in ", os.clock() - time, " seconds\n")
    end)
    
    if not s then
        io.write(err, "\n")
    end
    
    return obj
end

return ZoneEQG
