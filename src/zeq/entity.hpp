
#ifndef _ZEQ_ENTITY_HPP_
#define _ZEQ_ENTITY_HPP_

#include "define.hpp"
#include "mat4.hpp"
#include "vec3.hpp"

class Entity
{
private:
    bool    m_needsUpdate;
    int     m_modelIndex;
    Mat4    m_modelMatrix;
    Vec3    m_position;

public:
    Entity() : m_needsUpdate(false), m_modelIndex(-1) { }
    virtual ~Entity() { }
    
    bool updateMatrix();
    
    int     getModelIndex() const { return m_modelIndex; }
    void    setModelIndex(int index) { m_modelIndex = index; }
    
    void setPosition(const Vec3& pos);
    
    void adjustForWLD() { m_modelMatrix = m_modelMatrix.adjustForWLD(); }
    void adjustForEQG() { m_modelMatrix = m_modelMatrix.adjustForEQG(); }
    
    void    setModelMatrix(const Mat4& matrix);
    Mat4&   getModelMatrix() { return m_modelMatrix; }
};

#endif//_ZEQ_ENTITY_HPP_