
#include "model_resources.hpp"

ModelResources gModelResources;
extern Lua gLua;
extern Database gDatabase;

void ModelResources::init()
{
    //gDatabase.prepare(QUERY_BLOB, m_queryBlob);
    //gDatabase.prepare(QUERY_MODEL_TEXTURES, m_queryModelTextures);
    //gDatabase.prepare(QUERY_MODEL_TEXTURE_SETS, m_queryModelTextureSets);
    //gDatabase.prepare(QUERY_MODEL_VERTICES, m_queryModelVertices);
    //gDatabase.prepare(QUERY_MODEL_GEOMETRY, m_queryModelGeometry);
    //gDatabase.prepare(QUERY_ZONE_MODEL_ID, m_queryZoneModelId);
    //gDatabase.prepare(QUERY_ZONE_CACHED_OCTREES, m_queryZoneCachedOctrees);
}

void ModelResources::getBlob(int64_t id, Blob& blob)
{
    gDatabase.prepare(QUERY_BLOB, blob.query);
    
    blob.query.bindInt64(1, id);
    
    uint32_t realLength, dbLength;
    
    if (!blob.query.select())
    {
        //throw
    }

    realLength  = blob.query.getInt(1);
    blob.data   = (byte*)blob.query.getBlob(2, dbLength);
    blob.length = dbLength;
    
    // If realLength is non-zero, the blob is compressed
    if (realLength != 0)
    {
        byte* buf = new byte[realLength];
        
        FreeImage_ZLibUncompress(buf, realLength, (byte*)blob.data, blob.length);
        
        blob.data           = buf;
        blob.length         = realLength;
        blob.needsDelete    = true;
    }
}

int64_t ModelResources::insertBlob(void* data, uint32_t len, bool compress)
{
    Query insertBlob;
    gDatabase.prepare("INSERT INTO Blobs (realLength, data) VALUES (?, ?)", insertBlob);
    
    uint32_t dbLength;
    byte* blob;
    TempVector<byte> tempBuffer;
    
    if (compress)
    {
        tempBuffer.reserve(len * 2);
        
        dbLength = FreeImage_ZLibCompress(tempBuffer.data(), tempBuffer.capacity(), (byte*)data, len);
        
        if (dbLength == 0)
        {
            //throw
        }
        
        blob = tempBuffer.data();
    }
    else
    {
        dbLength = len;
        blob = (byte*)data;
    }
    
    insertBlob.bindInt64(1, (int64_t)len);
    insertBlob.bindBlob(2, blob, dbLength);
    insertBlob.commit();
    
    return gDatabase.lastInsertId();
}

ZoneModel* ModelResources::loadZoneModel(const std::string& shortname)
{
    ZoneModel* zoneModel = loadZoneModel_impl(shortname);
    
    if (zoneModel)
        return zoneModel;
    
    // Wasn't already loaded, attempt to do a conversion from the EQ folder
    printf("no data found\nAttempting to convert zone from original data files...\n");
    fflush(stdout);
    
    if (!gLua.convertZone(shortname))
    {
        //throw
        return nullptr;
    }
    
    return loadZoneModel_impl(shortname);
}

ZoneModel* ModelResources::loadZoneModel_impl(const std::string& shortname)
{
    int64_t modelId = -1;
    int octreeCached;
    PerfTimer timer;
    Query queryZoneModelId;
    
    printf("Loading zone data for '%s'... ", shortname.c_str());
    fflush(stdout);
    
    // Retrieve model ID for the zone
    gDatabase.prepare(QUERY_ZONE_MODEL_ID, queryZoneModelId);
    queryZoneModelId.bindString(1, shortname);
    
    while (queryZoneModelId.select())
    {
        modelId         = queryZoneModelId.getInt64(1);
        octreeCached    = queryZoneModelId.getInt(2);
    }
    
    if (modelId == -1)
        return nullptr;
    
    ZoneModel* zoneModel = new ZoneModel(modelId);
    m_buildModel = zoneModel;
    
    gModelResources.loadEssentials(modelId);
    
    timer.print("done");
    
    if (!octreeCached)
    {
        zoneModel->generateOctree();
    }
    else
    {
        loadCachedOctree(modelId, zoneModel);
    }
    
    m_buildModel = nullptr;
    return zoneModel;
}

void ModelResources::loadCachedOctree(int64_t modelId, ZoneModel* zoneModel)
{
    std::vector<VertexBuffer*>& src     = zoneModel->getReferencedVertexBuffers();
    std::vector<VertexBuffer*>& dest    = zoneModel->getVertexBuffers();
    std::vector<AABB>& boxes            = zoneModel->getBoundingBoxes();
    
    printf("Loading cached Octree... ");
    fflush(stdout);
    PerfTimer timer;
    Query queryZoneCachedOctrees;
    
    gDatabase.prepare(QUERY_ZONE_CACHED_OCTREES, queryZoneCachedOctrees);
    queryZoneCachedOctrees.bindInt64(1, modelId);
    
    for (VertexBuffer* vb : src)
    {
        vb->registerWithOpenGL();
        dest.push_back(vb);
        
        queryZoneCachedOctrees.bindInt64(2, vb->getId());
        
        while (queryZoneCachedOctrees.select())
        {
            Vec3 min, max;
            
            min.x = queryZoneCachedOctrees.getDouble(1);
            min.y = queryZoneCachedOctrees.getDouble(2);
            min.z = queryZoneCachedOctrees.getDouble(3);
            max.x = queryZoneCachedOctrees.getDouble(4);
            max.y = queryZoneCachedOctrees.getDouble(5);
            max.z = queryZoneCachedOctrees.getDouble(6);
            
            AABB box(min, max);
            
            boxes.push_back(box);
        }
    }
    
    src.clear();
    timer.print("done");
}

void ModelResources::cacheOctree(ZoneModel* zoneModel)
{
    int64_t modelId                             = zoneModel->getId();
    std::vector<VertexBuffer*>& vertexBuffers   = zoneModel->getVertexBuffers();
    std::vector<AABB>& boundingBoxes            = zoneModel->getBoundingBoxes();
    
    printf("Caching Octree... ");
    fflush(stdout);
    PerfTimer timer;
    
    // These queries are not cached because we don't expect this to happen often
    Query queryDeleteVertices;
    Query queryDeleteGeometry;
    Query queryDeleteModels2Vertices;
    Query queryDeleteModels2Geometry;
    
    // Step 1: delete existing vertex buffers and related associations for this zone from the DB
    // This is why we aren't bothering with foreign key contraints on the DB...
    gDatabase.prepare("DELETE FROM Vertices WHERE id IN (SELECT vertId FROM Models2Vertices WHERE modelId = ?)", queryDeleteVertices);
    gDatabase.prepare("DELETE FROM Geometry WHERE vertId IN (SELECT vertId FROM Models2Vertices WHERE modelId = ?)", queryDeleteGeometry);
    gDatabase.prepare("DELETE FROM Models2Vertices WHERE modelId = ?", queryDeleteModels2Vertices);
    gDatabase.prepare("DELETE FROM Models2Geometry WHERE modelId = ?", queryDeleteModels2Geometry);
    
    queryDeleteVertices.bindInt64(1, modelId);
    queryDeleteVertices.commit();
    
    queryDeleteGeometry.bindInt64(1, modelId);
    queryDeleteGeometry.commit();
    
    queryDeleteModels2Vertices.bindInt64(1, modelId);
    queryDeleteModels2Vertices.commit();
    
    queryDeleteModels2Geometry.bindInt64(1, modelId);
    queryDeleteModels2Geometry.commit();
    
    // Step 2: regenerate the above tables, and fill in the CachedOctree table as well
    Query queryInsertVertices;
    Query queryInsertGeometry;
    Query queryInsertModels2Vertices;
    Query queryInsertModels2Geometry;
    Query queryInsertCachedOctrees;
    Query queryUpdateZoneModel;
    
    gDatabase.prepare("INSERT INTO Vertices (noCollision, blobId) VALUES (0, ?)", queryInsertVertices); //fixme
    gDatabase.prepare("INSERT INTO Geometry (vertId, texSetId, matId) VALUES (?, ?, ?)", queryInsertGeometry);
    gDatabase.prepare("INSERT INTO Models2Vertices (modelId, vertId) VALUES (?, ?)", queryInsertModels2Vertices);
    gDatabase.prepare("INSERT INTO Models2Geometry (modelId, geoId) VALUES (?, ?)", queryInsertModels2Geometry);
    gDatabase.prepare("INSERT INTO CachedOctrees (zoneModelId, vertId, minX, minY, minZ, maxX, maxY, maxZ) VALUES (?, ?, ?, ?, ?, ?, ?, ?)", queryInsertCachedOctrees);
    gDatabase.prepare("UPDATE ZoneModels SET octreeCached = 1 WHERE modelId = ?", queryUpdateZoneModel);
    
    gDatabase.beginTransaction();
    
    queryUpdateZoneModel.bindInt64(1, modelId);
    queryUpdateZoneModel.commit();
    
    queryInsertModels2Vertices.bindInt64(1, modelId);
    queryInsertModels2Geometry.bindInt64(1, modelId);
    queryInsertCachedOctrees.bindInt64(1, modelId);
    
    uint32_t n = vertexBuffers.size();
    for (uint32_t i = 0; i < n; i++)
    {
        VertexBuffer* vb = vertexBuffers[i];
        
        int64_t id = insertBlob(vb->array(), vb->bytes());
        
        //queryInsertVertices.bindInt(1, noCollision);
        queryInsertVertices.bindInt64(1, id);
        queryInsertVertices.commit();
        id = gDatabase.lastInsertId();
        
        queryInsertModels2Vertices.bindInt64(2, id);
        queryInsertModels2Vertices.commit();
        
        AABB box = boundingBoxes[i];
        queryInsertCachedOctrees.bindInt64(2, id);
        queryInsertCachedOctrees.bindDouble(3, box.min().x);
        queryInsertCachedOctrees.bindDouble(4, box.min().y);
        queryInsertCachedOctrees.bindDouble(5, box.min().z);
        queryInsertCachedOctrees.bindDouble(6, box.max().x);
        queryInsertCachedOctrees.bindDouble(7, box.max().y);
        queryInsertCachedOctrees.bindDouble(8, box.max().z);
        queryInsertCachedOctrees.commit();
        
        queryInsertGeometry.bindInt64(1, id);
        queryInsertGeometry.bindInt64(2, vb->getTextureSetId());
        queryInsertGeometry.bindInt64(3, vb->getMaterialId());
        queryInsertGeometry.commit();
        id = gDatabase.lastInsertId();
        
        queryInsertModels2Geometry.bindInt64(2, id);
        queryInsertModels2Geometry.commit();
    }
    
    gDatabase.commitTransaction();
    
    timer.print("done");
}

void ModelResources::loadEssentials(int64_t modelId)
{
    loadTextures(modelId);
    loadTextureSets(modelId);
    loadVertices(modelId);
    loadGeometry(modelId);
}

void ModelResources::loadTextures(int64_t modelId)
{
    Query queryModelTextures;
    
    gDatabase.prepare(QUERY_MODEL_TEXTURES, queryModelTextures);
    queryModelTextures.bindInt64(1, modelId);
    
    while (queryModelTextures.select())
    {
        int64_t texId       = queryModelTextures.getInt64(1);
        int64_t diffuseId   = queryModelTextures.getInt64(2);
        int64_t normalId    = queryModelTextures.getInt64(3);
        int width           = queryModelTextures.getInt(4);
        int height          = queryModelTextures.getInt(5);
        
        bool alreadyExists  = (m_textures.count(texId) != 0);
        
        if (alreadyExists)
        {
            // Already had this texture loaded, simply increment ref counts
            m_textures[texId]->grab();
            continue;
        }
        
        // Texture ids may be null, which gets converted to 0 by getInt64 (sqlite starts auto increment primary keys at 1)
        Texture* tex = new Texture(
            texId,
            diffuseId ? getTextureBlob(diffuseId, width, height) : 0,
            normalId  ? getTextureBlob(normalId, width, height)  : 0,
            width,
            height
        );
        
        m_buildModel->addTexture(tex);
        m_textures[texId] = tex;
    }
}

void ModelResources::loadTextureSets(int64_t modelId)
{
    Query queryModelTextureSets;
    
    gDatabase.prepare(QUERY_MODEL_TEXTURE_SETS, queryModelTextureSets);
    queryModelTextureSets.bindInt64(1, modelId);
    
    AnimatedTexture* animTex = nullptr;
    
    while (queryModelTextureSets.select())
    {
        int64_t setId       = queryModelTextureSets.getInt64(1);
        int64_t animDelay   = queryModelTextureSets.getInt64(2);
        int64_t texId       = queryModelTextureSets.getInt64(3);
        
        bool alreadyExists  = (m_textureSets.count(setId) != 0);
        
        if (!alreadyExists)
        {
            animTex = new AnimatedTexture(setId, animDelay);
            m_textureSets[setId] = animTex;
        }
        
        if (m_textures.count(texId) == 0)
        {
            //throw
        }
        
        animTex->addTexture(m_textures[texId]);
        m_buildModel->addAnimatedTexture(animTex);
    }
}

void ModelResources::loadVertices(int64_t modelId)
{
    Query queryModelVertices;
    
    gDatabase.prepare(QUERY_MODEL_VERTICES, queryModelVertices);
    queryModelVertices.bindInt64(1, modelId);
    
    while (queryModelVertices.select())
    {
        int64_t vertId      = queryModelVertices.getInt64(1);
        int64_t blobId      = queryModelVertices.getInt64(2);
        //int noCollision     = queryModelVertices.getInt(3);
        //int verticesOnly    = queryModelVertices.getInt(4);
        
        bool alreadyExists = (m_vertices.count(vertId) != 0);
        
        if (alreadyExists)
        {
            m_vertices[vertId]->grab();
            continue;
        }
        
        Blob blob;
        getBlob(blobId, blob);
        
        VertexBuffer* vb = new VertexBuffer(vertId, blob.takeOwnership(), blob.length);
        
        m_buildModel->addVertexBuffer(vb);
        m_vertices[vertId] = vb;
    }
}

void ModelResources::loadGeometry(int64_t modelId)
{
    Query queryModelGeometry;
    
    gDatabase.prepare(QUERY_MODEL_GEOMETRY, queryModelGeometry);
    queryModelGeometry.bindInt64(1, modelId);
    
    while (queryModelGeometry.select())
    {
        int64_t vertId      = queryModelGeometry.getInt64(1);
        int64_t texSetId    = queryModelGeometry.getInt64(2);
        int64_t matId       = queryModelGeometry.getInt64(3);
        int blendType       = queryModelGeometry.getInt64(4);
        
        if (m_vertices.count(vertId) == 0)
            continue;
        
        VertexBuffer* vb = m_vertices[vertId];
        
        if (m_textureSets.count(texSetId) != 0)
            vb->setAnimatedTexture(m_textureSets[texSetId]);
        
        vb->setBlendType(matId, texSetId, blendType);
    }
}

ModelResources::Blob::~Blob()
{
    if (needsDelete && data)
        delete[] data;
    
    // There are some cases (e.g. Textures) where we want to avoid making an intermediate copy of a blob,
    // because we know Irrlicht is going to make its own copy immediately anyway. However, the memory that
    // SQLite hands us will be immediately invalidated as soon as the query is reset. So we need to make
    // sure the query is not reset until we are done using the blob, which is why this line is here.
    // Note that this also means you can only have one blob active at a time; the current blob must be
    // destructed before the query can be safely re-run.
    //gModelResources.m_queryBlob.reset();
}

uint32_t ModelResources::getTextureBlob(int64_t id, int width, int height)
{
    Blob blob;
    getBlob(id, blob);
    
    return OpenGL::loadTexture(blob.data, width, height);
}

void ModelResources::removeTexture(int64_t id)
{
    m_textures.erase(id);
}

void ModelResources::removeAnimatedTexture(int64_t id)
{
    m_textureSets.erase(id);
}

void ModelResources::removeVertexBuffer(int64_t id)
{
    m_vertices.erase(id);
}
