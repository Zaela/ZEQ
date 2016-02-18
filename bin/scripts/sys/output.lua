
local ffi = require "ffi"

ffi.cdef[[
void Log_print(const char*, int);
]]

local C = ffi.C

io.write = function(...)
    local str = table.concat{...}
    C.Log_print(str, #str + 1)
end
