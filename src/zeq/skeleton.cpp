
#include "skeleton.hpp"

Skeleton::Skeleton()
    : m_boneCount(0),
      m_bones(nullptr),
      m_curAnimId(0),
      m_curAnim(nullptr),
      m_curAnimDuration(0.0f),
      m_curAnimFrame(0.0f)
{
    
}

Skeleton::~Skeleton()
{

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

float Skeleton::incrementAnimation(float delta)
{
    float frame = m_curAnimFrame + delta;
    
    // Check against anim duration
    while (frame > m_curAnimDuration)
    {
        frame -= m_curAnimDuration;
        m_curAnimFrame = frame;
        for (uint32_t i = 0; i < m_boneCount; i++)
        {
            m_bones[i].animHint = Animation::DEFAULT_HINT;
        }
    }
    
    m_curAnimFrame = frame;
    return frame;
}

AABB Skeleton::animate(float delta)
{
    float frame = incrementAnimation(delta);
    
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
    
    buildMatrices();
    
    if (isEQG())
        return moveVerticesEQG();
    else
        return moveVerticesWLD();
}

AABB Skeleton::moveVerticesEQG()
{
    Bone* bones         = m_bones;
    uint32_t count      = m_boneCount;
    Mat4* animMatrices  = m_animMatrices;
    
    for (uint32_t i = 0; i < count; i++)
    {
        Bone& bone      = bones[i];
        animMatrices[i] = bone.globalAnimMatrix * bone.globalInverseMatrix;
    }
    
    AABB box;
    
    for (VertexBufferSet& set : m_vertexBufferSets)
    {
        uint32_t vcount                 = set.vertexCount;
        const VertexBuffer::Vertex* src = set.base;
        VertexBuffer::Vertex* dst       = set.target;
        uint32_t n                      = set.assignmentCount;
        WeightedBoneAssignment* bas     = set.assignments;
        
        for (uint32_t i = 0; i < vcount; i++)
        {
            dst[i].moved = false;
        }
        
        Vec3 pos;
        Vec3 normal;
        
        for (uint32_t i = 0; i < n; i++)
        {
            WeightedBoneAssignment& wt  = bas[i];
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
            
            box.addInternalPoint(vert.pos);
        }
    }
    
    return box;
}

AABB Skeleton::moveVerticesWLD()
{
    Bone* bones         = m_bones;
    uint32_t count      = m_boneCount;
    Mat4* animMatrices  = m_animMatrices;
    
    for (uint32_t i = 0; i < count; i++)
    {
        Bone& bone      = bones[i];
        animMatrices[i] = bone.globalAnimMatrix;
    }
    
    AABB box;
    
    for (SimpleVertexBufferSet& set : m_simpleVertexBufferSets)
    {
        uint32_t vcount                 = set.vertexCount;
        const VertexBuffer::Vertex* src = set.base;
        VertexBuffer::Vertex* dst       = set.target;
        
        Vec3 pos;
        Vec3 normal;
        
        for (uint32_t i = 0; i < vcount; i++)
        {
            VertexBuffer::Vertex& vert = dst[i];
            
            Mat4& pull = animMatrices[vert.boneIndex];
            
            pull.transformVector(pos, src[i].pos);
            pull.rotateVector(normal, src[i].normal);
            
            vert.pos    = pos;
            vert.normal = normal;
            
            box.addInternalPoint(vert.pos);
        }
    }
    
    return box;
}

void Skeleton::buildMatrices()
{
    for (uint32_t i = 0; i < m_boneCount; i++)
    {
        Bone& bone = m_bones[i];
        
        if (!bone.hasAnimFrames)
            continue;
        
        Mat4 mat;
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
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    
    glMultMatrixf(getModelMatrix().ptr());
    
    uint32_t lastDiffuseMap = 0;
    for (VertexBuffer& vb : m_ownedVertexBuffers)
    {
        //int blendType = vb->getBlendType();
        
        //if (blendType == Material::Blend::Invisible)
        //    continue;
        
        uint32_t diffuseMap = vb.getDiffuseMap();
        
        if (diffuseMap != lastDiffuseMap)
        {
            glBindTexture(GL_TEXTURE_2D, diffuseMap);
            lastDiffuseMap = diffuseMap;
        }
        
        vb.draw();
    }
    
    
    glPopMatrix();
}
