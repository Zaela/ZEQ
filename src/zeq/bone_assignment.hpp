
#ifndef _ZEQ_WEIGHTED_BONE_ASSIGNMENT_HPP_
#define _ZEQ_WEIGHTED_BONE_ASSIGNMENT_HPP_

#include "define.hpp"
#include "vertex_buffer.hpp"

#pragma pack(1)

struct WeightedBoneAssignment
{
    uint32_t    vertIndex;
    uint32_t    boneIndex;
    float       weight;
};

#pragma pack()

struct WeightedBoneAssignmentSet
{
    uint32_t                count;
    WeightedBoneAssignment* assignments;
    VertexBuffer*           vertexBuffer;
};

struct BoneAssignmentSet
{
    uint32_t        count;
    uint32_t*       assignments;
    VertexBuffer*   vertexBuffer;
};

#endif//_ZEQ_WEIGHTED_BONE_ASSIGNMENT_HPP_
