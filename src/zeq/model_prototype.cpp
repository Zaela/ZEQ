
#include "model_prototype.hpp"

ModelPrototype::~ModelPrototype()
{
    for (VertexBuffer* vb : m_referencedVertexBuffers)
    {
        vb->drop();
    }
    
    for (AnimatedTexture* animTex : m_referencedAnimatedTextures)
    {
        animTex->drop();
    }
    
    for (Texture* tex : m_referencedTextures)
    {
        tex->drop();
    }
}

void ModelPrototype::clearReferencedVertexBuffers()
{
    for (VertexBuffer* vb : m_referencedVertexBuffers)
    {
        vb->drop();
    }
    
    m_referencedVertexBuffers.clear();
}

void ModelPrototype::draw()
{
    MATERIAL_SETUP();
    
    for (VertexBuffer* vb : m_referencedVertexBuffers)
    {
        int blendType = vb->getBlendType();
        
        if (blendType == Material::Blend::Invisible)
            continue;
        
        uint32_t diffuseMap = vb->getDiffuseMap();
        
        MATERIAL_SET(diffuseMap, blendType);
        
        vb->draw();
    }
    
    MATERIAL_CLEANUP();
}
