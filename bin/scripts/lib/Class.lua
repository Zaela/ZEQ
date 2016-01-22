
local Util = require "Util"

local setmetatable = setmetatable

local function call(t, ...)
    return t.new(...)
end

local mt = {__call = call} -- needed to avoid Class -> Class index lookup loop
mt.__index = mt
local Class = setmetatable({__call = call}, mt)
Class.__index = Class

function Class.new(name, super)
    local is = "is" .. name
    Class[is] = Util.falseFunc

    local newclass = {
        __call    = call,
        [is]    = Util.trueFunc,
    }

    newclass.__index = newclass
    Class[name] = newclass
    return setmetatable(newclass, super or Class)
end

function Class.instance(class, tbl)
    return setmetatable(tbl or {}, class)
end

return Class
