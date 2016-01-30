
#include "skeleton.hpp"

Skeleton::Skeleton()
    : m_boneCount(0),
      m_bones(nullptr),
      m_vertexBufferCount(0),
      m_vertexBufferSets(nullptr),
      m_ownedVertexBuffers(nullptr),
      m_curAnimId(0),
      m_curAnim(nullptr),
      m_curAnimDuration(0.0f),
      m_curAnimFrame(0.0f)
{
    
}

Skeleton::~Skeleton()
{
    if (m_vertexBufferSets)
        delete[] m_vertexBufferSets;
    
    if (m_ownedVertexBuffers)
        delete[] m_ownedVertexBuffers;
}

void Skeleton::setAnimation(int animId)
{
    Animation* anim = m_animations.get(animId);
    
    m_curAnimId         = animId;
    m_curAnim           = anim;
    m_curAnimDuration   = anim->getDurationMilliseconds();
    m_curAnimFrame      = 0.0f;

    for (uint32_t i = 0; i < m_boneCount; i++)
    {
        m_bones[i].animHint = Animation::DEFAULT_HINT;
    }
}

void Skeleton::animate(double delta)
{
    PerfTimer timer;
    float frame = m_curAnimFrame + delta;
    
    //check against duration here
    
    m_curAnimFrame  = frame;
    Animation* anim = m_curAnim;
    Bone* bones     = m_bones;
    uint32_t count  = m_boneCount;
    
    // Animate bones
    for (uint32_t i = 0; i < count; i++)
    {
        Bone& bone = bones[i];
        
        if (!bone.hasAnimFrames)
            continue;
        
        anim->getFrameData(frame, i, bone.pos, bone.rot, bone.scale, bone.animHint);
    }
    
    // Build matrices
    buildLocalMatrices();
    buildGlobalMatrices();
    
    TempVector<Mat4> animMatrices;
    animMatrices.reserve(count);
    
    for (uint32_t i = 0; i < count; i++)
    {
        Bone& bone = bones[i];
        animMatrices.push_back(bone.globalAnimMatrix * bone.globalInverseMatrix);
    }
    
    // Transform vertices
    count = m_vertexBufferCount;
    
    for (uint32_t i = 0; i < count; i++)
    {
        VertexBufferSet& set = m_vertexBufferSets[i];
        
        const VertexBuffer::Vertex* src = set.base;
        VertexBuffer::Vertex* dst       = set.target;
        uint32_t n                      = set.assignmentCount;
        WeightedBoneAssignment* bas     = set.assignments;
        
        TempVector<bool> moved;
        moved.reserve(set.vertexCount);
        for (uint32_t j = 0; j < set.vertexCount; j++)
        {
            moved.push_back(false);
        }
        
        Vec3 pos;
        Vec3 normal;
        
        for (uint32_t j = 0; j < n; j++)
        {
            WeightedBoneAssignment& wt  = bas[j];
            uint32_t index              = wt.vertIndex;
            
            Mat4& pull = animMatrices[wt.boneIndex];
            
            pull.transformVector(pos, src[index].pos);
            pull.rotateVector(normal, src[index].normal);
            
            VertexBuffer::Vertex& vert  = dst[index];
            float weight                = wt.weight;
            
            if (!moved[index])
            {
                moved[index]    = true;
                vert.pos        = pos * weight;
                vert.normal     = normal * weight;
            }
            else
            {
                vert.pos    += pos * weight;
                vert.normal += normal * weight;
            }
        }
    }
    
    timer.print("Animated");
}

void Skeleton::buildLocalMatrices()
{
    for (uint32_t i = 0; i < m_boneCount; i++)
    {
        Bone& bone = m_bones[i];
        
        if (!bone.hasAnimFrames)
        {
            bone.localAnimMatrix = bone.localMatrix;
            continue;
        }
        
        Mat4& mat = bone.localAnimMatrix;
        
        bone.rot.getMatrixTransposed(mat);
        
        Vec3 pos = bone.pos;
        Vec3 scl = bone.scale;
        
        float scale = scl.x;
        mat[ 0] *= scale;
        mat[ 1] *= scale;
        mat[ 2] *= scale;
        mat[ 3] *= scale;
        
        scale = scl.y;
        mat[ 4] *= scale;
        mat[ 5] *= scale;
        mat[ 6] *= scale;
        mat[ 7] *= scale;
        
        scale = scl.z;
        mat[ 8] *= scale;
        mat[ 9] *= scale;
        mat[10] *= scale;
        mat[11] *= scale;
        
        mat[12] = pos.x;
        mat[13] = pos.y;
        mat[14] = pos.z;
    }
}

void Skeleton::buildGlobalMatrices()
{
    //should check if bone hierarchy is strictly sequential to avoid recursive call overhead...
    Bone& root = m_bones[0];
    
    root.globalAnimMatrix = root.localAnimMatrix;
    
    buildGlobalMatrixRecurse(root);
}

void Skeleton::buildGlobalMatrixRecurse(Bone& bone)
{
    for (uint32_t i = 0; i < bone.childCount; i++)
    {
        Bone& child = m_bones[bone.children[i]];
        
        // Every bone that isn't the root bone has a parent
        child.globalAnimMatrix = bone.globalAnimMatrix * child.localAnimMatrix;
        
        if (child.childCount)
            buildGlobalMatrixRecurse(child);
    }
}

void Skeleton::draw()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    
    uint32_t lastDiffuseMap = 0;
    for (uint32_t i = 0; i < m_vertexBufferCount; i++)
    {
        VertexBuffer* vb = &m_ownedVertexBuffers[i];
        //int blendType = vb->getBlendType();
        
        //if (blendType == Material::Blend::Invisible)
        //    continue;
        
        uint32_t diffuseMap = vb->getDiffuseMap();
        
        if (diffuseMap != lastDiffuseMap)
        {
            glBindTexture(GL_TEXTURE_2D, diffuseMap);
            lastDiffuseMap = diffuseMap;
        }
        
        vb->draw();
    }
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_TEXTURE_2D);
}
