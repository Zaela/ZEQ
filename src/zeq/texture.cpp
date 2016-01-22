
#include "texture.hpp"
#include "vertex_buffer.hpp"
#include "model_resources.hpp"

extern ModelResources gModelResources;

Texture::Texture(int64_t id, uint32_t diffuse, uint32_t normal, int width, int height)
    : m_id(id),
      m_diffuseMap(diffuse),
      m_normalMap(normal),
      m_width((uint32_t)width),
      m_height((uint32_t)height)
{
        
}

Texture::~Texture()
{
    gModelResources.removeTexture(m_id);
    
    if (m_diffuseMap)
        glDeleteTextures(1, &m_diffuseMap);
    
    if (m_normalMap)
        glDeleteTextures(1, &m_normalMap);
}

AnimatedTexture::AnimatedTexture(int64_t id, uint32_t animDelay)
    : m_id(id),
      m_animTimer(nullptr),
      m_currentIndex(0),
      m_count(0),
      m_textureSingle(nullptr)
{
    if (animDelay != 0)
    {
        m_animTimer = new Timer(animDelay, timerCallback, this);
    }
}

AnimatedTexture::~AnimatedTexture()
{
    gModelResources.removeAnimatedTexture(m_id);
    
    if (m_animTimer)
        delete m_animTimer;
    
    if (m_count > 1)
        delete[] m_textureArray;
}

void AnimatedTexture::timerCallback(Timer* timer)
{
    AnimatedTexture* animTex = (AnimatedTexture*)timer->getUserData();
    
    uint32_t index = animTex->m_currentIndex + 1;
    
    if (index == animTex->m_count)
        index = 0;
    
    animTex->m_currentIndex = index;
    
    uint32_t diffuseId = animTex->getTexture()->getDiffuseMap();
    
    // Do updates
    for (VertexBuffer* vb : animTex->m_referencingVertexBuffers)
    {
        vb->setDiffuseMap(diffuseId);
    }
}

void AnimatedTexture::addTexture(Texture* tex)
{
    uint16_t index = m_count++;
    
    switch (index)
    {
    case 0:
    {
        m_textureSingle = tex;
        break;
    }
    case 1:
    {
        Texture** array = new Texture*[2];
        array[0] = m_textureSingle;
        array[1] = tex;
        m_textureArray = array;
        break;
    }
    default:
        if (ispow2(index))
        {
            Texture** array = new Texture*[index * 2];
            memcpy(array, m_textureArray, index * sizeof(Texture*));
            delete[] m_textureArray;
            m_textureArray = array;
        }
        
        m_textureArray[index] = tex;
        break;
    }
}

Texture* AnimatedTexture::getTexture()
{
    if (m_count == 1)
        return m_textureSingle;
    
    return m_textureArray[m_currentIndex];
}

void AnimatedTexture::addReferencingVertexBuffer(VertexBuffer* vb)
{
    m_referencingVertexBuffers.push_back(vb);
}

void AnimatedTexture::removeReferencingVertexBuffer(VertexBuffer* vb)
{
    for (uint32_t i = 0; i < m_referencingVertexBuffers.size(); i++)
    {
        if (m_referencingVertexBuffers[i] == vb)
        {
            m_referencingVertexBuffers[i] = m_referencingVertexBuffers.back();
            m_referencingVertexBuffers.pop_back();
            return;
        }
    }
}
