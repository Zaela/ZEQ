
#ifndef _ZEQ_ANIMATED_MODEL_HPP_
#define _ZEQ_ANIMATED_MODEL_HPP_

#include "define.hpp"
#include "model_prototype.hpp"
#include "vec3.hpp"
#include "quaternion.hpp"
#include "mat4.hpp"
#include "axis_aligned_bounding_box.hpp"
#include "bone_assignment.hpp"
#include "animation.hpp"
#include "skeleton.hpp"
#include "temp_alloc.hpp"
#include <vector>
#include <unordered_map>

class ModelResources;
class MobModelPrototype;

class AnimatedModelPrototype : public ModelPrototype
{
protected:
#pragma pack(1)
    struct DBBone
    {
        uint32_t    childCount;
        Vec3        pos;
        Quaternion  rot;
        Vec3        scale;
    };
#pragma pack()
    
    struct Bone
    {
        bool        hasAnimFrames;
        
        Vec3        pos;
        Quaternion  rot;
        Vec3        scale;
        
        Mat4        globalMatrix;
        Mat4        globalInverseMatrix;
        
        uint32_t    parentIndex;
    };
    
    uint32_t    m_boneCount;
    Bone*       m_bones;
    
    AnimationSet    m_animations;
    
    std::vector<WeightedBoneAssignmentSet>  m_weightedBoneAssignments;
    
private:
    friend class ModelResources;
    friend class MobModelPrototype;
    void readSkeleton(byte* bones, uint32_t len);
    void readSkeletonRecurse(DBBone* frames, uint32_t& cur, Bone& bone, uint32_t count, Bone* parent = nullptr, uint32_t parentIndex = 0);
    void readAnimationFrames(int animId, int boneIndex, byte* frames, uint32_t len);

    WeightedBoneAssignmentSet& readWeightedBoneAssignments(byte* wbas, uint32_t len);

public:
    virtual ~AnimatedModelPrototype();

    virtual Skeleton* createSkeletonInstance();
};

class MobModelPrototype : public AnimatedModelPrototype
{
private:
    struct WeightedHeadModel
    {
        uint32_t count;
        union
        {
            WeightedBoneAssignmentSet   setSingle;
            WeightedBoneAssignmentSet*  setArray;
        };
    };
    
    struct SimpleHeadModel
    {
        uint32_t count;
        union
        {
            VertexBuffer*   vbSingle;
            VertexBuffer**  vbArray;
        };
    };
    
private:
    int     m_race;
    uint8_t m_gender;

    std::vector<WeightedHeadModel>  m_weightedHeads;
    std::vector<SimpleHeadModel>    m_simpleHeads;

private:
    friend class ModelResources;
    void addHeadModel(AnimatedModelPrototype& headModel, int headIndex);

public:
    MobModelPrototype(int race, uint8_t gender);
    virtual ~MobModelPrototype();

    virtual Skeleton* createSkeletonInstance();
};

#endif//_ZEQ_ANIMATED_MODEL_HPP_
