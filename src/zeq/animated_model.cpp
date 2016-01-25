
#include "animated_model.hpp"
#include "model_resources.hpp"

extern ModelResources gModelResources;

void AnimatedModelPrototype::readSkeleton(byte* binFrames, uint32_t len)
{
    DBFrame* frames = (DBFrame*)binFrames;
    uint32_t count  = len / sizeof(DBFrame);
    
    m_bones.resize(count);
    
    uint32_t cur = 0;
    readSkeletonRecurse(frames, cur, m_bones[0], count);
}

void AnimatedModelPrototype::readSkeletonRecurse(DBFrame* frames, uint32_t& cur, Bone& bone, uint32_t count)
{
    if (cur == count)
        return;
    
    DBFrame& frame = frames[cur];
    
    bone.pos    = frame.pos;
    bone.scale  = frame.scale;
    bone.rot    = frame.rot;
    
    uint32_t n = frame.childCount;
    
    for (uint32_t i = 0; i < n; i++)
    {
        cur++;
        
        Bone& child = m_bones[cur];
    
        bone.children.push_back(&child);
        readSkeletonRecurse(frames, cur, child, count);
    }
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
