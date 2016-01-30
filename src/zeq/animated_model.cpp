
#include "animated_model.hpp"
#include "model_resources.hpp"

extern ModelResources gModelResources;

AnimatedModelPrototype::~AnimatedModelPrototype()
{
    m_animations.destroy();
}

void AnimatedModelPrototype::readSkeleton(byte* binBones, uint32_t len)
{
    DBBone* frames  = (DBBone*)binBones;
    uint32_t count  = len / sizeof(DBBone);
    
    m_boneCount = count;
    m_bones = new Bone[count];
    
    uint32_t cur = 0;
    readSkeletonRecurse(frames, cur, m_bones[0], count);
}

void AnimatedModelPrototype::readSkeletonRecurse(DBBone* frames, uint32_t& cur, Bone& bone, uint32_t count, Bone* parent, uint32_t parentIndex)
{
    if (cur == count)
        return;
    
    DBBone& frame = frames[cur];
    
    bone.hasAnimFrames = false;
    
    bone.pos    = frame.pos;
    bone.scale  = frame.scale;
    bone.rot    = frame.rot;
    
    Mat4 posMatrix;
    posMatrix.setTranslation(frame.pos);
    Mat4 rotMatrix;
    frame.rot.getMatrixTransposed(rotMatrix);
    Mat4 scaleMatrix;
    scaleMatrix.setScale(frame.scale);
    
    bone.localMatrix = posMatrix * rotMatrix * scaleMatrix;
    
    if (parent)
        bone.globalMatrix = parent->globalMatrix * bone.localMatrix;
    else
        bone.globalMatrix = bone.localMatrix;
    
    bone.globalInverseMatrix = bone.globalMatrix;
    bone.globalInverseMatrix.invert();
    
    bone.parentIndex = parentIndex;
    
    uint32_t n = frame.childCount;
    
    if (n)
    {
        uint32_t index = cur;
        
        for (uint32_t i = 0; i < n; i++)
        {
            Bone& child = m_bones[++cur];
            readSkeletonRecurse(frames, cur, child, count, &bone, index);
        }
    }
}

void AnimatedModelPrototype::readAnimationFrames(int animId, int boneIndex, byte* frames, uint32_t len)
{
    m_bones[boneIndex].hasAnimFrames = true;
    m_animations.set(animId, boneIndex, m_boneCount, frames, len);
}

WeightedBoneAssignmentSet& AnimatedModelPrototype::readWeightedBoneAssignments(byte* binWbas, uint32_t len)
{
    WeightedBoneAssignmentSet set;
    
    set.count       = len / sizeof(WeightedBoneAssignment);
    set.assignments = (WeightedBoneAssignment*)binWbas;
    
    m_weightedBoneAssignments.push_back(set);
    return m_weightedBoneAssignments.back();
}

BoneAssignmentSet& AnimatedModelPrototype::readBoneAssignments(byte* binBas, uint32_t len)
{
    BoneAssignmentSet set;
    
    set.count       = len / sizeof(uint32_t);
    set.assignments = (uint32_t*)binBas;
    
    m_boneAssignments.push_back(set);
    return m_boneAssignments.back();
}

Skeleton* AnimatedModelPrototype::createSkeletonInstance()
{
    Skeleton* sk = new Skeleton;
    
    // The skeleton gets its own copies of all the bones
    uint32_t count  = m_boneCount;
    sk->m_boneCount = count;
    
    Skeleton::Bone* bones   = new Skeleton::Bone[count];
    sk->m_bones             = bones;
    sk->m_animMatrices      = new Mat4[count];
    
    memset(bones, 0, sizeof(Skeleton::Bone) * count);
    
    for (uint32_t i = 0; i < count; i++)
    {
        Skeleton::Bone& dst = bones[i];
        Bone& src           = m_bones[i];
        
        dst.hasAnimFrames   = src.hasAnimFrames;
        
        dst.pos     = src.pos;
        dst.rot     = src.rot;
        dst.scale   = src.scale;
        
        //dst.localMatrix         = src.localMatrix;
        dst.localAnimMatrix     = src.localMatrix;
        //dst.globalMatrix        = src.globalMatrix; //probably not needed
        dst.globalAnimMatrix    = src.globalMatrix;
        dst.globalInverseMatrix = src.globalInverseMatrix;
        
        dst.parentGlobalAnimMatrix = (i != 0) ? &bones[src.parentIndex].globalAnimMatrix : nullptr;
        
        dst.animHint = Animation::DEFAULT_HINT;
    }
    
    // The skeleton needs its own copies of all the VertexBuffers, but needs the original VertexBuffers to transform each frame
    // The bone assignment definitions are centralized, belonging to the prototype
    count = m_weightedBoneAssignments.size();
    
    sk->m_vertexBufferCount = count;
    
    Skeleton::VertexBufferSet* sets = new Skeleton::VertexBufferSet[count];
    VertexBuffer* vbs               = new VertexBuffer[count];
    
    sk->m_vertexBufferSets      = sets;
    sk->m_ownedVertexBuffers    = vbs;
    
    for (uint32_t i = 0; i < count; i++)
    {
        WeightedBoneAssignmentSet& src = m_weightedBoneAssignments[i];
        
        Skeleton::VertexBufferSet& set  = sets[i];
        VertexBuffer& vb                = vbs[i];
        
        vb.copy(*src.vertexBuffer);
        
        set.vertexCount     = vb.count();
        set.target          = vb.array();
        set.base            = src.vertexBuffer->array();
        set.assignmentCount = src.count;
        set.assignments     = src.assignments;
    }
    
    // Animation definitions are centralized, belonging to the prototype
    sk->m_animations.inherit(m_animations);
    
    return sk;
}

MobModelPrototype::MobModelPrototype(int race, uint8_t gender)
    : m_race(race),
      m_gender(gender)
{
    
}

MobModelPrototype::~MobModelPrototype()
{
    gModelResources.removeMobModel(m_race, m_gender);
}
