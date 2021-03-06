
local Struct    = require "Struct"
local Class     = require "Class"
local EQGCommon = require "EQGCommon"
local BinUtil   = require "BinUtil"
local AnimFrame = require "AnimFrame"

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
    
    local skele     = model:skeleton()
    local strings   = self:strings()
    local byName    = skele:boneIndicesByName()
    local indexMap  = skele:getBoneIndexMap()
    local byIndex   = self._animData
    
    -- Before reading frame data, find out the actual length of this animation
    local animMs    = 0
    local x         = p
    
    for i = 0, header.count - 1 do
        local frameHeader = FrameHeader:cast(data + x)
        x = x + FrameHeader:sizeof()
        
        self:checkLength(x)
        
        local frames = Frame:cast(data + x)
        x = x + Frame:sizeof() * frameHeader.frameCount
        
        self:checkLength(x)
        
        local dur = frames[frameHeader.frameCount - 1].milliseconds
        
        if dur > animMs then
            animMs = dur
        end
    end
    
    for i = 0, header.count - 1 do
        local frameHeader = FrameHeader:cast(data + p)
        p = p + FrameHeader:sizeof()
        
        local frames = Frame:cast(data + p)
        p = p + Frame:sizeof() * frameHeader.frameCount
        
        local boneName  = strings[frameHeader.boneNameIndex]
        local index     = indexMap[byName[boneName]:index()]
        
        local animFrames    = AnimFrame()
        byIndex[index]      = animFrames

        local fr = frames[0]
        local ms = fr.milliseconds
        
        if ms > 0 then
            fr.milliseconds = 0
            animFrames:add(fr)
            fr.milliseconds = ms
        end
        
        animFrames:add(fr)
        
        -- Check for redundant frames and skip them
        if frameHeader.frameCount > 2 then
            local first = 0
            for i = 1, frameHeader.frameCount - 2 do
                local a, b, c = frames[first], frames[i], frames[i + 1]
                
                if a.x ~= b.x or a.y ~= b.y or a.z ~= b.z or
                   a.rot.x ~= b.rot.x or a.rot.y ~= b.rot.y or a.rot.z ~= b.rot.z or a.rot.w ~= b.rot.w or
                   a.scale.x ~= b.scale.x or a.scale.y ~= b.scale.y or a.scale.z ~= b.scale.z or
                   c.x ~= b.x or c.y ~= b.y or c.z ~= b.z or
                   c.rot.x ~= b.rot.x or c.rot.y ~= b.rot.y or c.rot.z ~= b.rot.z or c.rot.w ~= b.rot.w or
                   c.scale.x ~= b.scale.x or c.scale.y ~= b.scale.y or c.scale.z ~= b.scale.z then
                    animFrames:add(b)
                    first = first + 1
                end
            end
        end
        
        fr = frames[frameHeader.frameCount - 1]
        animFrames:add(fr)
        
        if fr.milliseconds < animMs then
            fr.milliseconds = animMs
            animFrames:add(fr)
        end
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
