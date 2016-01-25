
#ifndef _ZEQ_ANIMATED_MODEL_HPP_
#define _ZEQ_ANIMATED_MODEL_HPP_

#include "define.hpp"
#include "model_prototype.hpp"
#include "vertex_buffer.hpp"
#include "axis_aligned_bounding_box.hpp"

class AnimatedModelPrototype : public ModelPrototype
{
private:
    int     m_race;
    uint8_t m_gender;

public:
    AnimatedModelPrototype(int race, uint8_t gender);
    virtual ~AnimatedModelPrototype();
};

#endif//_ZEQ_ANIMATED_MODEL_HPP_
