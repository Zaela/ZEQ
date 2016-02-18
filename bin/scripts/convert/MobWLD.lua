
local PFS       = require "PFS"
local WLD       = require "WLD"
local Model     = require "ModelWLD"
local Bone      = require "BoneWLD"
local Skeleton  = require "SkeletonWLD"
local Anim      = require "AnimWLD"
local Common    = require "WLDCommon"

local pcall     = pcall
local pairs     = pairs
local tonumber  = tonumber

local MobWLD = {}

function MobWLD.convert(a)
    local path, modelName = a[1], a[2]
    
    local obj
    
    local s, err = pcall(function()
        local time  = os.clock()
        local pfs   = PFS(path .. ".s3d")
        local wld   = WLD(pfs:getEntryByName(path .. ".wld"))
        
        local f14 = MobWLD.findModelFragment(wld, modelName)
        
        if not f14 or not f14:hasMeshRefs() then return end
        
        io.write("Loading raw mob data from S3D... ")
        
        local mobModel = Model(pfs, wld)
        
        obj = MobWLD.readModelData(mobModel, f14)
        
        io.write("done in ", os.clock() - time, " seconds\n")
    end)
    
    if not s then
        io.write("\n", err, "\n")
    end
    
    return obj
end

function MobWLD.findModelFragment(wld, modelName)
    modelName = modelName .. "_ACTORDEF"
    
    for i, f14 in wld:getFragsByType(0x14) do
        local name = wld:getFragName(f14)
        if name == modelName then
            return f14
        end
    end
end

function MobWLD.readModelData(model, f14)
    local pfs = model:getPFS()
    local wld = model:getWLD()
    
    -- f14 -> f11 -> f10 -> f13 -> f12
    --                  |-> f2d -> f36
    
    local f11 = wld:getFragByRef(f14:getFirstRef())
    
    if not f11 or f11:type() ~= 0x11 then return end
    
    local f10 = wld:getFragByRefVar(f11)
    
    if not f10 or f10:type() ~= 0x10 or not f10:hasRefList() then return end
    
    local boneCount = f10:boneCount()
    local rootBone  = f10:boneList()
    local binBone   = rootBone
    local bones     = {}
    
    -- Read and convert bones
    for i = 0, boneCount - 1 do
        local f13 = wld:getFragByRef(binBone.refA)
        
        if not f13 or f13:type() ~= 0x13 then error "bad skeleton data" end
        
        local f12 = wld:getFragByRefVar(f13)
        
        if not f12 or f12:type() ~= 0x12 then error "bad skeleton data" end
        
        local name = wld:getFragName(f13)
        
        local bone = Bone(name, f12.entry[0], i)
        
        bones[i]    = bone
        binBone     = binBone:next()
    end
    
    -- Read bone hierarchy
    binBone = rootBone
    for i = 0, boneCount - 1 do
        if binBone.size > 0 then
            local bone  = bones[i]
            local ptr   = binBone:indexList()
            
            for j = 0, binBone.size - 1 do
                bone:addChild(bones[ptr[j]])
            end
        end
        
        binBone = binBone:next()
    end
    
    local skele = Skeleton(bones[0], boneCount)
    model:setSkeleton(skele)
    
    -- Animations
    local animByCode    = {}
    local indexByName   = skele:boneIndicesByName()
    
    for i, f13 in wld:getFragsByType(0x13) do
        local name = wld:getFragName(f13)
        local code, bname = name:match("(...)(.+)")
        
        local index = indexByName[bname]
        
        if not index then goto skip end
        
        local anim = animByCode[code]
        if not anim then
            anim = {}
            animByCode[code] = anim
        end
        
        local f12 = wld:getFragByRefVar(f13)
        
        if not f12 or f12:type() ~= 0x12 then goto skip end
        
        anim[index] = f12
        
        ::skip::
    end
    
    for code, f12s in pairs(animByCode) do
        -- Find actual length of anim
        local count = 0
        for index, f12 in pairs(f12s) do
            if f12.count > count then
                count = f12.count
            end
        end

        local anim = Anim(code, count)
        
        for i = 0, boneCount - 1 do
            local f12 = f12s[i]
            if f12 then
                anim:addFrames(i, f12, skele:getParentIndexByBoneIndex(i))
            end
        end
        
        model:addAnimation(anim)
    end
    
    -- Models/VertexBuffers
    local baseModel     = model
    local ptr, count    = f10:refList()
    
    for i = 0, count - 1 do
        local f2d = wld:getFragByRef(ptr[i])
        
        if not f2d or f2d:type() ~= 0x2d then goto skip end
        
        local f36 = wld:getFragByRefVar(f2d)
        
        if not f36 or f36:type() ~= 0x36 then goto skip end
        
        local name = wld:getFragName(f36)
        local model
        
        local headId = name:match("...HE(%d+)")
        if headId then
            model = Model(pfs, wld)
            model:setSkeleton(baseModel:skeleton())
            baseModel:addHeadModel(model, tonumber(headId))
        else
            model = baseModel
        end
        
        Common.readAllMaterials(model)
        Common.readMesh(model, f36)
        
        ::skip::
    end
    
    return baseModel
end

return MobWLD
