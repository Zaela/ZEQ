
#ifndef _ZEQ_ZONE_MODEL_HPP_
#define _ZEQ_ZONE_MODEL_HPP_

#include "define.hpp"
#include "model_prototype.hpp"
#include "vertex_buffer.hpp"
#include "axis_aligned_bounding_box.hpp"
#include "temp.hpp"
#include "camera.hpp"
#include "opengl.hpp"
#include "config.hpp"
#include "material.hpp"

#define SMALL_OCTREE_NODE_MAX_RADIUS (250.0f * 250.0f)

class ModelResources;

class ZoneModel : public ModelPrototype
{
private:
    struct OctreeTempSet
    {
        TempVector<AABB>            boundingBoxes;
        TempVector<VertexBuffer*>   vertexBuffers;
    };
    
    struct OctreeTemp
    {
        TempVector<uint32_t>    claimed;
        OctreeTempSet           sets[Material::Blend::COUNT];
        
        void placeVertexBuffer(VertexBuffer* vb, AABB& box)
        {
            int i = vb->getBlendType();
            sets[i].boundingBoxes.push_back(box);
            sets[i].vertexBuffers.push_back(vb);
        }
    };
    
    int64_t m_id;
    
    std::vector<AABB>           m_boundingBoxes;
    std::vector<VertexBuffer*>  m_vertexBuffers;
    
    static const uint32_t MAX_TRIANGLES_PER_NODE = 512;
    
private:
    static void octreeRecurse(OctreeTemp& temp, VertexBuffer* vb, VertexBuffer::Triangle* triArray, uint32_t vbTriCount,
        AABB& parentBox, uint32_t parentId, uint32_t parentTriCount, uint32_t& curClaim);
    static void octreeMakeNode(OctreeTemp& temp, VertexBuffer* vb, VertexBuffer::Triangle* triArray, uint32_t triCount, uint32_t claim);
    static bool octreeCheckSmallVertexBuffer(VertexBuffer* vb, OctreeTemp& temp);

    static void setBlendType(int blendType);
    static void unsetBlendType(int blendType);

    friend class ModelResources;
    std::vector<AABB>&          getBoundingBoxes() { return m_boundingBoxes; }
    std::vector<VertexBuffer*>& getVertexBuffers() { return m_vertexBuffers; }
    
public:
    ZoneModel(int64_t id);
    virtual ~ZoneModel();

    int64_t getId() const { return m_id; }

    void generateOctree();
    
    void draw(Camera* camera);
};

#endif//_ZEQ_ZONE_MODEL_HPP_
