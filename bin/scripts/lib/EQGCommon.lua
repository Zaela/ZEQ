
local ffi           = require "ffi"
local bit           = require "bit"
local Struct        = require "Struct"
local Class         = require "Class"
local BinUtil       = require "BinUtil"
local MaterialEQG   = require "MaterialEQG"
local VertexBuffer  = require "VertexBuffer"
local Matrix        = require "Matrix"
local PFS           = require "PFS"
local EQGProperty   = require "EQGProperty"
local ModelEQG      = require "ModelEQG"
local BoneEQG       = require "BoneEQG"
local SkeletonEQG   = require "SkeletonEQG"
local BAEQG         = require "BoneAssignmentEQG"
local ANI -- must load later, mutual requires

local table = table

local EQGMaterial = Struct.packed[[
    uint32_t    index;          // Essentially meaningless
    uint32_t    nameIndex;      // Index of the material's name in the file's string block
    uint32_t    shaderIndex;    // Index of the name of the shader to use for this material in the file's string block
    uint32_t    propertyCount;  // Number of EQGProperty elements following this material
    EQGProperty properties[0];  // Properties array accessor
]]

local EQGVertex = Struct.packed[[
    float x, y, z;  // Position
    float i, j, k;  // Normal
    float u, v;     // Texture coordinates
]]

local EQGVertexV3 = Struct.packed[[
    float       x, y, z;    // Position
    float       i, j, k;    // Normal
    uint8_t     r, g, b, a;
    float       u, v;       // Texture coordinates
    float       unk_f[2];
]]

local EQGTriangle = Struct.packed[[
    uint32_t    index[3];
    int         materialIndex;
    uint32_t    flag;
]]

local EQGPos = Struct.namedPacked("EQGPos", [[
    float x, y, z;
]])

local EQGQuat = Struct.namedPacked("EQGQuat", [[
    float x, y, z, w;
]])

local EQGBone = Struct.packed[[
    uint32_t    nameIndex;
    uint32_t    linkBoneIndex;
    uint32_t    flag;
    uint32_t    childBoneIndex;
    EQGPos      pos;
    EQGQuat     rot;
    EQGPos      scale;
]]

local EQGWeight = Struct.namedPacked("EQGWeight", [[
    int     boneIndex;
    float   value;
]])

local EQGBoneAssignment = Struct.packed[[
    uint32_t    count;
    EQGWeight   weights[4];
]]

local EQGCommon = Class("EQGCommon")

function EQGCommon.new(pfs, data, len, header, headerType)
    local c = {
        _pfs                = pfs,
        _data               = data,
        _len                = len,
        _header             = header,
        _headerType         = headerType,
        _model              = ModelEQG(),
        _bonesListOrder     = {},
        _bonesRecurseOrder  = {},
        _bonesList2Recurse  = {},
    }
    
    return EQGCommon:instance(c)
end

function EQGCommon:getPFS()
    return self._pfs
end

function EQGCommon:version()
    return self._header.version
end

function EQGCommon:getStringBlockLength()
    return self._header.stringBlockLen
end

function EQGCommon:setStrings(strings)
    self._strings = strings
end

function EQGCommon:strings()
    return self._strings
end

function EQGCommon:data()
    return self._data
end

function EQGCommon:length()
    return self._len
end

function EQGCommon:header()
    return self._header
end

function EQGCommon:model()
    return self._model
end

function EQGCommon:checkLength(p)
    if p > self:length() then error "file is too short for the length of data indicated" end
end

function EQGCommon:extractStrings(p)
    local stringBlock   = BinUtil.Char:cast(self:data() + p)
    local len           = self:getStringBlockLength()
    
    p = p + len
    
    self:checkLength(p)

    -- Create a table mapping string indices to lua strings
    local strings   = {}
    local i         = 0

    while i < len do
        local str   = ffi.string(stringBlock + i)
        strings[i]  = str:lower()
        
        i = i + #str + 1 -- Need to skip null terminator
    end
    
    self:setStrings(strings)

    return p
end

function EQGCommon:extractMaterials(p)
    local strings   = self:strings()
    local data      = self:data()
    local header    = self:header()
    local model     = self:model()
    local pfs       = self:getPFS()

    for i = 1, header.materialCount do
        local binMat = EQGMaterial:cast(data + p)
        p = p + EQGMaterial:sizeof()

        self:checkLength(p)

        p = p + EQGProperty:sizeof() * binMat.propertyCount

        self:checkLength(p)
        
        local mat = MaterialEQG(strings[binMat.nameIndex])
        
        model:addMaterial(mat)
        
        for j = 0, binMat.propertyCount - 1 do
            local prop = binMat.properties[j]
            
            mat:addProperty(prop, strings, pfs)
        end
        
        for diffuse, normal in mat:textures() do
            model:addTexture(diffuse)
            if normal then
                model:addNormalMap(normal)
            end
        end
    end

    return p
end

function EQGCommon:extractVertexBuffers(p)
    local data      = self:data()
    local header    = self:header()
    local model     = self:model()
    
    local vertCount = header.vertexCount
    local triCount  = header.triangleCount
    local verts
    
    model:initVertexBuffers(nil, true)
    
    if self:version() < 3 then
        verts = EQGVertex:cast(data + p)
        p = p + EQGVertex:sizeof() * vertCount
    else
        verts = EQGVertexV3:cast(data + p)
        p = p + EQGVertexV3:sizeof() * vertCount
    end
    
    local tris = EQGTriangle:cast(data + p)
    p = p + EQGTriangle:sizeof() * triCount
    
    self:checkLength(p)
    
    self._srcTris = tris
    
    for i = 0, triCount - 1 do
        local tri   = tris[i]
        local index = tri.materialIndex + 1
        
        local vb, cvb   = model:getVertexBuffer(index)
        local use       = bit.band(tri.flag, 0x01) == 0 and vb or cvb
        
        --for j = 2, 0, -1 do
        for j = 0, 2 do
            local s = verts[tri.index[j]]
            local v = use:addVertex()
            
            --v.x, v.z, v.y   = s.x, s.y, s.z
            --v.i, v.k, v.j   = s.i, s.j, s.k
            v.x, v.y, v.z   = s.x, s.y, s.z
            v.i, v.j, v.k   = s.i, s.j, s.k
            v.u, v.v        = s.u, s.v
        end
    end
    
    return p
end

function EQGCommon:extractBones(p)
    local data      = self:data()
    local header    = self:header()
    local strings   = self:strings()
    local model     = self:model()
    
    local binBones  = EQGBone:cast(data + p)
    p = p + EQGBone:sizeof() * header.boneCount
    
    self:checkLength(p)
    
    -- "Recurse order" is the default, but some animations use the "list order" instead.
    -- We will be converting any list order animations to recurse order so that only one ordering is needed in the DB.
    local listOrder         = self._bonesListOrder
    local recurseOrder      = self._bonesRecurseOrder
    local recurseIndices    = self._bonesList2Recurse
    
    for i = 0, header.boneCount - 1 do
        local bone = binBones[i]
        local name = strings[bone.nameIndex]
        
        table.insert(listOrder, BoneEQG(name, bone.pos, bone.rot, bone.scale))
    end
    
    local function recurse(i, parent)
        local binBone   = binBones[i]
        local bone      = listOrder[i + 1]
        
        if binBone.linkBoneIndex ~= 0xFFFFFFFF then
            recurse(binBone.linkBoneIndex, parent)
        end
        
        table.insert(recurseOrder, bone)
        table.insert(recurseIndices, i)
        
        if parent then
            parent:addChild(bone)
        end
        
        if binBone.childBoneIndex ~= 0xFFFFFFFF then
            recurse(binBone.childBoneIndex, bone)
        end
    end
    
    recurse(0)
    
    -- Check if there is any difference between list and recurse order
    local diff
    
    for i, o in ipairs(recurseIndices) do
        if i ~= (o + 1) then
            diff = true
            break
        end
    end
    
    if not diff then
        self._bonesListOrder    = nil
        self._bonesList2Recurse = nil
    end
    
    model:setSkeleton(SkeletonEQG(recurseOrder[1], #recurseOrder))

    return p
end

function EQGCommon:extractBoneAssignments(p)
    local data      = self:data()
    local header    = self:header()
    local model     = self:model()
    
    local binBAs = EQGBoneAssignment:cast(data + p)
    p = p + EQGBoneAssignment:sizeof() * header.vertexCount
    
    self:checkLength(p)
    
    local tris = self._srcTris
    
    model:initWeightBuffers()
    
    -- Need to account for triangles indexing vertices as one huge flat array while we have them separated into buffers
    local vertCountsByMat           = {}
    local noCollideVertCountsByMat  = {}
    
    for i = 1, #model:getVertexBuffers() do
        vertCountsByMat[i]          = 0
        noCollideVertCountsByMat[i] = 0
    end
        
    for i = 0, header.triangleCount - 1 do
        local tri   = tris[i]
        local index = tri.materialIndex + 1
        
        local wt, nwt   = model:getWeightBuffer(index)
        local col       = bit.band(tri.flag, 0x01) == 0
        local use       = col and wt or nwt
        local vcounts   = col and vertCountsByMat or noCollideVertCountsByMat
        local vcount    = vcounts[index]
        
        for j = 0, 2 do
            local binBA = binBAs[tri.index[j]]

            for k = 0, binBA.count - 1 do
                local wt = binBA.weights[k]
                
                use:add(vcount + j, wt.boneIndex, wt.value)
            end
        end
        
        vcounts[index] = vcount + 3
    end
    
    model:sortWeights()
        
    --[[
    local function count(iter)
        for wt in iter do
            if wt:isEmpty() then goto skip end
            
            io.write("== WT COUNT ", wt._count, " ==\n")
            
            local arr = wt._array
            for i = 0, wt._count - 1 do
                local a = arr[i]
                io.write(string.format("VERT %u BONE %u WT %g\n", a.vertIndex, a.boneIndex, a.weight))
            end
            
            ::skip::
        end
    end
    
    count(model:weightBuffers())
    count(model:noCollideWeightBuffers())
    --]]
end

function EQGCommon:extractModel(p)
    p = self:extractStrings(p)
    p = self:extractMaterials(p)
    p = self:extractVertexBuffers(p)
    
    local header = self:header()
    if self._headerType:hasField("boneCount") and header.boneCount > 0 then
        p = self:extractBones(p)
        p = self:extractBoneAssignments(p)
    end
end

return EQGCommon
