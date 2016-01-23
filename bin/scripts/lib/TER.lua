
local Struct        = require "Struct"
local Class         = require "Class"
local EQGCommon     = require "EQGCommon"
local BinUtil       = require "BinUtil"
local Matrix        = require "Matrix"
local VertexBuffer  = require "VertexBuffer"

local Header = Struct[[
    uint32_t    signature;        // "EQGT"
    uint32_t    version;
    uint32_t    stringBlockLen;
    uint32_t    materialCount;
    uint32_t    vertexCount;
    uint32_t    triangleCount;
]]

local pairs     = pairs
local ipairs    = ipairs

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
    
    return TER:instance(eqg)
end

function TER:shift(obj)
    -- this is probably wrong
    local mat4 = Matrix.angleXYZ(obj.rot.y - 90, obj.rot.x + 90, obj.rot.z + 90)
    --local mat4 = Matrix.angleXYZ(obj.rot.x, obj.rot.y, obj.rot.z)
    --mat4:setTranslation(obj.x, obj.y, obj.z)
    --mat4:setTranslation(obj.y, obj.z, -obj.x)
    
    local scale = obj.scale
    if scale ~= 1.0 and scale ~= 0.0 then
        mat4 = mat4 * Matrix.scale(scale)
    end
    
    local function shiftVBs(iter)
        for src in iter do
            if src:isEmpty() then goto skip end
            
            local data = src:data()
            
            for i = 0, src:count() - 1 do
                mat4:transformVector(data[i])
            end
            
            ::skip::
        end
    end
    
    local model = self:model()
    
    shiftVBs(model:vertexBuffers())
    shiftVBs(model:noCollideVertexBuffers())
end

function TER:appendStaticGeometry(s)
    local model = self:model()
    
    local function handle(targetVBs, byTexName, addVB)
        local existing = {}
        
        for i, vb in ipairs(targetVBs) do
            local name
            local mat = vb:getMaterial()
            
            if mat:getName() == "NULL" or mat:getTextureCount() == 0 or not mat:getTextures()[1] then
                name = "NULL"
            else
                name = mat:getTextures()[1]:getName()
            end
            
            existing[name] = vb
        end
        
        local b = #targetVBs
        
        for name, src in pairs(byTexName) do
            local dst = existing[name]
            if not dst then
                dst = VertexBuffer()
                existing[name] = dst
                local mat = src:getMaterial()
                dst:setMaterial(mat)
                model:addMaterial(mat)
                model:addTexture(mat:getTextures()[1])
                table.insert(targetVBs, dst)
            else
                local data = src:data()
                for i = 0, src:count() - 1 do
                    dst:addVertexCopy(data + i)
                end
            end
        end
    end
    
    handle(model:getVertexBuffers(), s.vertexBuffersByTexName)
    handle(model:getNoCollideVertexBuffers(), s.noCollideVertexBuffersByTexName)
end

return TER
