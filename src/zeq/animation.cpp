
#include "animation.hpp"

Animation* Animation::create(uint32_t boneCount)
{
    uint32_t excess = sizeof(FrameSet) * boneCount;
    
    byte* data = new byte[sizeof(Animation) + excess];
    
    Animation* anim = (Animation*)data;
    
    anim->m_boneCount   = boneCount;
    anim->m_durationMs  = 0.0f;
    
    memset(anim->m_framesByBoneIndex, 0, excess);
    
    return anim;
}

Animation::~Animation()
{
    for (uint32_t i = 0; i < m_boneCount; i++)
    {
        FrameSet& set = m_framesByBoneIndex[i];
        
        if (set.frames)
            delete[] set.frames;
    }
}

void Animation::getFrameData(float frame, uint32_t boneIndex, Vec3& pos, Quaternion& rot, Vec3& scale, uint32_t& hint)
{
    FrameSet& set = m_framesByBoneIndex[boneIndex];
    
    if (set.count == 0)
        return;
    
    uint32_t index = 0;
    
    for (uint32_t i = hint; i < set.count; i++)
    {
        Frame& fr = set.frames[i];
        
        if (fr.milliseconds >= frame)
        {
            index = i;
            break;
        }
        
        hint++;
    }
    
    Frame& a = set.frames[index];
    Frame& b = set.frames[index - 1];
    
    float f1 = frame - a.milliseconds;
    float f2 = b.milliseconds - frame + f1;
    
    // Position
    pos = ((b.pos - a.pos) / f2) * f1 + a.pos;
    
    // Scale
    scale = ((b.scale - a.scale) / f2) * f1 + a.scale;
    
    // Rotation
    rot.slerp(a.rot, b.rot, f1 / f2);
}

void Animation::readFrameData(int boneIndex, byte* data, uint32_t len)
{
    FrameSet& set   = m_framesByBoneIndex[boneIndex];
    uint32_t count  = len / sizeof(Frame);
    Frame* frames   = (Frame*)data;
    
    set.count   = count;
    set.frames  = frames;
    
    frames = &frames[count - 1];
    
    if (frames->milliseconds > m_durationMs)
        m_durationMs = frames->milliseconds;
}

AnimationSet::AnimationSet()
{
    memset(m_animations, 0, sizeof(m_animations));
}

void AnimationSet::destroy()
{
    for (uint32_t i = 0; i < 72; i++)
    {
        Animation* anim = m_animations[i];
        
        if (anim)
            delete anim;
    }
}

void AnimationSet::inherit(const AnimationSet& set)
{
    for (uint32_t i = 0; i < 72; i++)
    {
        m_animations[i] = set.m_animations[i];
    }
}

Animation* AnimationSet::get(int animId)
{
    if (animId < 1 || animId > 72)
        return nullptr;
    
    return m_animations[animId];
}

void AnimationSet::set(int animId, int boneIndex, uint32_t boneCount, byte* frames, uint32_t len)
{
    Animation* anim = m_animations[animId];
    
    if (!anim)
    {
        anim = Animation::create(boneCount);
        m_animations[animId] = anim;
    }
    
    anim->readFrameData(boneIndex, frames, len);
}
