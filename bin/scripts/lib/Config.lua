
local Config = {}

local settings = {
    pathtoeverquest = "C:/Everquest"
}

function Config.setKeyValuePairs(tbl)
    for k, v in pairs(tbl) do
        settings[k] = v
    end
end

function Config.getEQPath()
    local path = settings.pathtoeverquest
    if not path then return "." end
    
    path = path:gsub("[/\\]+$", "")
    return path
end

return Config
