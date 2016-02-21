
#include "attach_point.hpp"
#include "skeleton.hpp"

void AttachPointSet::attach(AttachPoint::Slot slot, Skeleton* skele)
{
    AttachPoint& ap = m_set[slot];
    ap.isStatic     = false;
    ap.asAnimated   = skele;
}

void AttachPointSet::attach(AttachPoint::Slot slot, StaticModel* item)
{
    AttachPoint& ap = m_set[slot];
    ap.isStatic     = true;
    ap.asStatic     = item;
}

void AttachPointSet::remove(AttachPoint::Slot slot)
{
    AttachPoint& ap = m_set[slot];
    Entity* ent     = ap.asEntity;
    ap.isStatic     = true;
    ap.asEntity     = nullptr;
    
    delete ent;
}

void AttachPointSet::setMatrix(AttachPoint::Slot slot, const Mat4& matrix)
{
    Entity* ent = m_set[slot].asEntity;
    
    if (ent)
        ent->setModelMatrix(matrix);
}

void AttachPointSet::setAnimation(int animId)
{
    for (int i = 0; i < AttachPoint::Slot::COUNT; i++)
    {
        AttachPoint& ap = m_set[i];
        
        if (ap.isStatic || !ap.ptr)
            continue;
        
        ap.asAnimated->setAnimation(animId);
    }
}
