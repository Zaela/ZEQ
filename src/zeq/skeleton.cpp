
#include "skeleton.hpp"

Skeleton::~Skeleton()
{
    if (m_vertexBufferSets)
        delete[] m_vertexBufferSets;
    
    if (m_ownedVertexBuffers)
        delete[] m_ownedVertexBuffers;
}

void Skeleton::buildLocalMatrices()
{
    
}

void Skeleton::buildGlobalMatrices()
{
    
}



void Skeleton::draw()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    
    uint32_t lastDiffuseMap = 0;
    for (uint32_t i = 0; i < m_vertexBufferCount; i++)
    {
        VertexBuffer* vb = &m_ownedVertexBuffers[i];
        //int blendType = vb->getBlendType();
        
        //if (blendType == Material::Blend::Invisible)
        //    continue;
        
        uint32_t diffuseMap = vb->getDiffuseMap();
        
        if (diffuseMap != lastDiffuseMap)
        {
            glBindTexture(GL_TEXTURE_2D, diffuseMap);
            lastDiffuseMap = diffuseMap;
        }
        
        vb->draw();
    }
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_TEXTURE_2D);
}
