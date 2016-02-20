
#include "entity_list.hpp"

extern Log gLog;

void EntityList::add(Skeleton* skele, const Vec3& pos)
{
    int index = (int)m_animatedBoundingBoxes.size();
    m_animatedBoundingBoxes.push_back(AABB());
    
    skele->setBoundingBoxIndex(index);
    skele->setPosition(pos);
    
    if (skele->isEQG())
        skele->adjustForEQG();
    else
        skele->adjustForWLD();
    
    m_animatedModels.push_back(skele);
    
    ModelPosition mp;
    mp.pos = pos;
    mp.isClient = false; //fixme
    
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
        Vec3& pos   = m_animatedModelPositions[i].pos;
        float dist  = center.getDistanceSquared(pos);
        
        if (dist > 250000.0f)
            continue;
        
        float delta = m_animatedModelFrameDeltas[i];
        m_animatedModelFrameDeltas[i] = 0.0f;
        
        Skeleton* skele = m_animatedModels[i];
        AABB box = skele->animate(delta);
        
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
        skele->draw();
    }
}
