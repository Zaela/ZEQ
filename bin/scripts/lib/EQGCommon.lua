
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

local EQGMaterial = Struct.packed[[
    uint32_t    index;			// Essentially meaningless
    uint32_t    nameIndex;		// Index of the material's name in the file's string block
    uint32_t    shaderIndex;	// Index of the name of the shader to use for this material in the file's string block
    uint32_t    propertyCount;	// Number of EQGProperty elements following this material
    EQGProperty properties[0];	// Properties array accessor
]]

local EQGVertex = Struct.packed[[
    float x, y, z;  // Position
    float i, j, k;  // Normal
    float u, v;     // Texture coordinates
]]

local EQGVertexV3 = Struct.packed[[
    float       x, y, z;    // Position
    float       i, j, k;    // Normal
    uint32_t    unk_i;
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

function EQGCommon.new(pfs, data, len, header)
    local c = {
        _pfs    = pfs,
        _data   = data,
        _len    = len,
        _header = header,
        _model  = ModelEQG(),
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
		strings[i]  = str
        
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
            model:addNormalMap(normal)
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
    
    for i = 0, triCount - 1 do
        local tri   = tris[i]
        local index = tri.materialIndex + 1
        
        local vb, cvb   = model:getVertexBuffer(index)
        local use       = bit.band(tri.flag, 0x01) == 0 and vb or cvb
        
        for j = 2, 0, -1 do
            local s = verts[tri.index[j]]
            local v = use:addVertex()
            
            v.x, v.z, v.y   = s.x, s.y, s.z
            v.i, v.k, v.j   = s.i, s.j, s.k
            v.u, v.v        = s.u, s.v
        end
    end
    
    return p
end

function EQGCommon:extractModel(p)
    p = self:extractStrings(p)
    p = self:extractMaterials(p)
    p = self:extractVertexBuffers(p)
end

return EQGCommon
