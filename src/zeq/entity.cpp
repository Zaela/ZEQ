
#include "entity.hpp"

Entity::Entity()
    : m_modelIndex(-1),
      m_parentMatrix(nullptr)
{
    
}

void Entity::setPosition(const Vec3& pos)
{
    m_modelMatrix.setTranslation(pos);
}

void Entity::setModelMatrix(const Mat4& matrix)
{
    m_modelMatrix = matrix;
}

Mat4 Entity::getAttachedMatrix()
{
    Mat4 mat = *m_parentMatrix;
    Vec3 rot = mat.getRotationNoScale();
    
    mat[12] = -mat[12];
    mat[13] = -mat[13];
    
    //if (rot.x > 0) rot.x -= 3.14159f; else rot.x += 3.14159f;
    //if (rot.y > 0) rot.y -= 3.14159f; else rot.y += 3.14159f;
    if (rot.z > 0) rot.z -= 3.14159f; else rot.z += 3.14159f;

    mat.setRotationRadians(rot);
    
    return m_modelMatrix * mat;
}
