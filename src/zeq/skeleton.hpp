
#ifndef _ZEQ_SKELETON_HPP_
#define _ZEQ_SKELETON_HPP_

#include "define.hpp"
#include "vec3.hpp"
#include "quaternion.hpp"
#include "mat4.hpp"
#include "vertex_buffer.hpp"
#include "animation.hpp"
#include "bone_assignment.hpp"
#include "temp_alloc.hpp"

class AnimatedModelPrototype;

class Skeleton
{
private:
    friend class AnimatedModelPrototype;

    struct Bone
    {
        bool        hasAnimFrames;
        
        Vec3        pos;
        Quaternion  rot;
        Vec3        scale;
        
        Mat4        localMatrix;
        Mat4        globalMatrix;
        Mat4        localAnimMatrix;
        Mat4        globalAnimMatrix;
        Mat4        globalInverseMatrix;
        
        uint32_t    childCount;
        uint32_t*   children;
        
        uint32_t    animHint;
    };
    
    struct VertexBufferSet
    {
        uint32_t                    vertexCount;
        const VertexBuffer::Vertex* base;
        VertexBuffer::Vertex*       target;
        uint32_t                    assignmentCount;
        WeightedBoneAssignment*     assignments;
    };
    
    //uint32_t    m_boundingBoxIndex;
    
    uint32_t    m_boneCount;
    Bone*       m_bones;
    
    uint32_t            m_vertexBufferCount;
    VertexBufferSet*    m_vertexBufferSets;
    VertexBuffer*       m_ownedVertexBuffers;
    
    AnimationSet m_animations;
    
    // Active animation info
    int         m_curAnimId; 
    Animation*  m_curAnim;
    float       m_curAnimDuration;
    float       m_curAnimFrame;
    
private:
    void buildLocalMatrices();
    void buildGlobalMatrices();
    void buildGlobalMatrixRecurse(Bone& bone);
    
public:
    Skeleton();
    ~Skeleton();

    void setAnimation(int animId);
    void animate(double delta);

    void draw();
};

#endif//_ZEQ_SKELETON_HPP_
