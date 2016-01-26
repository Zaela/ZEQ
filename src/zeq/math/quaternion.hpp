
#ifndef _ZEQ_QUATERNION_HPP_
#define _ZEQ_QUATERNION_HPP_

#include "define.hpp"
#include "mat4.hpp"

struct Quaternion
{
    float x, y, z, w;
    
    Quaternion() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) { }
    
    void getMatrixTransposed(Mat4& m) const
    {
        m[ 0] = 1.0f - 2.0f*y*y - 2.0f*z*z;
        m[ 1] = 2.0f*x*y - 2.0f*z*w;
        m[ 2] = 2.0f*x*z + 2.0f*y*w;
        m[ 3] = 0.0f;
        
        m[ 4] = 2.0f*x*y + 2.0f*z*w;
        m[ 5] = 1.0f - 2.0f*x*x - 2.0f*z*z;
        m[ 6] = 2.0f*z*y - 2.0f*x*w;
        m[ 7] = 0.0f;
        
        m[ 8] = 2.0f*x*z - 2.0f*y*w;
        m[ 9] = 2.0f*z*y + 2.0f*x*w;
        m[10] = 1.0f - 2.0f*x*x - 2.0f*y*y;
        m[11] = 0.0f;
        
        m[12] = 0.0f;
        m[13] = 0.0f;
        m[14] = 0.0f;
        m[15] = 1.0f;
    }
};

#endif//_ZEQ_QUATERNION_HPP_
