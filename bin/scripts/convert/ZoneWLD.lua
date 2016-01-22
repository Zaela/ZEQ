
local PFS           = require "PFS"
local WLD           = require "WLD"
local Geo           = require "GeometryWLD"
local ffi           = require "ffi"
local bit           = require "bit"
local Model         = require "ModelWLD"
local Texture       = require "Texture"
local Material      = require "MaterialWLD"
local BinUtil       = require "BinUtil"
local Frag36        = require "wld/Frag36"
local VertexBuffer  = require "VertexBuffer"
local Matrix        = require "Matrix"
local Vertex        = require "Vertex"

local table     = table
local pcall     = pcall
local tonumber  = tonumber

local ZoneWLD = {}

function ZoneWLD.convert(shortname)
    local obj
    
    local s, err = pcall(function()
        local pfs = PFS(shortname .. ".s3d")
        local wld = WLD(pfs:getEntryByName(shortname .. ".wld"))
        
        local time = os.clock()
        io.write("Loading raw zone data from S3D... ")
        io.flush()
        
        local zoneModel = Model(pfs, wld)
        
        ZoneWLD.readAllMaterials(zoneModel)
        ZoneWLD.readAllMeshes(zoneModel)
        
        local objectModelDefs = {
            byName  = {},
        }
        
        local s, err = pcall(function()
            -- Object definitions
            local defPfs = PFS(shortname .. "_obj.s3d")
            local defWld = WLD(defPfs:getEntryByName(shortname .. "_obj.wld"))
            
            -- Not a real model
            local defModel = Model(defPfs, defWld)
            
            ZoneWLD.readAllMaterials(defModel)
            ZoneWLD.readObjectDefinitions(defModel, objectModelDefs)
            
            -- Object placements
            local placeWld = WLD(pfs:getEntryByName("objects.wld"))
            
            ZoneWLD.readObjectPlacements(defModel, objectModelDefs, placeWld)
            
            zoneModel:append(defModel)
        end)
    
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

function ZoneWLD.readAllMaterials(model)
    local wld       = model:getWLD()
    local matCount  = wld:getFragCountByType(0x30)
    
    if matCount == 0 then error "no materials" end
    
    for i, f03 in wld:getFragsByType(0x03) do
        if f03.stringLen > 0 then
            WLD.encodeString(f03.string, f03.stringLen)
        end
    end

    model:initVertexBuffers(matCount)
    
    local function getTexture(name, isMasked)
        name = name:lower()
        local tex = model:getTextureByName(name, isMasked)
        if tex then return tex end
        local pfs = model:getPFS()
        
        local data, len = pfs:getEntryByName(name)
        if not data then return end
        
        tex = Texture(name, data, len)
        model:addTexture(tex, isMasked)
        return tex
    end
    
    local function setMaterial(mat, vbi)
        local vb, cvb = model:getVertexBuffer(vbi)
        vb:setMaterial(mat)
        cvb:setMaterial(mat)
    end
    
    local function frag03toMaterial(f03, f30, vbi)
        local visibility = f30.visibilityFlag
        
        if visibility == 0 or f03 == nil or f03.stringLen == 0 then
            return setMaterial(Material.NULL, vbi)
        end
        
        local matName   = wld:getFragName(f30)
        local material  = Material(matName)
        
        material:setVisibility(visibility)
        
        --[[if material:isInvisible() then
            io.write("Material ", matName, " is INVISIBLE\n")
        end]]
        
        local texName = ffi.string(f03.string, f03.stringLen - 1)
        local texture = getTexture(texName, material:isMasked())
        
        if not texture then return setMaterial(Material.NULL, vbi) end
        
        material:addTexture(texture)
        
        setMaterial(material, vbi)
        model:addMaterial(material)
    end
    
    local function handleAnimated(f04, f30, vbi)
        f04 = f04:toAnimated()
        
        local visibility = f30.visibilityFlag
        
        if visibility == 0 then
            return setMaterial(Material.NULL, vbi)
        end
        
        local matName   = wld:getFragName(f30)
        local material  = Material(matName)
        
        material:setVisibility(visibility)
        material:setAnimDelay(f04.milliseconds)
        
        for i = 0, f04.count - 1 do
            local f03 = wld:getFragByRef(f04.refList[i])
            
            if f03 == nil or f03.stringLen == 0 then return vb:setMaterial(Material.NULL) end
            
            local texName = ffi.string(f03.string, f03.stringLen - 1)
            local texture = getTexture(texName, material:isMasked())
            
            if not texture then return vb:setMaterial(Material.NULL) end
            
            material:addTexture(texture)
        end
        
        setMaterial(material, vbi)
        model:addMaterial(material)
    end
    
    for i, f30 in wld:getFragsByType(0x30) do
        model:addMaterialIndexByF30(BinUtil.toAddress(f30), i)
        
        local f03
        if f30.ref > 0 then
            local f05 = wld:getFragByRefVar(f30)
            if not f05 then goto skip end
            local f04 = wld:getFragByRefVar(f05)
            if not f04 then goto skip end

            if not f04:isAnimated() then
                f03 = wld:getFragByRefVar(f04)
            else
                handleAnimated(f04, f30, i)
                goto skip
            end
        else
            f03 = wld:getFragByRefVar(f30)
        end

        frag03toMaterial(f03, f30, i)

        ::skip::
    end
end

function ZoneWLD.readAllMeshes(model)
    local wld = model:getWLD()
    
    for i, f36 in wld:getFragsByType(0x36) do
        ZoneWLD.readMesh(model, f36)
    end
end

function ZoneWLD.readMesh(model, f36)
    local wld   = model:getWLD()
    local len   = f36:length()
    local data  = BinUtil.Byte:cast(f36)
    local p     = Frag36:sizeof()
    
    local function checkTooShort()
        if p > len then error("Frag36 is too short for length of data indicated") end
    end
    
    checkTooShort()
    
    local scale     = 1.0 / bit.lshift(1, f36.scale)
    local normScale = 1.0 / 127.0
    
    -- vertices
    local wldVerts = Geo.Vertex:cast(data + p)
    p = p + Geo.Vertex:sizeof() * f36.vertCount
    
    checkTooShort()
    
    -- texture coords
    local uv16, uv32
    if f36.uvCount > 0 then
        if wld:getVersion() == 1 then
            uv16 = Geo.UV16:cast(data + p)
            p = p + Geo.UV16:sizeof() * f36.uvCount
        else
            uv32 = Geo.UV32:cast(data + p)
            p = p + Geo.UV32:sizeof() * f36.uvCount
        end
    end

    checkTooShort()
    
    -- normals
    local wldNorms = Geo.Normal:cast(data + p)
    p = p + Geo.Normal:sizeof() * f36.vertCount

    checkTooShort()

    -- vertex colors
    local wldColors = Geo.Color:cast(data + p)
    p = p + Geo.Color:sizeof() * f36.colorCount

    checkTooShort()

    -- triangles
    local wldTris = Geo.Triangle:cast(data + p)
    p = p + Geo.Triangle:sizeof() * f36.polyCount

    checkTooShort()

    -- bone assignments
    -- fill in later
    
    local matIndicesByF30   = model:getMaterialIndicesByF30()
    local matIndices        = {}
    local f31               = wld:getFragByRef(f36.materialListRef)
    
    for i = 0, f31.refCount - 1 do
        local f30       = wld:getFragByRef(f31.refList[i])
        matIndices[i]   = matIndicesByF30[BinUtil.toAddress(f30)]
    end
    
    local function triVertNorm(idx)
        local vert = wldVerts[idx]
        local norm = wldNorms[idx]
        return
            f36.x + tonumber(vert.x) * scale,
            f36.y + tonumber(vert.y) * scale,
            f36.z + tonumber(vert.z) * scale,
            tonumber(norm.i) * normScale,
            tonumber(norm.j) * normScale,
            tonumber(norm.k) * normScale
    end

    local triUV
    if uv16 then
        local uvScale = 1.0 / 256.0
        function triUV(idx)
            local uv = uv16[idx]
            return
                tonumber(uv.u) * uvScale,
                tonumber(-uv.v) * uvScale
        end
    elseif uv32 then
        function triUV(idx)
            local uv = uv32[idx]
            return uv.u, uv.v
        end
    else
        function triUV() return 0, 0 end
    end
    
    local function triColor(idx)
        local c = wldColors[idx]
        return c.r, c.b, c.g, c.a
    end
    
    local nvb, cvb
    
    -- construct vertices and triangles based on their materials
    for m = 1, f36.polyTextureCount do
        local te = Geo.TextureEntry:cast(data + p)
        p = p + Geo.TextureEntry:sizeof()

        checkTooShort()

        local matIndex = matIndices[te.index]
        -- hohonora seems to have an index that is way out of range
        if not matIndex then goto skip end

        nvb, cvb = model:getVertexBuffer(matIndex)

        for i = 0, te.count - 1 do
            local tri   = wldTris[i]
            local vb    = bit.band(tri.flag, Geo.TRIANGLE_PERMEABLE) == 0 and nvb or cvb

            for i = 0, 2 do
                local idx   = tri.index[i]
                local v     = vb:addVertex()
                
                v.x, v.z, v.y, v.i, v.k, v.j    = triVertNorm(idx)
                v.u, v.v                        = triUV(idx)
                --v.r, v.b, v.g, v.a              = triColor(idx)
            end
        end

        ::skip::
        -- advance tris ptr to next set
        wldTris = wldTris + te.count
    end
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
            ZoneWLD.readMesh(defModel, wld:getFragByRefVar(f2d))
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
