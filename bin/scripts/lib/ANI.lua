
local Struct    = require "Struct"
local Class     = require "Class"
local EQGCommon = require "EQGCommon"
local BinUtil   = require "BinUtil"

local Header = Struct.packed[[
    uint32_t signature;
    uint32_t version;
    uint32_t stringBlockLen;
    uint32_t count;
]]

local HeaderV2 = Struct.packed[[
    uint32_t signature;
    uint32_t version;
    uint32_t stringBlockLen;
    uint32_t count;
    uint32_t recurseBoneOrder;
]]

local FrameHeader = Struct.packed[[
    uint32_t frameCount;
    uint32_t boneNameIndex;
]]

local Frame = Struct.packed[[
    uint32_t    milliseconds;
    float       x, y, z;
    struct { float x, y, z, w; } rot;
    struct { float x, y, z; } scale;
]]

local Signature = BinUtil.toFileSignature("EQGA")

local ANI = Class("ANI", EQGCommon)

function ANI.new(pfs, data, len)
    local p = Header:sizeof()
    
    if len < p then
        error "file is too short for length of data indicated"
    end

    local header = Header:cast(data)
    
    if header.signature ~= Signature then
        error "file does not have a valid ANI signature"
    end
    
    local headerType = Header
    
    if header.version > 1 then
        header      = HeaderV2:cast(data)
        headerType  = HeaderV2
        p           = HeaderV2:sizeof()
        
        if len < p then
            error "file is too short for length of data indicated"
        end
    end
    
    local eqg = EQGCommon(pfs, data, len, header, headerType)
    
    return ANI:instance(eqg)
end

function ANI:boneOrdering()
    local header = self:header()
    
    if header.version <= 1 or header.recurseBoneOrder == 0 then
        return "recurse"
    end
    
    return "list"
end

return ANI
