
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

class ModelResources;

class AnimatedModelPrototype : public ModelPrototype
{
public:
#pragma pack(1)
    struct DBFrame
    {
        uint32_t    childCount;
        Vec3        pos;
        Quaternion  rot;
        Vec3        scale;
    };
#pragma pack()

private:
    struct Bone
    {
        Vec3        pos;
        Vec3        scale;
        Quaternion  rot;
        
        Bone*       parent;
        
        Mat4        localMatrix;
        Mat4        globalMatrix;
        Mat4        globalAnimMatrix;
        Mat4        globalInverseMatrix;
        
        std::vector<Bone*> children;
        
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
    void readSkeleton(byte* frames, uint32_t len);
    void readSkeletonRecurse(DBFrame* frames, uint32_t& cur, Bone& bone, uint32_t count);

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
