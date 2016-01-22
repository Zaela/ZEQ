
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
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    
    for (VertexBuffer* vb : m_referencedVertexBuffers)
    {
        vb->draw();
    }
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_TEXTURE_2D);
}
