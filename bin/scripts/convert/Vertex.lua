
local Struct = require "Struct"

return Struct[[
    float   x, y, z;
    float   i, j, k;
    float   u, v;
    union
    {
        bool        moved;      // Used by EQG skeletons
        uint16_t    boneIndex;  // Used by WLD skeletons
    };
]]
