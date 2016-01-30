
#include "zone_model.hpp"
#include "model_resources.hpp"

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
    
    printf("Generating Octree... ");
    fflush(stdout);
    
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
    
    printf("Creating VBOs... ");
    fflush(stdout);
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

void ZoneModel::draw(Camera* camera)
{
    Frustum& frustum    = camera->getFrustum();
    uint32_t n          = m_boundingBoxes.size();
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    
    /*
    glEnable(GL_FOG);
    //glFogi(GL_FOG_MODE, GL_LINEAR);
    //glFogf(GL_FOG_START, 1.0f);
    //glFogf(GL_FOG_END, 500.0f);
    glFogi(GL_FOG_MODE, GL_EXP);
    glFogf(GL_FOG_DENSITY, 0.01f);
    glHint(GL_FOG_HINT, GL_NICEST);
    */
    
    //glColor3f(0.1f, 0.1f, 0.1f);
    
    int lastBlendType       = -1;
    uint32_t lastDiffuseMap = 0;
    
    PerfTimer timer;
    uint32_t count      = 0;
    uint32_t texCount   = 0;
    
    for (uint32_t i = 0; i < n; i++)
    {
        if (!frustum.contains(m_boundingBoxes[i]))
            continue;
        
        VertexBuffer* vb = m_vertexBuffers[i];
        
        int blendType = vb->getBlendType();
        
        if (blendType == Material::Blend::Invisible)
            continue;
        
        uint32_t diffuseMap = vb->getDiffuseMap();
        
        if (diffuseMap != lastDiffuseMap)
        {
            glBindTexture(GL_TEXTURE_2D, diffuseMap);
            lastDiffuseMap = diffuseMap;
            texCount++;
        }
        
        if (blendType != lastBlendType)
        {
            unsetBlendType(lastBlendType);
            setBlendType(blendType);
            lastBlendType = blendType;
        }
        
        vb->draw();
        count++;
    }
    
    unsetBlendType(lastBlendType);
    /*static int c = 0;
    static double total = 0.0;
    total += timer.seconds();
    printf("avg: %g\n", total / ++c);*/
    //timer.printf("Octree: checked %u nodes and drew %u with %u textures", m_boundingBoxes.size(), count, texCount);
    
    glDisable(GL_FOG);
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_TEXTURE_2D);
}

void ZoneModel::setBlendType(int blendType)
{
    switch (blendType)
    {
    case Material::Blend::Particle:
        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
        // Fallthrough
    case Material::Blend::Masked:
        glEnable(GL_ALPHA_TEST);
        glAlphaFunc(GL_GEQUAL, 0.5f);
        break;
    case Material::Blend::Additive:
        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_SRC_COLOR);
        break;
    default:
        break;
    }
}

void ZoneModel::unsetBlendType(int blendType)
{
    switch (blendType)
    {
    case Material::Blend::Particle:
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        // Fallthrough
    case Material::Blend::Masked:
        glDisable(GL_ALPHA_TEST);
        break;
    case Material::Blend::Additive:
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        break;
    default:
        break;
    }
}
