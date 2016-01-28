
#ifndef _ZEQ_VERTEX_BUFFER_HPP_
#define _ZEQ_VERTEX_BUFFER_HPP_

#include "define.hpp"
#include "ref_counter.hpp"
#include "texture.hpp"
#include "opengl.hpp"

class VertexBuffer : public RefCounter
{
public:
    struct Vertex
    {
        float x, y, z;
        float i, j, k;
        float u, v;
        //uint8_t r, g, b, a;
    };
    
    struct Triangle
    {
        Vertex points[3];
    };
    
private:
    int64_t     m_id;
    int64_t     m_materialId;
    int64_t     m_textureSetId;
    Vertex*     m_vertices;
    uint32_t    m_count;
    
    int         m_blendType;
    uint32_t    m_diffuseId;
    uint32_t    m_vboId;
    
    AnimatedTexture*    m_animTexture;
    
public:
    VertexBuffer();
    VertexBuffer(int64_t, byte* rawData, uint32_t len);
    VertexBuffer(uint32_t vbo, uint32_t count);
    virtual ~VertexBuffer();

    void copy(const VertexBuffer& vb);

    int64_t getId() const { return m_id; }
    int64_t getMaterialId() const { return m_materialId; }
    int64_t getTextureSetId() const { return m_textureSetId; }

    void draw();

    uint32_t count() const { return m_count; }
    uint32_t triangleCount() const { return count() / 3; }
    uint32_t bytes() const { return m_count * sizeof(Vertex); }
    
    Vertex*     array() const { return m_vertices; }
    Triangle*   triangleArray() const { return (Triangle*)m_vertices; }
    
    void setAnimatedTexture(AnimatedTexture* animTex);
    void setDiffuseMap(uint32_t id) { m_diffuseId = id; }
    void setBlendType(int64_t materialId, int64_t textureSetId, int type) { m_materialId = materialId; m_textureSetId = textureSetId; m_blendType = type; }
    
    AnimatedTexture*    getAnimatedTexture() const { return m_animTexture; }
    uint32_t            getDiffuseMap() const { return m_diffuseId; }
    int                 getBlendType() const { return m_blendType; }
    
    void registerWithOpenGL(bool isDynamic = false);
};

#endif//_ZEQ_VERTEX_BUFFER_HPP_
