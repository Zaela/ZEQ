
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

function ANI.new(pfs, name, data, len)
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
    
    p = eqg:extractStrings(p)
    
    eqg:checkLength(p)
    
    eqg._p          = p
    eqg._animName   = name
    eqg._animData   = {}
    
    return ANI:instance(eqg)
end

function ANI:boneOrdering()
    local header = self:header()
    
    if header.version <= 1 or header.recurseBoneOrder == 0 then
        return "recurse"
    end
    
    return "list"
end

function ANI:readFrames(model)
    local p         = self._p
    local header    = self:header()
    local data      = self:data()
    
    local strings   = self:strings()
    local byName    = model:skeleton():boneIndicesByName()
    local byIndex   = self._animData
    
    for i = 0, header.count - 1 do
        local frameHeader = FrameHeader:cast(data + p)
        p = p + FrameHeader:sizeof()
        
        self:checkLength(p)
        p = p + Frame:sizeof() * frameHeader.frameCount
        self:checkLength(p)
        
        local boneName  = strings[frameHeader.boneNameIndex]
        local index     = byName[boneName]:index()
        
        byIndex[index] = frameHeader
        
        -- Check for redundant frames and remove them
        --[=[
        if not xyz then
            xyz = true
            local frame = Frame:cast(data - Frame:sizeof() * frameHeader.frameCount)
            for j = 0, frameHeader.frameCount - 1 do
                local f = frame[j]
                local r, s = f.rot, f.scale
                io.write(string.format("%g,%g,%g : %g,%g,%g,%g : %g,%g,%g\n", f.x, f.y, f.z, r.x, r.y, r.z, r.w, s.x, s.y, s.z))
            end
        end
        --]=]
    end
    
    model:addAnimation(self)
end

function ANI:name()
    return self._animName
end

function ANI:dataByBoneIndex()
    return self._animData
end

ANI.FrameHeader = FrameHeader
ANI.Frame       = Frame

return ANI
