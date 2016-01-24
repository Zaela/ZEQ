
#ifndef _ZEQ_ANIMATED_MODEL_HPP_
#define _ZEQ_ANIMATED_MODEL_HPP_

#include "define.hpp"
#include "model_prototype.hpp"
#include "vertex_buffer.hpp"
#include "axis_aligned_bounding_box.hpp"

class AnimatedModel : public ModelPrototype
{
private:
    int64_t m_id;

public:
    AnimatedModel(int64_t id);
    virtual ~AnimatedModel();
};

#endif//_ZEQ_ANIMATED_MODEL_HPP_
