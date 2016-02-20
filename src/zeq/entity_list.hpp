
#ifndef _ZEQ_ENTITY_LIST_HPP_
#define _ZEQ_ENTITY_LIST_HPP_

#include "define.hpp"
#include "entity.hpp"
#include "skeleton.hpp"
#include "camera.hpp"
#include "axis_aligned_bounding_box.hpp"
#include "vec3.hpp"
#include "mat4.hpp"
#include "log.hpp"
#include <vector>

class EntityList
{
private:
    struct ModelPosition
    {
        Vec3 pos;
        bool isClient;
    };
    
private:
    // The following fields contain data mainly related to drawing, i.e. in as much as an entity is represented by a model.
    // The data is split up to facilitate high-efficiency iteration.

    // Bounding boxes are used for fast frustum culling of entire models
    std::vector<AABB>           m_animatedBoundingBoxes;
    std::vector<float>          m_animatedModelFrameDeltas;
    std::vector<Skeleton*>      m_animatedModels;
    std::vector<ModelPosition>  m_animatedModelPositions;

public:
    void add(Skeleton* ent, const Vec3& pos = Vec3(0.0f, 0.0f, 0.0f));

    void animateModels(double delta, const Vec3& center);
    void drawModels(Camera& camera);
};

#endif//_ZEQ_ENTITY_LIST_HPP_
