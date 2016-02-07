
local Struct = require "Struct"

local Geo = {
    Vertex = Struct.packed[[
        int16_t x, y, z;
    ]],
    
    UV16 = Struct.packed[[
        int16_t u, v;
    ]],
    
    UV32 = Struct.packed[[
        float u, v;
    ]],
    
    Normal = Struct.packed[[
        int8_t i, j, k;
    ]],
    
    Color = Struct.packed[[
        uint8_t r, g, b, a;
    ]],
    
    Triangle = Struct.packed[[
        uint16_t flag;
        uint16_t index[3];
    ]],
    
    TextureEntry = Struct.packed[[
        uint16_t count;
        uint16_t index;
    ]],
    
    TRIANGLE_PERMEABLE = 0x10,
}

Geo.BoneAssignment = Geo.TextureEntry

return Geo
