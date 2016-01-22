
#ifndef _ZEQ_MODEL_PROTOTYPE_HPP_
#define _ZEQ_MODEL_PROTOTYPE_HPP_

#include "define.hpp"
#include "texture.hpp"
#include "vertex_buffer.hpp"
#include "ref_counter.hpp"
#include <vector>

class ModelResources;

class ModelPrototype : public RefCounter
{
private:
    std::vector<Texture*>           m_referencedTextures;
    std::vector<AnimatedTexture*>   m_referencedAnimatedTextures;
    std::vector<VertexBuffer*>      m_referencedVertexBuffers;

protected:
    friend class ModelResources;
    std::vector<VertexBuffer*>& getReferencedVertexBuffers() { return m_referencedVertexBuffers; }
    
    void clearReferencedVertexBuffers();
    
public:
    virtual ~ModelPrototype();

    void addTexture(Texture* tex) { m_referencedTextures.push_back(tex); }
    void addAnimatedTexture(AnimatedTexture* animTex) { m_referencedAnimatedTextures.push_back(animTex); }
    void addVertexBuffer(VertexBuffer* vb) { m_referencedVertexBuffers.push_back(vb); }
    
    void draw(); //remove later
};

#endif//_ZEQ_MODEL_PROTOTYPE_HPP_
