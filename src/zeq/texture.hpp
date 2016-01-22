
#ifndef _ZEQ_TEXTURE_HPP_
#define _ZEQ_TEXTURE_HPP_

#include "define.hpp"
#include "timer.hpp"
#include "bit.hpp"
#include "ref_counter.hpp"
#include <vector>

class VertexBuffer;

class Texture : public RefCounter
{
private:
    int64_t m_id;

    uint32_t m_diffuseMap;
    uint32_t m_normalMap;
    uint32_t m_width;
    uint32_t m_height;

public:
    Texture(int64_t id, uint32_t diffuse, uint32_t normal, int width, int height);
    virtual ~Texture();

    uint32_t getDiffuseMap()    { return m_diffuseMap; }
    uint32_t getNormalMap()     { return m_normalMap; }
    uint32_t width() const      { return m_width; }
    uint32_t height() const     { return m_height; }
};

// Also used by non-animated textures,for consistency; non-animated textures are simply a set of one
class AnimatedTexture : public RefCounter
{
private:
    int64_t m_id;

    Timer* m_animTimer;
    // VertexBuffers that reference this animated texture, need to update them when the current texture changes
    std::vector<VertexBuffer*> m_referencingVertexBuffers;

    uint16_t    m_currentIndex;
    uint16_t    m_count;

    union
    {
        Texture*    m_textureSingle;
        Texture**   m_textureArray;
    };
    
private:
    static void timerCallback(Timer* timer);

public:
    AnimatedTexture(int64_t id, uint32_t animDelay);
    virtual ~AnimatedTexture();

    void        addTexture(Texture* tex);
    Texture*    getTexture();

    void addReferencingVertexBuffer(VertexBuffer* vb);
    void removeReferencingVertexBuffer(VertexBuffer* vb);
};

#endif//_ZEQ_TEXTURE_HPP_
