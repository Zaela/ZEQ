
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

void Skeleton::animate(double delta)
{
    //PerfTimer atimer;
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
    
    if (!m_vertexBufferSets.empty())
        animateEQG(frame);
    else
        animateWLD(frame);
    
    /*
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
    
    //atimer.print("Moved bones");
    
    // Transform vertices
    //PerfTimer timer;
    
    if (!m_vertexBufferSets.empty())
        moveVerticesEQG(animMatrices);
    else
        moveVerticesWLD(animMatrices);
    
    //timer.print("Moved vertices");
    */
}

void Skeleton::animateEQG(float frame)
{
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
    
    buildMatricesEQG();
    
    for (uint32_t i = 0; i < count; i++)
    {
        Bone& bone      = bones[i];
        animMatrices[i] = bone.globalAnimMatrix * bone.globalInverseMatrix;
    }
    
    //atimer.print("Moved bones");
    
    // Transform vertices
    //PerfTimer timer;

    moveVerticesEQG(animMatrices);
    
    //timer.print("Moved vertices");
}

void Skeleton::moveVerticesEQG(Mat4* animMatrices)
{
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
        }
    }
}

void Skeleton::animateWLD(float frame)
{
    m_curAnimFrame      = frame;
    Animation* anim     = m_curAnim;
    Bone* bones         = m_bones;
    uint32_t count      = m_boneCount;
    Mat4* animMatrices  = m_animMatrices;
    
    // Animate bones
    Vec3 rot;
    for (uint32_t i = 0; i < count; i++)
    {
        Bone& bone = bones[i];
        
        if (!bone.hasAnimFrames)
            continue;
        
        anim->getFrameData(frame, i, bone.pos, rot, bone.animHint);
        
        //printf("Bone %u rot %g, %g, %g\n", i, rot.x, rot.y, rot.z);
        
        /*
        if (bone.rot.x > 90.0f) bone.rot.x = 90.0f;
        if (bone.rot.y > 90.0f) bone.rot.y = 90.0f;
        if (bone.rot.z > 90.0f) bone.rot.z = 90.0f;
        */
        
        bone.rot.x = rot.x;
        bone.rot.y = rot.y;
        bone.rot.z = rot.z;
    }
    
    buildMatricesWLD();
    
    for (uint32_t i = 0; i < count; i++)
    {
        Bone& bone      = bones[i];
        animMatrices[i] = bone.globalAnimMatrix;
    }
    
    //atimer.print("Moved bones");
    
    // Transform vertices
    //PerfTimer timer;

    moveVerticesWLD(animMatrices);
    
    //timer.print("Moved vertices");
}

void Skeleton::moveVerticesWLD(Mat4* animMatrices)
{
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
        }
    }
}

void Skeleton::buildMatricesEQG()
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

void Skeleton::buildMatricesWLD()
{
    for (uint32_t i = 0; i < m_boneCount; i++)
    {
        Bone& bone = m_bones[i];
        
        if (!bone.hasAnimFrames)
            continue;
        
        /*
        Vec3 rot(bone.rot.x, bone.rot.y, bone.rot.z);
        
        if (bone.parentGlobalAnimMatrix)
        {
            Mat4& pMat = *bone.parentGlobalAnimMatrix;
            
            Vec3 tmp = bone.pos;
            pMat.rotateVector(bone.pos, tmp);
            
            bone.pos.x += pMat[12];
            bone.pos.y += pMat[13];
            bone.pos.z += pMat[14];
            
            Mat4 mat = Mat4::angleXYZ(rot);
            mat = pMat * mat;
            mat.setTranslation(bone.pos);
            bone.globalAnimMatrix = mat;
            continue;
        }
        
        Mat4 mat = Mat4::angleXYZ(rot);
        mat.setTranslation(bone.pos);
        bone.globalAnimMatrix = mat;
        */
        
        Vec3 rot(bone.rot.x, bone.rot.y, bone.rot.z);
        
        Mat4 mat = Mat4::angleXYZ(rot);
        mat.setTranslation(bone.pos);
        
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
    if (!m_vertexBufferSets.empty())
    {
        glScalef(1, -1, 1);
    }
    glRotatef(-90, 1, 0, 0);
    
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
    
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_TEXTURE_2D);
}
