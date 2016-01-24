
local ffi           = require "ffi"
local Struct        = require "Struct"
local Class         = require "Class"
local EQGCommon     = require "EQGCommon"
local BinUtil       = require "BinUtil"
local Vertex        = require "Vertex"
local VertexBuffer  = require "VertexBuffer"
local Matrix        = require "Matrix"

local Header = Struct[[
    uint32_t    signature;        // "EQGM"
    uint32_t    version;
    uint32_t    stringBlockLen;
    uint32_t    materialCount;
    uint32_t    vertexCount;
    uint32_t    triangleCount;
    uint32_t    boneCount;
]]

local Signature = BinUtil.toFileSignature("EQGM")

local MOD = Class("MOD", EQGCommon)

function MOD.new(pfs, data, len)
    local p = Header:sizeof()
    
    if len < p then
        error "file is too short for length of data indicated"
    end

    local header = Header:cast(data)
    
    if header.signature ~= Signature then
        error "file does not have a valid MOD signature"
    end

    local eqg = EQGCommon(pfs, data, len, header, Header)
    
    eqg:extractModel(p)
    
    return MOD:instance(eqg)
end

function MOD:staticGeometry(obj, s)
    -- this is definitely wrong
    local mat4 = Matrix.angleXYZ(obj.rot.x, -obj.rot.z, -obj.rot.y)
    mat4:setTranslation(-obj.y, obj.z, -obj.x)
    
    local scale = obj.scale
    if scale ~= 1.0 and scale ~= 0.0 then
        mat4 = mat4 * Matrix.scale(scale)
    end

    local temp = Vertex()
    
    local function shiftVBs(iter, targets)
        for src in iter do
            if src:isEmpty() then goto skip end
            
            local mat = src:getMaterial()
            local name
            
            if mat:getName() == "NULL" or mat:getTextureCount() == 0 or not mat:getTextures()[1] then
                name = "NULL"
            else
                name = mat:getTextures()[1]:getName()
            end
            
            local vb = targets[name]
            if not vb then
                vb = VertexBuffer()
                vb:setMaterial(mat)
                targets[name] = vb
            end
            
            local data = src:data()
            
            for i = 0, src:count() - 1 do
                ffi.copy(temp, data + i, Vertex:sizeof())
                mat4:transformVector(temp)
                vb:addVertexCopy(temp)
            end
            
            ::skip::
        end
    end
    
    local model = self:model()
    
    shiftVBs(model:vertexBuffers(), s.vertexBuffersByTexName)
    shiftVBs(model:noCollideVertexBuffers(), s.noCollideVertexBuffersByTexName)
end

return MOD
