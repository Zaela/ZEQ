
local Struct        = require "Struct"
local Class         = require "Class"
local EQGCommon     = require "EQGCommon"
local BinUtil       = require "BinUtil"
local ModelEQG      = require "ModelEQG"

local tonumber = tonumber

local Header = Struct[[
    uint32_t    signature;      // "EQGS"
    uint32_t    version;
    uint32_t    stringBlockLen;
    uint32_t    materialCount;
    uint32_t    boneCount;
    uint32_t    modelCount;
]]

local ModelHeader = Struct[[
    uint32_t    mainPiece;      // 1 = yes, 0 = no; yes applies to both the main body and default "head"
    uint32_t    nameIndex;
    uint32_t    vertexCount;
    uint32_t    triangleCount;
    uint32_t    boneAssignmentCount;
]]

local Signature = BinUtil.toFileSignature("EQGS")

local MDS = Class("MDS", EQGCommon)

function MDS.new(pfs, data, len)
    local p = Header:sizeof()
    
    if len < p then
        error "file is too short for length of data indicated"
    end

    local header = Header:cast(data)
    
    if header.signature ~= Signature then
        error "file does not have a valid MDS signature"
    end

    local eqg = EQGCommon(pfs, data, len, header, Header)
    
    p = eqg:extractStrings(p)
    p = eqg:extractMaterials(p)
    p = eqg:extractBones(p)
    
    local model     = eqg:model()
    local materials = model:getAllMaterials()
    local skele     = model:skeleton()
    local strings   = eqg:strings()
    local version   = header.version
    local heads     = {}
    local baseModel
    
    local function newModel(sub)
        local e = EQGCommon(pfs, data, len, sub, ModelHeader)
        e:setVersion(version)
        local m = e:model()
        m:setAllMaterials(materials)
        m:inheritTextures(model)
        m:setSkeleton(skele)
        return e
    end
    
    for i = 1, header.modelCount do
        local sub   = ModelHeader:cast(data + p)
        local name  = strings[sub.nameIndex]
        
        p = p + ModelHeader:sizeof()
        eqg:checkLength(p)
        
        local model     = newModel(sub)
        local headIndex = name:match("^...he(%d+)")
        
        if headIndex then
            heads[tonumber(headIndex)] = model
        else
            baseModel = model
        end
        
        p = model:extractVertexBuffers(p)
        p = model:extractBoneAssignments(p)
    end
    
    model = baseModel:model()
    eqg:setModel(model)
    
    for i, head in pairs(heads) do
        model:addHeadModel(head:model(), i)
    end
    
    eqg:extractAnimations()
    
    return MDS:instance(eqg)
end

return MDS
