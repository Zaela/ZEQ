
#ifndef _ZEQ_ENTITY_HPP_
#define _ZEQ_ENTITY_HPP_

#include "define.hpp"
#include "mat4.hpp"
#include "vec3.hpp"

class Entity
{
private:
    int     m_boundingBoxIndex;
    Mat4    m_modelMatrix;

public:
    Entity() : m_boundingBoxIndex(-1) { }
    
    int     getBoundingBoxIndex() const { return m_boundingBoxIndex; }
    void    setBoundingBoxIndex(int index) { m_boundingBoxIndex = index; }
    
    void setPosition(const Vec3& pos) { m_modelMatrix.setTranslation(pos); }
    
    void adjustForWLD() { m_modelMatrix = m_modelMatrix.adjustForWLD(); }
    void adjustForEQG() { m_modelMatrix = m_modelMatrix.adjustForEQG(); }
    
    Mat4& getModelMatrix() { return m_modelMatrix; }
};

#endif//_ZEQ_ENTITY_HPP_