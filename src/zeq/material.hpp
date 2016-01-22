
#ifndef _ZEQ_MATERIAL_HPP_
#define _ZEQ_MATERIAL_HPP_

#include "define.hpp"

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
};

#endif//_ZEQ_MATERIAL_HPP_
