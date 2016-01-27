
#ifndef _ZEQ_ANIMATED_MODEL_HPP_
#define _ZEQ_ANIMATED_MODEL_HPP_

#include "define.hpp"
#include "model_prototype.hpp"
#include "vec3.hpp"
#include "quaternion.hpp"
#include "mat4.hpp"
#include "axis_aligned_bounding_box.hpp"
#include "weighted_bone_assignment.hpp"
#include <vector>
#include <unordered_map>

class ModelResources;

class AnimatedModelPrototype : public ModelPrototype
{
private:
#pragma pack(1)
    struct DBBone
    {
        uint32_t    childCount;
        Vec3        pos;
        Quaternion  rot;
        Vec3        scale;
    };

    struct Frame
    {
        float       milliseconds;
        Vec3        pos;
        Quaternion  rot;
        Vec3        scale;
    };
#pragma pack()
    
    struct Bone
    {
        Vec3        pos;
        Quaternion  rot;
        Vec3        scale;
        
        Bone*       parent;
        
        Mat4        localMatrix;
        Mat4        globalMatrix;
        Mat4        globalAnimMatrix;
        Mat4        globalInverseMatrix;
        
        std::vector<Bone*> children;
        std::unordered_map<int, std::vector<Frame>> frames;
        
        Bone()
        {
            parent = nullptr;
        }
    };
    
    std::vector<Bone> m_bones;
    
    //maybe have these as vectors of flat arrays, with a separate count for each
    std::vector<std::vector<WeightedBoneAssignment>>    m_weightedBoneAssignments;
    std::vector<std::vector<uint32_t>>                  m_boneAssignments;
    
private:
    friend class ModelResources;
    void readSkeleton(byte* bones, uint32_t len);
    void readSkeletonRecurse(DBBone* frames, uint32_t& cur, Bone& bone, uint32_t count);
    void readAnimationFrames(int animId, int boneIndex, byte* frames, uint32_t len);

    std::vector<WeightedBoneAssignment>&    readWeightedBoneAssignments(byte* wbas, uint32_t len);
    std::vector<uint32_t>&                  readBoneAssignments(byte* bas, uint32_t len);

public:
    virtual ~AnimatedModelPrototype() { }
};

class MobModelPrototype : public AnimatedModelPrototype
{
private:
    int     m_race;
    uint8_t m_gender;

public:
    MobModelPrototype(int race, uint8_t gender);
    virtual ~MobModelPrototype();
};

#endif//_ZEQ_ANIMATED_MODEL_HPP_
