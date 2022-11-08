//
// Created by VincentZhang on 4/23/2022.
//

#ifndef HUAHUOENGINE_QUATERNIONF_H
#define HUAHUOENGINE_QUATERNIONF_H

#include <cmath>
#include "Serialize/SerializationMetaFlags.h"
#include "Serialize/SerializeUtility.h"
#include "FloatConversion.h"
#include "Matrix3x3.h"
#include "Math/Simd/RotationOrder.h"

class Matrix4x4f;
class Quaternionf;
float Dot(const Quaternionf& q1, const Quaternionf& q2);

inline float SqrMagnitude(const Quaternionf& q)
{
    return Dot(q, q);
}

inline float Magnitude(const Quaternionf& q)
{
    return SqrtImpl(SqrMagnitude(q));
}

class Quaternionf {
public:
    float x, y, z, w;

    DEFINE_GET_TYPESTRING_IS_ANIMATION_CHANNEL(Quaternionf)
    template<class TransferFunction> void Transfer(TransferFunction& transfer);

    Quaternionf() {}  // Default ctor is intentionally empty for performance reasons
    Quaternionf(float inX, float inY, float inZ, float inW);
    explicit Quaternionf(const float* array)   { x = array[0]; y = array[1]; z = array[2]; w = array[3]; }

    // methods

    const float* GetPtr() const             { return &x; }
    float* GetPtr()                                { return &x; }

    const float& operator[](int i) const   { return GetPtr()[i]; }
    float& operator[](int i)                  { return GetPtr()[i]; }

    void Set(float inX, float inY, float inZ, float inW);
    void Set(const Quaternionf& aQuat);
    void Set(const float* array)   { x = array[0]; y = array[1]; z = array[2]; w = array[3]; }

//    friend Quaternionf Normalize(const Quaternionf& q) {    return q / Magnitude(q); }
    friend Quaternionf NormalizeSafe(const Quaternionf& q);

    static Quaternionf identity() { return Quaternionf(0.0F, 0.0F, 0.0F, 1.0F); }

    friend float SqrMagnitude(const Quaternionf& q);
    friend float Magnitude(const Quaternionf& q);

    bool operator==(const Quaternionf& q) const        { return x == q.x && y == q.y && z == q.z && w == q.w; }
    bool operator!=(const Quaternionf& q) const        { return x != q.x || y != q.y || z != q.z || w != q.w; }

    Quaternionf&    operator+=(const Quaternionf&  aQuat);
    Quaternionf&    operator-=(const Quaternionf&  aQuat);
    Quaternionf&    operator*=(const float        aScalar);
    Quaternionf&    operator*=(const Quaternionf&     aQuat);
    Quaternionf&    operator/=(const float        aScalar);


    friend Quaternionf operator+(const Quaternionf& lhs, const Quaternionf& rhs)
    {
        Quaternionf q(lhs);
        return q += rhs;
    }

    friend Quaternionf  operator-(const Quaternionf& lhs, const Quaternionf& rhs)
    {
        Quaternionf t(lhs);
        return t -= rhs;
    }

    Quaternionf operator-() const
    {
        return Quaternionf(-x, -y, -z, -w);
    }

    Quaternionf operator*(const float s) const
    {
        return Quaternionf(x * s, y * s, z * s, w * s);
    }

    friend Quaternionf  operator*(const float s, const Quaternionf& q)
    {
        Quaternionf t(q);
        return t *= s;
    }

    friend Quaternionf  operator/(const Quaternionf& q, const float s)
    {
        Quaternionf t(q);
        return t /= s;
    }

    inline friend Quaternionf operator*(const Quaternionf& lhs, const Quaternionf& rhs)
    {
        return Quaternionf(
                lhs.w * rhs.x + lhs.x * rhs.w + lhs.y * rhs.z - lhs.z * rhs.y,
                lhs.w * rhs.y + lhs.y * rhs.w + lhs.z * rhs.x - lhs.x * rhs.z,
                lhs.w * rhs.z + lhs.z * rhs.w + lhs.x * rhs.y - lhs.y * rhs.x,
                lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z);
    }

    friend Quaternionf Normalize(const Quaternionf& q) {    return q / Magnitude(q); }
};

// operator overloads
//  inlines

inline Quaternionf::Quaternionf(float inX, float inY, float inZ, float inW)
{
    x = inX;
    y = inY;
    z = inZ;
    w = inW;
}

template<class TransferFunction> inline
void Quaternionf::Transfer(TransferFunction& transfer)
{
    transfer.AddMetaFlag(kTransferUsingFlowMappingStyle);
    TRANSFER(x);
    TRANSFER(y);
    TRANSFER(z);
    TRANSFER(w);
}

void EXPORT_COREMODULE QuaternionToMatrix(const Quaternionf& q, Matrix4x4f& m);
void EXPORT_COREMODULE QuaternionToMatrix(const Quaternionf& q, Matrix3x3f& m);

void EXPORT_COREMODULE MatrixToQuaternion(const Matrix3x3f& m, Quaternionf& q);
void EXPORT_COREMODULE MatrixToQuaternion(const Matrix4x4f& m, Quaternionf& q);

Vector3f EXPORT_COREMODULE QuaternionToEuler(const Quaternionf& quat, math::RotationOrder order = math::kOrderUnityDefault);

inline bool IsFinite(const Quaternionf& f)
{
    return IsFinite(f.x) & IsFinite(f.y) & IsFinite(f.z) & IsFinite(f.w);
}

inline Quaternionf Conjugate(const Quaternionf& q)
{
    return Quaternionf(-q.x, -q.y, -q.z, q.w);
}

inline Quaternionf Inverse(const Quaternionf& q)
{
    // Is it necessary to divide by SqrMagnitude???
    Quaternionf res = Conjugate(q);
    return res;
}

inline Quaternionf& Quaternionf::operator/=(const float       aScalar)
{
    Assert(!CompareApproximately(aScalar, 0.0F));
    x /= aScalar;
    y /= aScalar;
    z /= aScalar;
    w /= aScalar;
    return *this;
}

#endif //HUAHUOENGINE_QUATERNIONF_H
