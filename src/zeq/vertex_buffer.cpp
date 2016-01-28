
#include "vertex_buffer.hpp"
#include "model_resources.hpp"

extern ModelResources gModelResources;

VertexBuffer::VertexBuffer()
    : m_id(0),
      m_materialId(0),
      m_textureSetId(0),
      m_vertices(nullptr),
      m_count(0),
      m_blendType(0),
      m_diffuseId(0),
      m_vboId(0),
      m_animTexture(nullptr)
{

}

VertexBuffer::VertexBuffer(int64_t id, byte* rawData, uint32_t len)
    : m_id(id),
      m_materialId(0),
      m_textureSetId(0),
      m_vertices((Vertex*)rawData),
      m_count(len / sizeof(Vertex)),
      m_blendType(0),
      m_diffuseId(0),
      m_vboId(0),
      m_animTexture(nullptr)
{

}

VertexBuffer::VertexBuffer(uint32_t vbo, uint32_t count)
    : m_id(0),
      m_materialId(0),
      m_textureSetId(0),
      m_vertices(nullptr),
      m_count(count),
      m_blendType(0),
      m_diffuseId(0),
      m_vboId(vbo),
      m_animTexture(nullptr)
{
    
}

VertexBuffer::~VertexBuffer()
{
    if (m_id)
        gModelResources.removeVertexBuffer(m_id);
    
    if (m_vboId)
        OpenGL::deleteVBO(m_vboId);
    
    if (m_animTexture)
        m_animTexture->removeReferencingVertexBuffer(this);
    
    if (m_vertices)
        delete[] m_vertices;
}

void VertexBuffer::copy(const VertexBuffer& vb)
{
    uint32_t count      = vb.m_count;
    Vertex* vertices    = new Vertex[count];
    
    memcpy(vertices, vb.m_vertices, sizeof(Vertex) * count);
    
    m_vertices  = vertices;
    m_count     = count;
    m_blendType = vb.m_blendType;
    
    setAnimatedTexture(vb.m_animTexture);
}

void VertexBuffer::setAnimatedTexture(AnimatedTexture* animTex)
{
    if (!animTex)
        return;
    
    setDiffuseMap(animTex->getTexture()->getDiffuseMap());
    
    m_animTexture = animTex;
    animTex->addReferencingVertexBuffer(this);
}

void VertexBuffer::registerWithOpenGL(bool isDynamic)
{
    if (!m_vertices)
        return;
    
    uint32_t id = OpenGL::generateVBO(m_vertices, m_count * sizeof(Vertex), isDynamic);
    
    if (id)
    {
        m_vboId = id;
        
        if (!isDynamic)
        {
            delete[] m_vertices;
            m_vertices = nullptr;
        }
    }
}

void VertexBuffer::draw()
{
    byte* ptr;
    
    if (m_vboId)
    {
        OpenGL::bindVBO(m_vboId);
        ptr = nullptr;
    }
    else
    {
        OpenGL::bindVBO(0);
        ptr = (byte*)m_vertices;
    }
    
    glVertexPointer(3, GL_FLOAT,    sizeof(Vertex), ptr);
    glNormalPointer(GL_FLOAT,       sizeof(Vertex), ptr + 3 * sizeof(float));
    glTexCoordPointer(2, GL_FLOAT,  sizeof(Vertex), ptr + 6 * sizeof(float));
    
    glDrawArrays(GL_TRIANGLES, 0, m_count);
    assert(!glGetError());
}
