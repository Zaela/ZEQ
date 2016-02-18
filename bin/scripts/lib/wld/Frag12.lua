
local Struct        = require "Struct"
local FragHeader    = require "wld/FragHeader"

Struct.named("WLDFrag12Entry", [[
    int16_t    rotW;
    int16_t    rotX, rotY, rotZ;
    int16_t    shiftX, shiftY, shiftZ;
    int16_t    shiftDenom;
]])

return Struct([[
    WLDFragHeader   header;
    uint32_t        flag;
    uint32_t        count;
    WLDFrag12Entry  entry[0];
]], FragHeader)
