//
// Created by VincentZhang on 4/23/2022.
//

#ifndef PERSISTENTMANAGER_TYPECONVERSION_H
#define PERSISTENTMANAGER_TYPECONVERSION_H

#include "Math/Quaternionf.h"
#include "Math/Simd/vec-types.h"

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

#endif //PERSISTENTMANAGER_TYPECONVERSION_H
