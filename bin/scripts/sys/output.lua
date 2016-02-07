
local ffi = require "ffi"

ffi.cdef[[
void Log_print(const char*);
]]

local C = ffi.C

io.write = function(...)
    local str = table.concat{...}
    C.Log_print(str)
end
