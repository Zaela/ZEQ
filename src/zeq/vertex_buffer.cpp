
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

VertexBuffer::VertexBuffer(VertexBuffer&& vb)
{
    m_id            = vb.m_id;
    m_materialId    = vb.m_materialId;
    m_textureSetId  = vb.m_textureSetId;
    m_vertices      = vb.m_vertices;
    m_count         = vb.m_count;
    m_blendType     = vb.m_blendType;
    m_diffuseId     = vb.m_diffuseId;
    m_vboId         = vb.m_vboId;
    m_animTexture   = vb.m_animTexture;
    
    vb.m_id             = 0;
    vb.m_materialId     = 0;
    vb.m_textureSetId   = 0;
    vb.m_vertices       = nullptr;
    vb.m_count          = 0;
    vb.m_blendType      = 0;
    vb.m_diffuseId      = 0;
    vb.m_vboId          = 0;
    vb.m_animTexture    = nullptr;
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
