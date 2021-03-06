
#include "zone_model.hpp"
#include "model_resources.hpp"

extern Log gLog;
extern Config gConfig;
extern ModelResources gModelResources;

ZoneModel::ZoneModel(int64_t id)
    : m_id(id)
{
    
}

ZoneModel::~ZoneModel()
{
    for (VertexBuffer* vb : m_vertexBuffers)
    {
        vb->drop();
    }
}

void ZoneModel::generateOctree()
{
    OctreeTemp temp;
    PerfTimer timer;
    
    gLog.printf("Generating Octree... ");
    
    for (VertexBuffer* vb : getReferencedVertexBuffers())
    {
        uint32_t triCount = vb->triangleCount();
        
        if (triCount <= MAX_TRIANGLES_PER_NODE && octreeCheckSmallVertexBuffer(vb, temp))
            continue;
        
        AABB box;
        VertexBuffer::Vertex* array = vb->array();
        uint32_t count              = triCount * 3;
        uint32_t curClaim           = 0;
        
        for (uint32_t i = 0; i < count; i++)
        {
            VertexBuffer::Vertex& vert = array[i];
            box.addInternalPoint(vert.pos);
        }
        
        temp.claimed.reserve(triCount);
        memset(temp.claimed.data(), 0, sizeof(uint32_t) * triCount);
        
        octreeRecurse(temp, vb, (VertexBuffer::Triangle*)array, triCount, box, 0, triCount, curClaim);
    }
    
    clearReferencedVertexBuffers();
    
    for (int i = 0; i < Material::Blend::COUNT; i++)
    {
        OctreeTempSet& set = temp.sets[i];
        
        for (AABB& box : set.boundingBoxes)
        {
            m_boundingBoxes.push_back(box);
        }
        
        for (VertexBuffer* vb : set.vertexBuffers)
        {
            m_vertexBuffers.push_back(vb);
        }
    }
    
    double loadingTime = timer.seconds();
    timer.print("done");
    
    if (loadingTime > gConfig.getDouble(Config::CacheOctreesIfLongerThan, 1.5) ||
        gConfig.getBool(Config::AlwaysCacheOctrees, false))
    {
        gModelResources.cacheOctree(this);
    }
    
    gLog.printf("Creating VBOs... ");

    PerfTimer timerVbo;
    
    for (VertexBuffer* vb : m_vertexBuffers)
    {
        vb->registerWithOpenGL();
    }
    
    timerVbo.print("done");
}

void ZoneModel::octreeRecurse(OctreeTemp& temp, VertexBuffer* vb, VertexBuffer::Triangle* triArray, uint32_t vbTriCount,
    AABB& parentBox, uint32_t parentId, uint32_t parentTriCount, uint32_t& curClaim)
{
    Vec3 corners[8];
    Vec3 mid;
    
    parentBox.getAllCorners(corners, mid);
    
    uint32_t* claimed = temp.claimed.data();
    
    for (int c = 0; c < 8; c++)
    {
        uint32_t claim = ++curClaim;
        AABB box(mid, corners[c]);
        uint32_t triCount = 0;
        
        for (uint32_t i = 0; i < vbTriCount; i++)
        {
            if (claimed[i] != parentId)
                continue;
            
            VertexBuffer::Triangle& tri = triArray[i];
            
            for (int j = 0; j < 3; j++)
            {
                VertexBuffer::Vertex& vert = tri.points[j];
                if (!box.containsPoint(vert.pos))
                    goto skip;
            }
            
            claimed[i] = claim;
            triCount++;
            
        skip: ;
        }
        
        parentTriCount -= triCount;
        
        if (triCount > MAX_TRIANGLES_PER_NODE)
        {
            octreeRecurse(temp, vb, triArray, vbTriCount, box, claim, triCount, curClaim);
        }
        else if (triCount > 0)
        {
            octreeMakeNode(temp, vb, triArray, vbTriCount, claim);
        }
    }
    
    // Check if parent box still has ownership of any triangles
    if (parentTriCount > 0)
    {
        octreeMakeNode(temp, vb, triArray, vbTriCount, parentId);
    }
}

void ZoneModel::octreeMakeNode(OctreeTemp& temp, VertexBuffer* vb, VertexBuffer::Triangle* triArray, uint32_t triCount, uint32_t claim)
{
    AABB box;
    TempVector<VertexBuffer::Triangle> tempTriangles;
    
    uint32_t* claimed = temp.claimed.data();
    
    for (uint32_t i = 0; i < triCount; i++)
    {
        if (claimed[i] != claim)
            continue;
        
        VertexBuffer::Triangle& tri = triArray[i];
        
        tempTriangles.push_back(tri);
        
        for (int j = 0; j < 3; j++)
        {
            VertexBuffer::Vertex& vert = tri.points[j];
            box.addInternalPoint(vert.pos);
        }
    }
    
    uint32_t len                    = tempTriangles.size();
    VertexBuffer::Triangle* triCopy = new VertexBuffer::Triangle[len];
    
    len *= sizeof(VertexBuffer::Triangle);
    memcpy(triCopy, tempTriangles.data(), len);
    
    VertexBuffer* newVB = new VertexBuffer(0, (byte*)triCopy, len);
    
    newVB->setAnimatedTexture(vb->getAnimatedTexture());
    newVB->setBlendType(vb->getMaterialId(), vb->getTextureSetId(), vb->getBlendType());
    
    temp.placeVertexBuffer(newVB, box);
}

bool ZoneModel::octreeCheckSmallVertexBuffer(VertexBuffer* vb, OctreeTemp& temp)
{
    AABB box;
    VertexBuffer::Vertex* array = vb->array();
    uint32_t count              = vb->count();
    
    for (uint32_t i = 0; i < count; i++)
    {
        VertexBuffer::Vertex& vert = array[i];
        box.addInternalPoint(vert.pos);
    }
    
    if (box.getSphereRadiusSquared() > SMALL_OCTREE_NODE_MAX_RADIUS)
        return false;
    
    // Grab this VB so it isn't destroyed when we clear all the VBs from ModelPrototype's buffer
    vb->grab();
    temp.placeVertexBuffer(vb, box);
    return true;
}

void ZoneModel::draw(Camera& camera)
{
    Frustum& frustum    = camera.getFrustum();
    uint32_t n          = m_boundingBoxes.size();
    
    /**/
    //float clr[4] = {0.1f, 0.65f, 0.7f, 1.0f};
    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, GL_EXP);
    glFogf(GL_FOG_DENSITY, 0.01f);
    glHint(GL_FOG_HINT, GL_NICEST);
    //float clr[4] = {0.05, 0.0f, 0.0f, 1.0f};
    //glFogfv(GL_FOG_COLOR, clr);
    /**/
    
    //glColor3f(0.1f, 0.65f, 0.7f);
    //glColor3f(0.5f, 0.25f, 0.25f);
    
    MATERIAL_SETUP();
    
    for (uint32_t i = 0; i < n; i++)
    {
        if (!frustum.contains(m_boundingBoxes[i]))
            continue;
        
        VertexBuffer* vb = m_vertexBuffers[i];
        
        int blendType = vb->getBlendType();
        
        if (blendType == Material::Blend::Invisible)
            continue;
        
        uint32_t diffuseMap = vb->getDiffuseMap();
        
        MATERIAL_SET(diffuseMap, blendType);
        
        vb->draw();
    }
    
    MATERIAL_CLEANUP();
    
    glDisable(GL_FOG);
    glColor3f(1.0f, 1.0f, 1.0f);
}
