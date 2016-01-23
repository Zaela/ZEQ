
local PFS       = require "PFS"
local TER       = require "TER"
local MOD       = require "MOD"
local ZON       = require "ZON"
local File      = require "File"
local Config    = require "Config"

local pcall = pcall

local ZoneEQG = {}

function ZoneEQG.convert(shortname)
    local obj
    
    local s, err = pcall(function()
        local pfs       = PFS(shortname .. ".eqg")
        local data, len = pfs:getEntryByExtension("zon")
        
        if data == nil then
            local path = Config.getEQPath() .."/".. shortname .. ".zon"
            if File.exists(path) then
                data, len = File.openBinary(path)
            else
                return
            end
        end
        
        local time = os.clock()
        io.write("Loading raw zone data from EQG... ")
        io.flush()
        
        local zon = ZON(pfs, data, len)
        
        obj = {
            zone = zon:getTER():model(),
        }
        
        io.write("done in ", os.clock() - time, " seconds\n")
    end)
    
    if not s then
        io.write(err, "\n")
    end
    
    return obj
end

return ZoneEQG
