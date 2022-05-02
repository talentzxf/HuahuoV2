//
// Created by VincentZhang on 4/23/2022.
//

#include "Quaternionf.h"
inline float Dot(const Quaternionf& q1, const Quaternionf& q2)
{
    return (q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w);
}