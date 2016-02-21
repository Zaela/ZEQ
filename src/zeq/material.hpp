
#ifndef _ZEQ_MATERIAL_HPP_
#define _ZEQ_MATERIAL_HPP_

#include "define.hpp"
#include "opengl.hpp"

#define MATERIAL_SETUP()                                \
    int _lastBlendType = -1;                            \
    uint32_t _lastDiffuseMap = 0
    
#define MATERIAL_CLEANUP()                              \
    Material::unsetBlendType(_lastBlendType)

#define MATERIAL_SET(map, blend)                        \
    if (map != _lastDiffuseMap)                         \
    {                                                   \
        glBindTexture(GL_TEXTURE_2D, map);              \
        _lastDiffuseMap = map;                          \
    }                                                   \
    if (blend != _lastBlendType)                        \
    {                                                   \
        Material::unsetBlendType(_lastBlendType);       \
        Material::setBlendType(blendType);              \
        _lastBlendType = blendType;                     \
    }

class Material
{
public:
    enum Blend
    {
        Solid,
        Masked,
        Particle,
        Additive,
        Invisible,
        COUNT
    };
    
public:
    static void setBlendType(int type);
    static void unsetBlendType(int type);
};

#endif//_ZEQ_MATERIAL_HPP_
