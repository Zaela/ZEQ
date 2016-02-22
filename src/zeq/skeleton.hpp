
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
#include "entity.hpp"
#include "axis_aligned_bounding_box.hpp"
#include "material.hpp"
#include "attach_point.hpp"

class AnimatedModelPrototype;
class MobModelPrototype;

class Skeleton : public Entity
{
private:
    friend class AnimatedModelPrototype;
    friend class MobModelPrototype;

    struct Bone
    {
        bool        hasAnimFrames;
        
        Vec3        pos;
        Quaternion  rot;
        Vec3        scale;
        
        Mat4        globalAnimMatrix;
        Mat4        globalInverseMatrix;
        
        Mat4*       parentGlobalAnimMatrix;
        
        uint32_t    animHint;
        int         attachPointSlot;
    };
    
    struct SimpleVertexBufferSet
    {
        uint32_t                    vertexCount;
        const VertexBuffer::Vertex* base;
        VertexBuffer::Vertex*       target;
    };
    
    struct VertexBufferSet : public SimpleVertexBufferSet
    {
        uint32_t                    assignmentCount;
        WeightedBoneAssignment*     assignments;
    };
    
    uint32_t    m_boneCount;
    Bone*       m_bones;
    Mat4*       m_animMatrices;
    
    std::vector<VertexBufferSet>        m_vertexBufferSets;
    std::vector<SimpleVertexBufferSet>  m_simpleVertexBufferSets;
    std::vector<VertexBuffer>           m_ownedVertexBuffers;

    AnimationSet    m_animations;
    AttachPointSet  m_attachPoints;
    
    AnimatedModelPrototype* m_prototype;
    
    // Active animation info
    int         m_curAnimId; 
    Animation*  m_curAnim;
    float       m_curAnimDuration;
    float       m_curAnimFrame;
    
private:
    void buildMatrices();
    AABB moveVerticesEQG();
    AABB moveVerticesWLD();

    float incrementAnimation(float delta);
    
public:
    Skeleton();
    virtual ~Skeleton();

    void setAnimation(int animId);
    bool isAnimating() const { return m_curAnim != nullptr; }
    AABB animate(float delta);

    bool isEQG() const { return !m_vertexBufferSets.empty(); }
    void adjustModelMatrix();
    
    void attach(AttachPoint::Slot slot, Skeleton* model);

    void draw();
};

#endif//_ZEQ_SKELETON_HPP_
