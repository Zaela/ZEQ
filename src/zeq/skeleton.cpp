
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
    PerfTimer atimer;
    float frame = m_curAnimFrame + delta;
    
    //check against duration here
    while (frame > m_curAnimDuration)
    {
        frame -= m_curAnimDuration;
        m_curAnimFrame = frame;
        for (uint32_t i = 0; i < m_boneCount; i++)
        {
            m_bones[i].animHint = Animation::DEFAULT_HINT;
        }
    }
    
    m_curAnimFrame      = frame;
    Animation* anim     = m_curAnim;
    Bone* bones         = m_bones;
    uint32_t count      = m_boneCount;
    Mat4* animMatrices  = m_animMatrices;
    
    // Animate bones
    for (uint32_t i = 0; i < count; i++)
    {
        Bone& bone = bones[i];
        
        if (!bone.hasAnimFrames)
            continue;
        
        anim->getFrameData(frame, i, bone.pos, bone.rot, bone.scale, bone.animHint);
    }
    
    buildMatrices();
    
    for (uint32_t i = 0; i < count; i++)
    {
        Bone& bone      = bones[i];
        animMatrices[i] = bone.globalAnimMatrix * bone.globalInverseMatrix;
    }
    
    atimer.print("Moved bones");
    
    // Transform vertices
    count = m_vertexBufferCount;
    
    PerfTimer timer;
    
    for (uint32_t i = 0; i < count; i++)
    {
        VertexBufferSet& set = m_vertexBufferSets[i];
        
        const VertexBuffer::Vertex* src = set.base;
        VertexBuffer::Vertex* dst       = set.target;
        uint32_t n                      = set.assignmentCount;
        WeightedBoneAssignment* bas     = set.assignments;
        
        for (uint32_t j = 0; j < n; j++)
        {
            dst[j].moved = false;
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
            
            if (!vert.moved)
            {
                vert.moved  = true;
                vert.pos    = pos * weight;
                vert.normal = normal * weight;
            }
            else
            {
                vert.pos    += pos * weight;
                vert.normal += normal * weight;
            }
        }
    }
    
    timer.print("Moved vertices");
}

void Skeleton::buildMatrices()
{
    for (uint32_t i = 0; i < m_boneCount; i++)
    {
        Bone& bone = m_bones[i];
        
        if (!bone.hasAnimFrames)
            continue;
        
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
        
        if (bone.parentGlobalAnimMatrix)
            bone.globalAnimMatrix = (*bone.parentGlobalAnimMatrix) * mat;
        else
            bone.globalAnimMatrix = mat;
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
    
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glRotatef(-90, 1, 0, 0);
    glScalef(1, -1, 1);
    
    
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
    
    
    glPopMatrix();
    
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_TEXTURE_2D);
}
