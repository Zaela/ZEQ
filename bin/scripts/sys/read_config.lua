
local File      = require "File"
local Config    = require "Config"

if not File.exists("config.lua") then
    error "Could not find config.lua, expected it to be in the same folder as the zeq executable"
end

local f, err = loadfile("config.lua")

if not f then
    error(string.format("Error reading config.lua: %s", err))
end

local mapping = ConfigKeyToIndexMapping
ConfigKeyToIndexMapping = nil

local tostring  = tostring
local rawset    = rawset
local ivalues   = {}
local kvalues   = {}

local mt = {
    __newindex = function(t, k, v)
        local k = tostring(k):lower()
        local i = mapping[k]
        
        rawset(kvalues, k, v)
        
        if i then
            rawset(ivalues, i, v) 
        end
    end,
}

setmetatable(ivalues, mt)
setfenv(f, ivalues)

f, err = pcall(f)

if not f then
    error(string.format("Error running config.lua: %s", err))
end

Config.setKeyValuePairs(kvalues)

return ivalues
