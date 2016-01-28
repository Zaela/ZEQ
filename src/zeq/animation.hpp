
#ifndef _ZEQ_ANIMATION_HPP_
#define _ZEQ_ANIMATION_HPP_

#include "define.hpp"
#include "vec3.hpp"
#include "quaternion.hpp"

class AnimationSet;

class Animation
{
public:
#pragma pack(1)
    struct Frame
    {
        float       milliseconds;
        Vec3        pos;
        Quaternion  rot;
        Vec3        scale;
    };
#pragma pack()
    
    struct FrameSet
    {
        uint32_t    count;
        Frame*      frames;
    };
    
private:
    uint32_t    m_boneCount;
    float       m_durationMs;
    FrameSet    m_framesByBoneIndex[1]; // Stub -- THIS MUST BE THE LAST DATA MEMBER OF THIS CLASS

private:
    friend class AnimationSet;
    void readFrameData(int boneIndex, byte* data, uint32_t len);
    
public:
    static Animation* create(uint32_t boneCount);
    ~Animation();
};

class AnimationSet
{
private:
    Animation* m_animations[72]; // Find out how many player models actually have

public:
    AnimationSet();

    // Should only be called from the AnimatedModelPrototype that owns the Animation definitions
    void destroy();
    void inherit(const AnimationSet& set);

    Animation*  get(int animId);
    void        set(int animId, int boneIndex, uint32_t boneCount, byte* frame, uint32_t len);
};

#endif//_ZEQ_ANIMATION_HPP_
