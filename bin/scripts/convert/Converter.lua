
local ZoneEQG       = require "ZoneEQG"
local ZoneWLD       = require "ZoneWLD"
local MobEQG        = require "MobEQG"
local MobWLD        = require "MobWLD"
local Database      = require "Database"
local File          = require "File"
local Image         = require "Image"
local Texture       = require "Texture"
local ConvMaterial  = require "ConvMaterial"
local BinUtil       = require "BinUtil"

local type  = type
local table = table

local Converter = {}

function Converter.convertZone(shortname)
    local obj

    obj = ZoneEQG.convert(shortname)
    
    if not obj then
        obj = ZoneWLD.convert(shortname)
    end
    
    if obj then
        Converter.insertZone(obj, shortname)
        return
    end
    
    error(string.format("Could not find zone '%s'", shortname))
end

function Converter.convertMob(race, gender)
    local map = require "RaceMapping"
    
    local mapping = map.getFileFor(race, gender)
    
    if not mapping then
        error(string.format("No known filename for race %i gender %i\n", race, gender))
    end
    
    local obj
    
    local s, err = pcall(function()
        if type(mapping) == "string" and mapping:find("%.eqg$") then
            obj = MobEQG.convert(mapping)
        else
            obj = MobWLD.convert(mapping)
        end
    end)
    
    if not s then
        io.write(err, "\n")
    end
    
    if obj then
        Converter.insertMob(obj, race, gender)
    end
end

function Converter.initDB()
    return Database("zeq.db")
end

function Converter.insertBlob(db, q, blob, len, compress)
    local clen = len
    
    if compress and len >= 256 then
        blob, clen = Image.compress(blob, len)
    else
        len = 0
    end
    
    -- Check for binary duplicates
    local stmt = q.selectBlob
    stmt:bindInt(1, len)
    stmt:bindBlob(2, blob, clen)
    if stmt:select() then
        local id = stmt:getInt64(1)
        stmt:reset()
        return id, true
    end
    
    stmt = q.insertBlob
    stmt:bindInt(1, len)
    stmt:bindBlob(2, blob, clen)
    stmt:commit()
    
    return db:getLastInsertId()
end

function Converter.initCommonQueries(db, q)
    local stmt = db:prepare("SELECT id FROM Materials WHERE name = 'NULL'")
    if stmt:select() then
        q.nullMaterialId = stmt:getInt64(1)
        ConvMaterial.NULL:setId(q.nullMaterialId)
    end
    stmt:finalize()

    q.selectBlob = db:prepare[[
        SELECT id FROM Blobs WHERE realLength = ? AND data = ?
    ]]
    
    q.insertBlob = db:prepare[[
        INSERT INTO Blobs (realLength, data) VALUES (?, ?)
    ]]
    
    q.insertMaterial = db:prepare[[
        INSERT INTO Materials (name, blendType) VALUES (?, ?)
    ]]
    
    q.insertTexture = db:prepare[[
        INSERT INTO Textures (name, diffuseId, normalId, width, height) VALUES (?, ?, ?, ?, ?)
    ]]
    
    q.selectTextureDiffuse = db:prepare[[
        SELECT id FROM Textures WHERE diffuseId = ?
    ]]
    
    q.insertTextureSet = db:prepare[[
        INSERT INTO TextureSets (animMilliseconds) VALUES (?)
    ]]
    
    q.insertSet2Texture = db:prepare[[
        INSERT INTO Sets2Textures (setId, texId) VALUES (?, ?)
    ]]
    
    q.insertVertices = db:prepare[[
        INSERT INTO Vertices (blobId, noCollision) VALUES (?, ?)
    ]]
    
    q.insertGeometry = db:prepare[[
        INSERT INTO Geometry (vertId, matId, texSetId) VALUES (?, ?, ?) 
    ]]
    
    q.insertModel = db:prepare[[
        INSERT INTO Models DEFAULT VALUES
    ]]
    
    q.insertModels2Textures = db:prepare[[
        INSERT OR IGNORE INTO Models2Textures (modelId, texId) VALUES (?, ?)
    ]]
    
    q.insertModels2TextureSets = db:prepare[[
        INSERT OR IGNORE INTO Models2TextureSets (modelId, setId) VALUES (?, ?)
    ]]
    
    q.insertModels2Materials = db:prepare[[
        INSERT OR IGNORE INTO Models2Materials (modelId, matId) VALUES (?, ?)
    ]]
    
    q.insertModels2Vertices = db:prepare[[
        INSERT OR IGNORE INTO Models2Vertices (modelId, vertId) VALUES (?, ?)
    ]]
    
    q.insertModels2Geometry = db:prepare[[
        INSERT OR IGNORE INTO Models2Geometry (modelId, geoId) VALUES (?, ?)
    ]]
end

function Converter.initAnimQueries(db, q)
    q.insertSkeletons = db:prepare[[
        INSERT INTO Skeletons (modelId, blobId) VALUES (?, ?)
    ]]
    
    q.insertAnimationFrames = db:prepare[[
        INSERT INTO AnimationFrames (modelId, boneIndex, animType, blobId) VALUES (?, ?, ?, ?)
    ]]
    
    q.insertBoneAssignments = db:prepare[[
        INSERT INTO BoneAssignments (vertId, blobId, isWeighted) VALUES (?, ?, ?)
    ]]
end

function Converter.initZoneQueries(db, q)
    Converter.initCommonQueries(db, q)
    
    q.insertZoneModel = db:prepare[[
        INSERT INTO ZoneModels (shortname, modelId) VALUES (?, ?)
    ]]
end

function Converter.initMobQueries(db, q)
    Converter.initCommonQueries(db, q)
    Converter.initAnimQueries(db, q)
    
    q.insertMobModel = db:prepare[[
        INSERT INTO MobModels (race, gender, modelId) VALUES (?, ?, ?)
    ]]
    
    q.insertMobHeadModel = db:prepare[[
        INSERT INTO MobHeadModels (mainModelId, headModelId, headNumber) VALUES (?, ?, ?)
    ]]
end

function Converter.wrapInsert(typeName, func)
    local db    = Converter.initDB()
    local q     = {}
    
    io.write("Inserting ", typeName, " data into local database... ")
    
    local time = os.clock()
    
    func(db, q)
    
    db:analyze()
    
    q.nullMaterialId = nil
    for k, query in pairs(q) do
        query:finalize()
    end
    
    q   = nil
    db  = nil
    
    collectgarbage()
    
    io.write("done in ", os.clock() - time, " seconds\n")
end

function Converter.insertZone(obj, shortname)
    Converter.wrapInsert("zone", function(db, q)
        Converter.initZoneQueries(db, q)
        
        local id    = Converter.insertStaticModel(db, q, obj.zone)
        local stmt  = q.insertZoneModel
        
        stmt:bindString(1, shortname)
        stmt:bindInt64(2, id)
        stmt:commit()
    end)
end

function Converter.insertMob(obj, race, gender)
    Converter.wrapInsert("mob", function(db, q)
        Converter.initMobQueries(db, q)
    
        local id    = Converter.insertAnimatedModel(db, q, obj)
        local stmt  = q.insertMobModel
        
        stmt:bindInt(1, race)
        stmt:bindInt(2, gender)
        stmt:bindInt64(3, id)
        stmt:commit()
    end)
end

function Converter.insertAnimatedModel(db, q, model)
    local modelId = Converter.insertStaticModel(db, q, model)
    local stmt
    
    local skele = model:skeleton()
    if skele then
        local function handleBAs(iter)
            for wt in iter do
                if wt:isEmpty() then goto skip end
                
                local vb = wt:vertexBuffer()
                if not vb or not vb:getId() then goto skip end
                
                local blobId = Converter.insertBlob(db, q, wt:data(), wt:bytes(), true)
                
                stmt:bindInt64(1, vb:getId())
                stmt:bindInt64(2, blobId)
                stmt:bindInt(3, wt:isWeighted() and 1 or 0)
                stmt:commit()
                
                ::skip::
            end
        end
            
        local function insertBoneAssignments(model)
            if not model:hasSeparateBoneAssignments() then return end
            
            stmt = q.insertBoneAssignments

            handleBAs(model:weightBuffers())
            handleBAs(model:noCollideWeightBuffers())
        end
        
        local function insertHeadModel(head, num)
            local headId = Converter.insertStaticModel(db, q, head)
            
            db:transaction(function()
                insertBoneAssignments(head)
                
                stmt = q.insertMobHeadModel
                
                stmt:bindInt64(1, modelId)
                stmt:bindInt64(2, headId)
                stmt:bindInt(3, num)
                stmt:commit()
            end)
        end
        
        db:transaction(function()
            stmt = q.insertSkeletons
            
            local blobId = Converter.insertBlob(db, q, skele:data(), skele:bytes(), true)

            stmt:bindInt64(1, modelId)
            stmt:bindInt64(2, blobId)
            stmt:commit()
            
            -- Insert bone assignments
            insertBoneAssignments(model)
            
            -- Insert animation frames
            stmt = q.insertAnimationFrames
            
            --modelId, boneIndex, animType, blobId
            local animType = 1
            for ani in model:animations() do
                local byIndex = ani:dataByBoneIndex()
            
                stmt:bindInt64(1, modelId)
                stmt:bindInt(3, animType) --fixme
                animType = animType + 1
                
                local FrameHeader   = ani.FrameHeader
                local Frame         = ani.Frame
                
                for index, frames in pairs(byIndex) do
                    local blobId = Converter.insertBlob(db, q, frames:data(), frames:bytes(), true)
                    
                    stmt:bindInt(2, index)
                    stmt:bindInt64(4, blobId)
                    stmt:commit()
                end
            end
        end)
            
        -- Insert head models
        for index, head in model:headModels() do
            insertHeadModel(head, index)
        end
    end
    
    return modelId
end

function Converter.insertStaticModel(db, q, model)
    local stmt, st2
    local modelId
    
    local matIds    = {}
    local texIds    = {}
    local texSetIds = {}
    local vertIds   = {}
    local geoIds    = {}

    -- materials
    db:transaction(function()
        stmt = q.insertMaterial
        for mat in model:materials() do
            if mat:getName() == "NULL" then
                mat:setId(q.nullMaterialId)
                table.insert(matIds, q.nullMaterialId)
                goto done
            end
            
            stmt:bindString(1, mat:getName())
            stmt:bindInt(2, mat:getBlendType())
            stmt:commit()
            
            local id = db:getLastInsertId()
            mat:setId(id)
            table.insert(matIds, id)
            ::done::
        end
    end)
    
    local function blobTexture(tex, isMasked)
        if tex == Texture.NULL then return 0 end
        
        tex:open()
        tex:normalize()
        if isMasked then
            tex:mask()
        end
        
        local id, dupe = Converter.insertBlob(db, q, tex:data(), tex:length(), true)
        tex:close()
        
        return id, dupe
    end
    
    local function handleTexture(tex, normalTex, isMasked)
        if tex:getId() then return end
        
        local diffuseId, diffuseDupe = blobTexture(tex, isMasked)
        local normalId, normalDupe = 0
        
        if normalTex then
            normalId, normalDupe = blobTexture(normalTex, false)
        end
        
        if diffuseId == 0 then return end
        
        stmt:bindString(1, tex:getName())
        stmt:bindInt(2, diffuseId)
        stmt:bindInt(3, normalId)
        stmt:bindInt(4, tex:width())
        stmt:bindInt(5, tex:height())
        stmt:commit()
        
        local id
        if diffuseDupe then
            st2:bindInt64(1, diffuseId)
            
            if st2:select() then
                id = st2:getInt64(1)
                st2:reset()
                goto gotId
            end
        end
        
        id = db:getLastInsertId()
        ::gotId::
        tex:setId(id)
        table.insert(texIds, id)
    end
    
    -- textures
    db:transaction(function()
        stmt = q.insertTexture
        st2  = q.selectTextureDiffuse
        for tex, normalTex in model:textures() do
            handleTexture(tex, normalTex, false)
        end

        for tex in model:maskedTextures() do
            handleTexture(tex, nil, true)
        end
    end)
    
    -- texture sets
    db:transaction(function()
        stmt = q.insertTextureSet
        st2  = q.insertSet2Texture
        for mat in model:materials() do
            stmt:bindInt(1, mat:getAnimDelay())
            stmt:commit()
            
            local setId = db:getLastInsertId()
            mat:setTextureSetId(setId)
            table.insert(texSetIds, setId)
            
            for tex in mat:textures() do
                st2:bindInt64(1, setId)
                st2:bindInt64(2, tex:getId())
                --st2:bindString(3, tex:getName())
                st2:commit()
            end
        end
    end)
    
    local function handleVertexBuffer(vb, noCollide)
        if vb:isEmpty() then return end
        
        local blobId = Converter.insertBlob(db, q, vb:data(), vb:length(), true)
        stmt:bindInt64(1, blobId)
        stmt:bindInt(2, noCollide and 1 or 0)
        stmt:commit()
        
        local id = db:getLastInsertId()
        vb:setId(id)
        table.insert(vertIds, id)
    end
    
    -- vertex buffers
    db:transaction(function()
        stmt = q.insertVertices
        for vb in model:vertexBuffers() do
            handleVertexBuffer(vb, false)
        end

        for vb in model:noCollideVertexBuffers() do
            handleVertexBuffer(vb, true)
        end
    end)
    
    local function handleGeometry(vb)
        if vb:isEmpty() then return end
        
        local mat = vb:getMaterial()
        stmt:bindInt64(1, vb:getId())
        --io.write("VB ", mat:getName(), " ", tostring(mat:getId()), "\n")
        stmt:bindInt64(2, mat:getId())
        
        -- null materials usually have no textures
        local setId = mat:getTextureSetId()
        if setId then
            stmt:bindInt64(3, setId)
        else
            stmt:bindNull(3)
        end
        
        stmt:commit()
        
        table.insert(geoIds, db:getLastInsertId())
    end
    
    -- geometry
    db:transaction(function()
        stmt = q.insertGeometry
        for vb in model:vertexBuffers() do
            handleGeometry(vb)
        end

        for vb in model:noCollideVertexBuffers() do
            handleGeometry(vb)
        end
    end)
    
    -- model
    db:transaction(function()
        stmt = q.insertModel
        
        stmt:commit()
        modelId = db:getLastInsertId()
        
        stmt = q.insertModels2Textures
        for i, texId in ipairs(texIds) do
            stmt:bindInt64(1, modelId)
            stmt:bindInt64(2, texId)
            stmt:commit()
        end
        
        stmt = q.insertModels2TextureSets
        for i, setId in ipairs(texSetIds) do
            stmt:bindInt64(1, modelId)
            stmt:bindInt64(2, setId)
            stmt:commit()
        end
        
        stmt = q.insertModels2Materials
        for i, matId in ipairs(matIds) do
            stmt:bindInt64(1, modelId)
            stmt:bindInt64(2, matId)
            stmt:commit()
        end
        
        stmt = q.insertModels2Vertices
        for i, vertId in ipairs(vertIds) do
            stmt:bindInt64(1, modelId)
            stmt:bindInt64(2, vertId)
            stmt:commit()
        end
        
        stmt = q.insertModels2Geometry
        for i, geoId in ipairs(geoIds) do
            stmt:bindInt64(1, modelId)
            stmt:bindInt64(2, geoId)
            stmt:commit()
        end
    end)
    
    return modelId
end

return Converter
