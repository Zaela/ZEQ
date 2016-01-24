
local ZoneEQG       = require "ZoneEQG"
local ZoneWLD       = require "ZoneWLD"
local MobEQG        = require "MobEQG"
local Database      = require "Database"
local File          = require "File"
local Image         = require "Image"
local Texture       = require "Texture"
local ConvMaterial  = require "ConvMaterial"

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
        io.write(string.format("No known filename for race %i gender %i\n", race, gender))
        return
    end
    
    local obj
    
    if type(mapping) == "string" and mapping:find("%.eqg$") then
        obj = MobEQG.convert(filename)
    else
    
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
    
    if compress then
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

function Converter.initZoneQueries(db)
    local q = {}
    
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
    
    q.insertZoneModel = db:prepare[[
        INSERT INTO ZoneModels (shortname, modelId) VALUES (?, ?)
    ]]
    
    return q
end

function Converter.insertZone(obj, shortname)
    local db    = Converter.initDB()
    local q     = Converter.initZoneQueries(db)
    
    io.write("Inserting zone data into local database... ")
    io.flush()
    
    local time  = os.clock()
    local id    = Converter.insertStaticModel(db, q, obj.zone, shortname)
    local stmt  = q.insertZoneModel
    
    stmt:bindString(1, shortname)
    stmt:bindInt64(2, id)
    stmt:commit()
    
    db:analyze()
    
    q.nullMaterialId = nil
    for k, query in pairs(q) do
        query:finalize()
    end
    
    obj = nil
    q   = nil
    db  = nil
    
    collectgarbage()
    
    io.write("done in ", os.clock() - time, " seconds\n")
end

function Converter.insertStaticModel(db, q, model, name)
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
