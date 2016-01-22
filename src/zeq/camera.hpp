
#ifndef _ZEQ_CAMERA_HPP_
#define _ZEQ_CAMERA_HPP_

#include "define.hpp"
#include "frustum.hpp"
#include "vec3.hpp"
#include "mat4.hpp"
#include "math.hpp"
#include "config.hpp"

class Camera
{
private:
    Frustum m_frustum;
    float   m_fieldOfViewRadians;
    float   m_aspectRatio;
    float   m_nearZ;
    float   m_farZ;
    Vec3    m_position;
    Vec3    m_target;
    Vec3    m_up;
    Mat4    m_viewMatrix;
    Mat4    m_projectionMatrix;

public:
    Camera();

    void recalculateView();
    void recalculatePerspective();
    void recalculate();

    void applyView();

    Frustum&    getFrustum() { return m_frustum; }
    Vec3&       getPosition() { return m_position; }
    Vec3&       getTarget() { return m_target; }
    Vec3&       getUpVector() { return m_up; }
    
    void setPosition(Vec3& pos) { m_position = pos; }
    void setTarget(Vec3& target) { m_target = target; }
};

#endif//_ZEQ_CAMERA_HPP_
