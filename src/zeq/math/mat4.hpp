
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
        identity();
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
    
    float& operator[](int i)
    {
        return m[i];
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
    
    void setTranslation(Vec3& vec)
    {
        m[12] = vec.x;
        m[13] = vec.y;
        m[14] = vec.z;
    }
    
    void setScale(Vec3& vec)
    {
        m[ 0] = vec.x;
        m[ 5] = vec.y;
        m[10] = vec.z;
        m[15] = 1.0f;
    }
    
    void transformVector(Vec3& vec)
    {
        float x = vec.x * m[ 0] + vec.y * m[ 4] + vec.z * m[ 8] + m[12];
        float y = vec.x * m[ 1] + vec.y * m[ 5] + vec.z * m[ 9] + m[13];
        float z = vec.x * m[ 2] + vec.y * m[ 6] + vec.z * m[10] + m[14];
        
        vec.set(x, y, z);
    }
    
    void transformVector(Vec3& dst, const Vec3& src)
    {
        dst.x = src.x * m[ 0] + src.y * m[ 4] + src.z * m[ 8] + m[12];
        dst.y = src.x * m[ 1] + src.y * m[ 5] + src.z * m[ 9] + m[13];
        dst.z = src.x * m[ 2] + src.y * m[ 6] + src.z * m[10] + m[14];
    }
    
    void rotateVector(Vec3& dst, const Vec3& src)
    {
        dst.x = src.x * m[ 0] + src.y * m[ 4] + src.z * m[ 8];
        dst.y = src.x * m[ 1] + src.y * m[ 5] + src.z * m[ 9];
        dst.z = src.x * m[ 2] + src.y * m[ 6] + src.z * m[10];
    }
    
    bool invert()
    {
        Mat4 temp(1);
        
        float d = (m[ 0] * m[ 5] - m[ 1] * m[ 4]) * (m[10] * m[15] - m[11] * m[14]) -
                  (m[ 0] * m[ 6] - m[ 2] * m[ 4]) * (m[ 9] * m[15] - m[11] * m[13]) +
                  (m[ 0] * m[ 7] - m[ 3] * m[ 4]) * (m[ 9] * m[14] - m[10] * m[13]) +
                  (m[ 1] * m[ 6] - m[ 2] * m[ 5]) * (m[ 8] * m[15] - m[11] * m[12]) -
                  (m[ 1] * m[ 7] - m[ 3] * m[ 5]) * (m[ 8] * m[14] - m[10] * m[12]) +
                  (m[ 2] * m[ 7] - m[ 3] * m[ 6]) * (m[ 8] * m[13] - m[ 9] * m[12]);
        
        if (fabsf(d) < 0.000001f)
            return false;
        
        d = Math::reciprocal(d);
        
        temp[ 0] = d * (m[ 5] * (m[10] * m[15] - m[11] * m[14]) + m[ 6] * (m[11] * m[13] - m[ 9] * m[15]) + m[ 7] * (m[ 9] * m[14] - m[10] * m[13]));
        temp[ 1] = d * (m[ 9] * (m[ 2] * m[15] - m[ 3] * m[14]) + m[10] * (m[ 3] * m[13] - m[ 1] * m[15]) + m[11] * (m[ 1] * m[14] - m[ 2] * m[13]));
        temp[ 2] = d * (m[13] * (m[ 2] * m[ 7] - m[ 3] * m[ 6]) + m[14] * (m[ 3] * m[ 5] - m[ 1] * m[ 7]) + m[15] * (m[ 1] * m[ 6] - m[ 2] * m[ 5]));
        temp[ 3] = d * (m[ 1] * (m[ 7] * m[10] - m[ 6] * m[11]) + m[ 2] * (m[ 5] * m[11] - m[ 7] * m[ 9]) + m[ 3] * (m[ 6] * m[ 9] - m[ 5] * m[10]));
        temp[ 4] = d * (m[ 6] * (m[ 8] * m[15] - m[11] * m[12]) + m[ 7] * (m[10] * m[12] - m[ 8] * m[14]) + m[ 4] * (m[11] * m[14] - m[10] * m[15]));
        temp[ 5] = d * (m[10] * (m[ 0] * m[15] - m[ 3] * m[12]) + m[11] * (m[ 2] * m[12] - m[ 0] * m[14]) + m[ 8] * (m[ 3] * m[14] - m[ 2] * m[15]));
        temp[ 6] = d * (m[14] * (m[ 0] * m[ 7] - m[ 3] * m[ 4]) + m[15] * (m[ 2] * m[ 4] - m[ 0] * m[ 6]) + m[12] * (m[ 3] * m[ 6] - m[ 2] * m[ 7]));
        temp[ 7] = d * (m[ 2] * (m[ 7] * m[ 8] - m[ 4] * m[11]) + m[ 3] * (m[ 4] * m[10] - m[ 6] * m[ 8]) + m[ 0] * (m[ 6] * m[11] - m[ 7] * m[10]));
        temp[ 8] = d * (m[ 7] * (m[ 8] * m[13] - m[ 9] * m[12]) + m[ 4] * (m[ 9] * m[15] - m[11] * m[13]) + m[ 5] * (m[11] * m[12] - m[ 8] * m[15]));
        temp[ 9] = d * (m[11] * (m[ 0] * m[13] - m[ 1] * m[12]) + m[ 8] * (m[ 1] * m[15] - m[ 3] * m[13]) + m[ 9] * (m[ 3] * m[12] - m[ 0] * m[15]));
        temp[10] = d * (m[15] * (m[ 0] * m[ 5] - m[ 1] * m[ 4]) + m[12] * (m[ 1] * m[ 7] - m[ 3] * m[ 5]) + m[13] * (m[ 3] * m[ 4] - m[ 0] * m[ 7]));
        temp[11] = d * (m[ 3] * (m[ 5] * m[ 8] - m[ 4] * m[ 9]) + m[ 0] * (m[ 7] * m[ 9] - m[ 5] * m[11]) + m[ 1] * (m[ 4] * m[11] - m[ 7] * m[ 8]));
        temp[12] = d * (m[ 4] * (m[10] * m[13] - m[ 9] * m[14]) + m[ 5] * (m[ 8] * m[14] - m[10] * m[12]) + m[ 6] * (m[ 9] * m[12] - m[ 8] * m[13]));
        temp[13] = d * (m[ 8] * (m[ 2] * m[13] - m[ 1] * m[14]) + m[ 9] * (m[ 0] * m[14] - m[ 2] * m[12]) + m[10] * (m[ 1] * m[12] - m[ 0] * m[13]));
        temp[14] = d * (m[12] * (m[ 2] * m[ 5] - m[ 1] * m[ 6]) + m[13] * (m[ 0] * m[ 6] - m[ 2] * m[ 4]) + m[14] * (m[ 1] * m[ 4] - m[ 0] * m[ 5]));
        temp[15] = d * (m[ 0] * (m[ 5] * m[10] - m[ 6] * m[ 9]) + m[ 1] * (m[ 6] * m[ 8] - m[ 4] * m[10]) + m[ 2] * (m[ 4] * m[ 9] - m[ 5] * m[ 8]));
        
        *this = temp;
        
        return true;
    }
};

#endif//_ZEQ_MAT4_HPP_
