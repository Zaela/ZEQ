
local ffi = require "ffi"

local tonumber = tonumber

local Util = {}

function Util.nullFunc() end
function Util.falseFunc() return false end
function Util.trueFunc() return true end

return Util
