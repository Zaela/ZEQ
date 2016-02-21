
#ifndef _ZEQ_ATTACH_POINT_HPP_
#define _ZEQ_ATTACH_POINT_HPP_

#include "define.hpp"
#include "entity.hpp"
#include "mat4.hpp"

class Skeleton;
class StaticModel;

struct AttachPoint
{
    bool isStatic;
    union
    {
        void*           ptr;
        Entity*         asEntity;
        Skeleton*       asAnimated;
        StaticModel*    asStatic;
    };
    
    enum Slot
    {
        None = -1,
        Camera,
        RightHand,
        LeftHand,
        Shield,
        COUNT
    };
    
    AttachPoint() : isStatic(true), ptr(nullptr) { }
};

class AttachPointSet
{
private:
    AttachPoint m_set[AttachPoint::Slot::COUNT];
    
public:
    void attach(AttachPoint::Slot slot, Skeleton* skele);
    void attach(AttachPoint::Slot slot, StaticModel* item);
    void remove(AttachPoint::Slot slot);

    void setMatrix(AttachPoint::Slot slot, const Mat4& matrix);

    void setAnimation(int animId);
};

#endif//_ZEQ_ATTACH_POINT_HPP_
