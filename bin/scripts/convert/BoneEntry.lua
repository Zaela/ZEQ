
local Struct = require "Struct"

return Struct.packed[[
    uint32_t    childCount;
    float       x, y, z;
    struct { float x, y, z, w; } rot;
    struct { float x, y, z; } scale;
]]
