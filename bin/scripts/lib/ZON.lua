
local Struct    = require "Struct"
local Class     = require "Class"
local EQGCommon = require "EQGCommon"
local BinUtil   = require "BinUtil"
local MOD       = require "MOD"
local TER       = require "TER"

local Header = Struct[[
    uint32_t    signature;        // "EQGZ"
    uint32_t    version;
    uint32_t    stringBlockLen;
    uint32_t    modelCount;
    uint32_t    objectCount;
    uint32_t    regionCount;
    uint32_t    lightCount;
]]

local Object = Struct.packed[[
    int         id;
    uint32_t    nameIndex;
    float       x, y, z;
    struct { float x, y, z; } rot;
    float       scale;
]]

local Region = Struct.packed[[
    uint32_t    nameIndex;
    struct { float x, y, z; } center;
    float       unk_f;
    uint32_t    unk_i[2];
    struct { float x, y, z; } extent;
]]

local Light = Struct.packed[[
    uint32_t    nameIndex;
    float       x, y, z;
    float       r, g, b;
    float       radius;
]]

local Signature = BinUtil.toFileSignature("EQGZ")

local ZON = Class("ZON", EQGCommon)

function ZON.new(pfs, data, len)
    local p = Header:sizeof()
    
    if len < p then
        error "file is too short for length of data indicated"
    end

    local header = Header:cast(data)
    
    if header.signature ~= Signature then
        error "file does not have a valid ZON signature"
    end

    local eqg = EQGCommon(pfs, data, len, header)
    
    p = eqg:extractStrings(p)
    
    local strings       = eqg:strings()
    local nameIndices   = BinUtil.Uint32:cast(data + p)
    p = p + BinUtil.Uint32:sizeof() * header.modelCount
    
    eqg:checkLength(p)
    
    local ter
    local modelNames    = {}
    local models        = {}
    
    for i = 0, header.modelCount - 1 do
        local name      = strings[nameIndices[i]]:lower()
        modelNames[i]   = name
        
        local model
        local data, len = pfs:getEntryByName(name)
        
        if name:find("%.ter$") then
            ter     = TER(pfs, data, len)
            model   = ter
        else
            model   = MOD(pfs, data, len)
        end
        
        models[i] = model
    end
    
    local objects = Object:cast(data + p)
    p = p + Object:sizeof() * header.objectCount
    
    eqg:checkLength(p)
    
    local staticGeometry = {
        vertexBuffersByTexName          = {},
        noCollideVertexBuffersByTexName = {},
    }
    
    for i = 0, header.objectCount - 1 do
        local obj   = objects[i]
        local index = obj.id
        local name  = modelNames[index]
        local model = models[index]
        
        obj.rot.x = math.deg(obj.rot.x)
        obj.rot.y = math.deg(obj.rot.y)
        obj.rot.z = math.deg(obj.rot.z)
        
        if model == ter then
            ter:shift(obj)
        else
            if obj.rot.y ~= 0.0 then obj.rot.y = obj.rot.y + 90 end
            if obj.rot.y ~= 0.0 or obj.rot.z ~= 0.0 then
                io.write(string.format("[%i] %s %g,%g,%g %g,%g,%g\n", i, name, obj.x, obj.y, obj.z, obj.rot.x, obj.rot.y, obj.rot.z))
            end
            model:staticGeometry(obj, staticGeometry)
        end
    end
    
    ter:appendStaticGeometry(staticGeometry)
    
    eqg._ter = ter
    
    return ZON:instance(eqg)
end

function ZON:getTER()
    return self._ter
end

return ZON
