
local Struct    = require "Struct"
local Class     = require "Class"
local EQGCommon = require "EQGCommon"
local BinUtil   = require "BinUtil"

local Header = Struct[[
    uint32_t    signature;        // "EQGT"
    uint32_t    version;
    uint32_t    stringBlockLen;
    uint32_t    materialCount;
    uint32_t    vertexCount;
    uint32_t    triangleCount;
]]

local Signature = BinUtil.toFileSignature("EQGT")

local TER = Class("TER", EQGCommon)

function TER.new(pfs, data, len)
    local p = Header:sizeof()
    
    if len < p then
        error "file is too short for length of data indicated"
    end

    local header = Header:cast(data)
    
    if header.signature ~= Signature then
        error "file does not have a valid TER signature"
    end

    local eqg = EQGCommon(pfs, data, len, header)
    
    eqg:extractModel(p)
    
    return eqg
end

return TER
