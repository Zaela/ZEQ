
#include "entity_list.hpp"

void EntityList::add(Skeleton* skele, const Vec3& pos, bool isClient)
{
    int index = (int)m_animatedBoundingBoxes.size();
    m_animatedBoundingBoxes.push_back(AABB());
    
    skele->setModelIndex(index);
    skele->setPosition(pos);
    
    skele->adjustModelMatrix();
    
    m_animatedModels.push_back(skele);
    
    ModelPosition mp;
    mp.pos      = pos;
    mp.isClient = isClient;
    
    m_animatedModelPositions.push_back(mp);
    
    m_animatedModelFrameDeltas.push_back(0.0f);
}

void EntityList::animateModels(double delta, const Vec3& center)
{
    // Increment animation frames, regardless of position
    for (float& frame : m_animatedModelFrameDeltas)
    {
        frame += delta;
    }
    
    // Animate models; ignore models more than 500 units away
    uint32_t n = m_animatedModelPositions.size();
    
    for (uint32_t i = 0; i < n; i++)
    {
        Vec3 pos    = m_animatedModelPositions[i].pos;
        float dist  = center.getDistanceSquared(pos);
        
        if (dist > 250000.0f)
            continue;
        
        float delta = m_animatedModelFrameDeltas[i];
        m_animatedModelFrameDeltas[i] = 0.0f;
        
        AABB box = m_animatedModels[i]->animate(delta);
        
        box += pos;
        
        m_animatedBoundingBoxes[i] = box;
    }
}

void EntityList::drawModels(Camera& camera)
{
    Frustum& frustum    = camera.getFrustum();
    uint32_t n          = m_animatedBoundingBoxes.size();
    
    for (uint32_t i = 0; i < n; i++)
    {
        if (!frustum.contains(m_animatedBoundingBoxes[i]))
            continue;
        
        Skeleton* skele = m_animatedModels[i];
        if (skele->updateMatrix())
            skele->adjustModelMatrix();
        skele->draw();
    }
}
