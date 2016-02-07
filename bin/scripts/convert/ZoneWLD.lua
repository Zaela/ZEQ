
local PFS           = require "PFS"
local WLD           = require "WLD"
local ffi           = require "ffi"
local Model         = require "ModelWLD"
local BinUtil       = require "BinUtil"
local Matrix        = require "Matrix"
local Vertex        = require "Vertex"
local VertexBuffer  = require "VertexBuffer"
local WLDCommon     = require "WLDCommon"

local table     = table
local pcall     = pcall
local tonumber  = tonumber

local ZoneWLD = {}

function ZoneWLD.convert(shortname)
    local obj
    
    local s, err = pcall(function()
        local time  = os.clock()
        local pfs   = PFS(shortname .. ".s3d")
        local wld   = WLD(pfs:getEntryByName(shortname .. ".wld"))
        
        io.write("Loading raw zone data from S3D... ")
        
        local zoneModel = Model(pfs, wld)
        
        WLDCommon.readAllMaterials(zoneModel)
        WLDCommon.readAllMeshes(zoneModel)
        
        local objectModelDefs = {
            byName  = {},
        }
        
        local s, err = pcall(function()
            -- Object definitions
            local defPfs = PFS(shortname .. "_obj.s3d")
            local defWld = WLD(defPfs:getEntryByName(shortname .. "_obj.wld"))
            
            -- Not a real model
            local defModel = Model(defPfs, defWld)
            
            WLDCommon.readAllMaterials(defModel)
            ZoneWLD.readObjectDefinitions(defModel, objectModelDefs)
            
            -- Object placements
            local placeWld = WLD(pfs:getEntryByName("objects.wld"))
            
            ZoneWLD.readObjectPlacements(defModel, objectModelDefs, placeWld)
            
            zoneModel:append(defModel)
        end)
        
        if not s then
            io.write("\n", err, "\n")
        end
    
        obj = {
            zone = zoneModel,
        }
        
        io.write("done in ", os.clock() - time, " seconds\n")
    end)
    
    if not s then
        --remove later
        io.write("\n", err, "\n")
    end
    
    return obj
end

function ZoneWLD.readObjectDefinitions(defModel, objectModelDefs)
    local wld       = defModel:getWLD()
    local pfs       = defModel:getPFS()
    local vbs       = defModel:getVertexBuffers()
    local cvb       = defModel:getNoCollideVertexBuffers()
    local byName    = objectModelDefs.byName
    
    local function takeVBs(objModel, vertexBuffers, target)
        for i = 1, #vertexBuffers do
            local vb = vertexBuffers[i]
            if vb:isEmpty() then goto skip end
            
            table.insert(target, vb)
            
            local replace = VertexBuffer()
            replace:setMaterial(vb:getMaterial())
            vertexBuffers[i] = replace
            
            ::skip::
        end
    end
    
    for i, f14 in wld:getFragsByType(0x14) do
        if not f14:hasMeshRefs() then goto skip end
        
        local modelName = wld:getFragName(f14)
        if modelName == "" then goto skip end
        
        local f2d = wld:getFragByRef(f14:getFirstRef())
        if f2d == nil then goto skip end
        
        local t = f2d:type()
        if t ~= 0x11 and t ~= 0x2d then goto skip end
        
        local objModel = Model(pfs, wld)
        
        if t == 0x11 then
            goto skip --fixme
        else
            WLDCommon.readMesh(defModel, wld:getFragByRefVar(f2d))
        end
        
        takeVBs(objModel, vbs, objModel:getVertexBuffers())
        takeVBs(objModel, cvb, objModel:getNoCollideVertexBuffers())
        
        table.insert(objectModelDefs, objModel)
        byName[modelName] = objModel
        
        ::skip::
    end
end

function ZoneWLD.readObjectPlacements(defModel, objectModelDefs, wld)
    local byName                        = objectModelDefs.byName
    local vertexBuffersByMat            = {}
    local noCollideVertexBuffersByMat   = {}
    
    defModel.vertexBuffersByMat             = vertexBuffersByMat
    defModel.noCollideVertexBuffersByMat    = noCollideVertexBuffersByMat
    defModel:resetVertexBuffers()
    
    local vbs = defModel:getVertexBuffers()
    local cvb = defModel:getNoCollideVertexBuffers()
    
    local temp = Vertex()
    
    local function applyVBs(mat4, iter, byMat, target)
        for srcVB in iter do
            local material  = srcVB:getMaterial()
            local vb        = byMat[material]
            
            if not vb then
                vb = VertexBuffer()
                vb:setMaterial(material)
                byMat[material] = vb
                table.insert(target, vb)
            end
            
            local data = srcVB:data()
            
            for i = 0, srcVB:count() - 1 do
                ffi.copy(temp, data + i, Vertex:sizeof())
                
                mat4:transformVector(temp)
                vb:addVertexCopy(temp)
            end
        end
    end

    for i, f15 in wld:getFragsByType(0x15) do
        local modelName = wld:getFragNameByRef(f15.refName)
        if modelName == "" then goto skip end
        
        local x = f15.x
        local y = f15.z
        local z = f15.y

        --local rotX = f15.rotZ / 512.0 * 360.0
        local rotY = -f15.rotX / 512.0 * 360.0
        local rotZ = f15.rotY / 512.0 * 360.0
        
        local scale = f15.scaleZ
        
        local objectModel = byName[modelName]
        if not objectModel then goto skip end
        
        local mat4 = Matrix.angleYZ(rotY, rotZ)
        mat4:setTranslation(x, y, z)
        
        if scale ~= 1.0 and scale ~= 0.0 then
            mat4 = mat4 * Matrix.scale(scale)
        end
        
        applyVBs(mat4, objectModel:vertexBuffers(), vertexBuffersByMat, vbs)
        applyVBs(mat4, objectModel:noCollideVertexBuffers(), noCollideVertexBuffersByMat, cvb)
        
        ::skip::
    end
end

return ZoneWLD
