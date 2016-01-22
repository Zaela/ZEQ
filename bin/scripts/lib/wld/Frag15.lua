
local Struct        = require "Struct"
local FragHeader    = require "wld/FragHeader"

return Struct([[
    WLDFragHeader    header;
    int                refName;
    uint32_t        flag;
    int                refB;
    float            x, y, z;
    float            rotX, rotY, rotZ;
    float            scaleX, scaleY, scaleZ;
    int                refC;
    uint32_t        refCParam;
]], FragHeader)
