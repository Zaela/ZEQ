
#ifndef _ZEQ_WEIGHTED_BONE_ASSIGNMENT_HPP_
#define _ZEQ_WEIGHTED_BONE_ASSIGNMENT_HPP_

#include "define.hpp"

#pragma pack(1)

struct WeightedBoneAssignment
{
    uint32_t    vertIndex;
    uint32_t    boneIndex;
    float       weight;
};

#pragma pack()

#endif//_ZEQ_WEIGHTED_BONE_ASSIGNMENT_HPP_
