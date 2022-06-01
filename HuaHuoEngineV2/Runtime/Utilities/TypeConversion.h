//
// Created by VincentZhang on 4/23/2022.
//

#ifndef HUAHUOENGINE_TYPECONVERSION_H
#define HUAHUOENGINE_TYPECONVERSION_H

#include "Math/Quaternionf.h"
#include "Math/Simd/vec-types.h"
#include "Math/Vector3f.h"
#include "Math/Matrix3x3.h"
#include "Math/Matrix4x4.h"

inline math::float4 QuaternionfTofloat4(const Quaternionf& q)
{
    //  todo: this won't compile. It throws an ICE tizen
    //  return math::vload4f(q.GetPtr());
    return math::float4(q.x, q.y, q.z, q.w);
}

inline math::float3 Vector3fTofloat3(const Vector3f& v)
{
    return math::vload3f(v.GetPtr());
}

inline math::float3x3 Matrix4x4fTofloat3x3(const Matrix4x4f& m)
{
    math::float3x3 r;
    r.m0 = math::vload3f(&m.m_Data[0]);
    r.m1 = math::vload3f(&m.m_Data[4]);
    r.m2 = math::vload3f(&m.m_Data[8]);
    return r;
}

inline Matrix3x3f float3x3ToMatrix3x3f(const math::float3x3& m)
{
    Matrix3x3f r;
    math::vstore3f(&r.m_Data[0], m.m0);
    math::vstore3f(&r.m_Data[3], m.m1);
    math::vstore3f(&r.m_Data[6], m.m2);
    return r;
}
#endif //HUAHUOENGINE_TYPECONVERSION_H
