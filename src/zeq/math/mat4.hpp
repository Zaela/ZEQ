
#ifndef _ZEQ_MAT4_HPP_
#define _ZEQ_MAT4_HPP_

#include "define.hpp"
#include "math.hpp"

class Mat4
{
private:
    float m[16];

private:
    Mat4(int i)
    {
        (void)i;
    }

public:
    Mat4()
    {
        memset(m, 0, sizeof(float) * 16);
    }
    
    void identity()
    {
        m[ 0] = 1;
        m[ 5] = 1;
        m[10] = 1;
        m[15] = 1;
    }
    
    float* ptr()
    {
        return m;
    }
    
    const float* ptr() const
    {
        return m;
    }
    
    Mat4 operator*(const Mat4& o)
    {
        Mat4 ret(1);
        
        ret.m[ 0] = m[ 0] * o.m[ 0] + m[ 4] * o.m[ 1] + m[ 8] * o.m[ 2] + m[12] * o.m[ 3];
        ret.m[ 1] = m[ 1] * o.m[ 0] + m[ 5] * o.m[ 1] + m[ 9] * o.m[ 2] + m[13] * o.m[ 3];
        ret.m[ 2] = m[ 2] * o.m[ 0] + m[ 6] * o.m[ 1] + m[10] * o.m[ 2] + m[14] * o.m[ 3];
        ret.m[ 3] = m[ 3] * o.m[ 0] + m[ 7] * o.m[ 1] + m[11] * o.m[ 2] + m[15] * o.m[ 3];
        
        ret.m[ 4] = m[ 0] * o.m[ 4] + m[ 4] * o.m[ 5] + m[ 8] * o.m[ 6] + m[12] * o.m[ 7];
        ret.m[ 5] = m[ 1] * o.m[ 4] + m[ 5] * o.m[ 5] + m[ 9] * o.m[ 6] + m[13] * o.m[ 7];
        ret.m[ 6] = m[ 2] * o.m[ 4] + m[ 6] * o.m[ 5] + m[10] * o.m[ 6] + m[14] * o.m[ 7];
        ret.m[ 7] = m[ 3] * o.m[ 4] + m[ 7] * o.m[ 5] + m[11] * o.m[ 6] + m[15] * o.m[ 7];
        
        ret.m[ 8] = m[ 0] * o.m[ 8] + m[ 4] * o.m[ 9] + m[ 8] * o.m[10] + m[12] * o.m[11];
        ret.m[ 9] = m[ 1] * o.m[ 8] + m[ 5] * o.m[ 9] + m[ 9] * o.m[10] + m[13] * o.m[11];
        ret.m[10] = m[ 2] * o.m[ 8] + m[ 6] * o.m[ 9] + m[10] * o.m[10] + m[14] * o.m[11];
        ret.m[11] = m[ 3] * o.m[ 8] + m[ 7] * o.m[ 9] + m[11] * o.m[10] + m[15] * o.m[11];
        
        ret.m[12] = m[ 0] * o.m[12] + m[ 4] * o.m[13] + m[ 8] * o.m[14] + m[12] * o.m[15];
        ret.m[13] = m[ 1] * o.m[12] + m[ 5] * o.m[13] + m[ 9] * o.m[14] + m[13] * o.m[15];
        ret.m[14] = m[ 2] * o.m[12] + m[ 6] * o.m[13] + m[10] * o.m[14] + m[14] * o.m[15];
        ret.m[15] = m[ 3] * o.m[12] + m[ 7] * o.m[13] + m[11] * o.m[14] + m[15] * o.m[15];
        
        return ret;
    }
    
    static Mat4 perspective(float fovRadians, float aspectRatio, float nearZ, float farZ)
    {
        Mat4 ret;
        
        float f     = 1.0f / tan(fovRadians * 0.5f);
        float diffZ = nearZ - farZ;
        
        ret.m[ 0] = f / aspectRatio;
        ret.m[ 5] = f;
        ret.m[10] = (farZ + nearZ) / diffZ;
        ret.m[11] = -1.0f;
        ret.m[14] = (2.0f * farZ * nearZ) / diffZ;
        
        return ret;
    }
    
    static Mat4 lookAt(Vec3& pos, Vec3& targ, Vec3& up)
    {
        Vec3 zaxis = targ - pos;
        zaxis.normalize();
        
        Vec3 xaxis = zaxis.crossProduct(up);
        xaxis.normalize();
        
        Vec3 yaxis = xaxis.crossProduct(zaxis);
        
        Mat4 ret(1);
        
        ret.m[ 0] = xaxis.x;
        ret.m[ 1] = yaxis.x;
        ret.m[ 2] = zaxis.x;
        ret.m[ 3] = 0.0f;
        
        ret.m[ 4] = xaxis.y;
        ret.m[ 5] = yaxis.y;
        ret.m[ 6] = zaxis.y;
        ret.m[ 7] = 0.0f;
        
        ret.m[ 8] = xaxis.z;
        ret.m[ 9] = yaxis.z;
        ret.m[10] = zaxis.z;
        ret.m[11] = 0.0f;
        
        ret.m[12] = -xaxis.dotProduct(pos);
        ret.m[13] = -yaxis.dotProduct(pos);
        ret.m[14] = -zaxis.dotProduct(pos);
        ret.m[15] = 1.0f;
        
        return ret;
    }
    
    void setRotation(float x, float y, float z)
    {
        setRotationRadians(Math::toRadians(x), Math::toRadians(y), Math::toRadians(z));
    }
    
    void setRotationRadians(float x, float y, float z)
    {
        float cr = cos(x);
        float sr = sin(x);
        float cp = cos(y);
        float sp = sin(y);
        float cy = cos(z);
        float sy = sin(z);

        m[ 0] = cp * cy;
        m[ 1] = cp * sy;
        m[ 2] = -sp;

        float srsp = sr * sp;
        float crsp = cr * sp;

        m[ 4] = srsp * cy - cr * sy;
        m[ 5] = srsp * sy + cr * cy;
        m[ 6] = sr * cp;

        m[ 8] = crsp * cy + sr * sy;
        m[ 9] = crsp * sy - sr * cy;
        m[10] = cr * cp;
    }
    
    void transformVector(Vec3& vec)
    {
        float x = vec.x * m[ 0] + vec.y * m[ 4] + vec.z * m[ 8] + m[12];
        float y = vec.x * m[ 1] + vec.y * m[ 5] + vec.z * m[ 9] + m[13];
        float z = vec.x * m[ 2] + vec.y * m[ 6] + vec.z * m[10] + m[14];
        
        vec.set(x, y, z);
    }
};

#endif//_ZEQ_MAT4_HPP_
