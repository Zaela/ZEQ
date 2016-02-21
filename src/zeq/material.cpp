
#include "material.hpp"

void Material::setBlendType(int blendType)
{
    switch (blendType)
    {
    case Material::Blend::Particle:
        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
        // Fallthrough
    case Material::Blend::Masked:
        glEnable(GL_ALPHA_TEST);
        glAlphaFunc(GL_GEQUAL, 0.5f);
        break;
    case Material::Blend::Additive:
        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_SRC_COLOR);
        break;
    default:
        break;
    }
}

void Material::unsetBlendType(int blendType)
{
    switch (blendType)
    {
    case Material::Blend::Particle:
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        // Fallthrough
    case Material::Blend::Masked:
        glDisable(GL_ALPHA_TEST);
        break;
    case Material::Blend::Additive:
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        break;
    default:
        break;
    }
}
