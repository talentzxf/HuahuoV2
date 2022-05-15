//
// Created by VincentZhang on 4/23/2022.
//

#include "Vector3f.h"
#include <limits>

const float     Vector3f::epsilon = 0.00001F;
const float     Vector3f::infinity = std::numeric_limits<float>::infinity();
const Vector3f  Vector3f::infinityVec = Vector3f(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity());

const Vector3f  Vector3f::zero  = Vector3f(0, 0, 0);
const Vector3f  Vector3f::one  = Vector3f(1.0F, 1.0F, 1.0F);
const Vector3f  Vector3f::xAxis = Vector3f(1, 0, 0);
const Vector3f  Vector3f::yAxis = Vector3f(0, 1, 0);
const Vector3f  Vector3f::zAxis = Vector3f(0, 0, 1);

void OrthoNormalizeFast(Vector3f* inU, Vector3f* inV, Vector3f* inW)
{
    // compute u0
    *inU = Normalize(*inU);

    // compute u1
    float dot0 = Dot(*inU, *inV);
    *inV -= dot0 * *inU;
    *inV = Normalize(*inV);

    // compute u2
    float dot1 = Dot(*inV, *inW);
    dot0 = Dot(*inU, *inW);
    *inW -= dot0 * *inU + dot1 * *inV;
    *inW = Normalize(*inW);
}

void OrthoNormalize(Vector3f* inU, Vector3f* inV)
{
    // compute u0
    float mag = Magnitude(*inU);
    if (mag > Vector3f::epsilon)
        *inU /= mag;
    else
        *inU = Vector3f(1.0F, 0.0F, 0.0F);

    // compute u1
    float dot0 = Dot(*inU, *inV);
    *inV -= dot0 * *inU;
    mag = Magnitude(*inV);
    if (mag < Vector3f::epsilon)
        *inV = OrthoNormalVectorFast(*inU);
    else
        *inV /= mag;
}

void OrthoNormalize(Vector3f* inU, Vector3f* inV, Vector3f* inW)
{
    // compute u0
    float mag = Magnitude(*inU);
    if (mag > Vector3f::epsilon)
        *inU /= mag;
    else
        *inU = Vector3f(1.0F, 0.0F, 0.0F);

    // compute u1
    float dot0 = Dot(*inU, *inV);
    *inV -= dot0 * *inU;
    mag = Magnitude(*inV);
    if (mag > Vector3f::epsilon)
        *inV /= mag;
    else
        *inV = OrthoNormalVectorFast(*inU);

    // compute u2
    float dot1 = Dot(*inV, *inW);
    dot0 = Dot(*inU, *inW);
    *inW -= dot0 * *inU + dot1 * *inV;
    mag = Magnitude(*inW);
    if (mag > Vector3f::epsilon)
        *inW /= mag;
    else
        *inW = Cross(*inU, *inV);
}

#define k1OverSqrt2 float(0.7071067811865475244008443621048490)

Vector3f OrthoNormalVectorFast(const Vector3f& n)
{
    Vector3f res;
    if (Abs(n.z) > k1OverSqrt2)
    {
        // choose p in y-z plane
        float a = n.y * n.y + n.z * n.z;
        float k = 1.0F / Sqrt(a);
        res.x = 0;
        res.y = -n.z * k;
        res.z = n.y * k;
    }
    else
    {
        // choose p in x-y plane
        float a = n.x * n.x + n.y * n.y;
        float k = 1.0F / Sqrt(a);
        res.x = -n.y * k;
        res.y = n.x * k;
        res.z = 0;
    }
    return res;
}