
#include "animated_model.hpp"
#include "model_resources.hpp"

extern ModelResources gModelResources;

void AnimatedModelPrototype::readSkeleton(byte* binBones, uint32_t len)
{
    DBBone* frames  = (DBBone*)binBones;
    uint32_t count  = len / sizeof(DBBone);
    
    m_bones.resize(count);
    
    uint32_t cur = 0;
    readSkeletonRecurse(frames, cur, m_bones[0], count);
}

void AnimatedModelPrototype::readSkeletonRecurse(DBBone* frames, uint32_t& cur, Bone& bone, uint32_t count)
{
    if (cur == count)
        return;
    
    DBBone& frame = frames[cur];
    
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
    
    if (bone.parent)
        bone.globalMatrix = bone.parent->globalMatrix * bone.localMatrix;
    else
        bone.globalMatrix = bone.localMatrix;
    
    //bone.localAnimMatrix = bone.localMatrix;
    bone.globalAnimMatrix = bone.globalMatrix;
    
    bone.globalInverseMatrix = bone.globalMatrix;
    bone.globalInverseMatrix.invert();
    
    uint32_t n = frame.childCount;
    
    for (uint32_t i = 0; i < n; i++)
    {
        cur++;
        
        Bone& child = m_bones[cur];
        
        child.parent = &bone;
    
        bone.children.push_back(&child);
        readSkeletonRecurse(frames, cur, child, count);
    }
}

void AnimatedModelPrototype::readAnimationFrames(int animId, int boneIndex, byte* frames, uint32_t len)
{
    std::vector<Frame>& dst = m_bones[boneIndex].frames[animId];
    
    dst.resize(len / sizeof(Frame));
    memcpy(dst.data(), frames, len);
}

std::vector<WeightedBoneAssignment>& AnimatedModelPrototype::readWeightedBoneAssignments(byte* binWbas, uint32_t len)
{
    WeightedBoneAssignment* wbas    = (WeightedBoneAssignment*)binWbas;
    uint32_t count                  = len / sizeof(WeightedBoneAssignment);
    
    m_weightedBoneAssignments.push_back(std::vector<WeightedBoneAssignment>());
    
    std::vector<WeightedBoneAssignment>& vector = m_weightedBoneAssignments.back();
    vector.resize(count);
    memcpy(vector.data(), wbas, len);
    
    return vector;
}

std::vector<uint32_t>& AnimatedModelPrototype::readBoneAssignments(byte* binBas, uint32_t len)
{
    uint32_t* bas   = (uint32_t*)binBas;
    uint32_t count  = len / sizeof(uint32_t);
    
    m_boneAssignments.push_back(std::vector<uint32_t>());
    
    std::vector<uint32_t>& vector = m_boneAssignments.back();
    vector.resize(count);
    memcpy(vector.data(), bas, len);
    
    return vector;
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
