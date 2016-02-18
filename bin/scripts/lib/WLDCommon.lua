
local ffi           = require "ffi"
local bit           = require "bit"
local BinUtil       = require "BinUtil"
local WLD           = require "WLD"
local Geo           = require "GeometryWLD"
local Texture       = require "Texture"
local Material      = require "MaterialWLD"
local Frag36        = require "wld/Frag36"

local table = table

local WLDCommon = {}

function WLDCommon.readAllMaterials(model)
    local wld       = model:getWLD()
    local matCount  = wld:getFragCountByType(0x30)
    
    if matCount == 0 then error "no materials" end
    
    if not wld._f03sTranslated then
        wld._f03sTranslated = true
        
        for i, f03 in wld:getFragsByType(0x03) do
            if f03.stringLen > 0 then
                WLD.encodeString(f03.string, f03.stringLen)
            end
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

function WLDCommon.readAllMeshes(model)
    local wld = model:getWLD()
    
    for i, f36 in wld:getFragsByType(0x36) do
        WLDCommon.readMesh(model, f36, true)
    end
end

function WLDCommon.readMesh(model, f36, isZone)
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
    local triBA
    local boneCount = f36.boneAssignCount
    if boneCount > 0 then
        local wldBA = Geo.BoneAssignment:cast(data + p)
        p = p + Geo.BoneAssignment:sizeof() * boneCount
        
        local vert2ba = {}
        
        for i = 0, boneCount - 1 do
            local ba = wldBA[i]
            for j = 1, ba.count do
                table.insert(vert2ba, ba.index)
            end
        end
        
        local indexMap = model:skeleton():getBoneIndexMap()
        
        function triBA(idx)
            return indexMap[vert2ba[idx + 1]]
        end
    else
        function triBA(idx)
            return 0
        end
    end
    
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

            if isZone then
                for i = 2, 0, -1 do
                --for i = 0, 2 do
                    local idx   = tri.index[i]
                    local v     = vb:addVertex()
                    
                    v.x, v.z, v.y, v.i, v.k, v.j    = triVertNorm(idx)
                    v.u, v.v                        = triUV(idx)
                    v.boneIndex                     = triBA(idx)
                end
            else
                --for i = 0, 2 do
                for i = 2, 0, -1 do
                    local idx   = tri.index[i]
                    local v     = vb:addVertex()
                    
                    v.x, v.y, v.z, v.i, v.j, v.k    = triVertNorm(idx)
                    v.u, v.v                        = triUV(idx)
                    v.boneIndex                     = triBA(idx)
                end
            end
        end

        ::skip::
        -- advance tris ptr to next set
        wldTris = wldTris + te.count
    end
end

return WLDCommon
