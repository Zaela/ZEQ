
#include "entity.hpp"

bool Entity::updateMatrix()
{
    if (!m_needsUpdate)
        return false;
    
    m_needsUpdate = false;
    m_modelMatrix.translate(m_position);
    return true;
}

void Entity::setPosition(const Vec3& pos)
{
    m_position = pos;
    m_modelMatrix.setTranslation(pos);
}

void Entity::setModelMatrix(const Mat4& matrix)
{
    m_modelMatrix = matrix;
    m_needsUpdate = true;
}
