
#include "camera.hpp"

extern Config gConfig;

Camera::Camera()
    : m_fieldOfViewRadians(Math::toRadians(45)),
      m_aspectRatio(gConfig.getDouble(Config::ScreenWidth, 800.0f) / gConfig.getDouble(Config::ScreenHeight, 600.0f)),
      m_nearZ(0.1f),
      m_farZ(250.0f),
      m_position(0.0f, 0.0f, 0.0f),
      m_target(0.0f, 0.0f, 100.0f),
      m_up(0.0f, 1.0f, 0.0f)
{
    recalculate();
}

void Camera::recalculateView()
{
    m_viewMatrix = Mat4::lookAt(m_position, m_target, m_up);
    
    m_frustum.set(m_projectionMatrix * m_viewMatrix);
}

void Camera::recalculatePerspective()
{
    m_projectionMatrix = Mat4::perspective(m_fieldOfViewRadians, m_aspectRatio, m_nearZ, m_farZ);
}

void Camera::recalculate()
{
    recalculatePerspective();
    recalculateView();
}

void Camera::applyView()
{
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(m_viewMatrix.ptr());
    
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(m_projectionMatrix.ptr());
}
