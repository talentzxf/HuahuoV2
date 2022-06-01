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

inline static Vector3f NormalizeRobustInternal(const Vector3f& a, float &l, float &div, float eps)
{
    float a0, a1, a2, aa0, aa1, aa2;
    a0 = a[0];
    a1 = a[1];
    a2 = a[2];

#if FPFIXES
    if (CompareApproximately(a0, 0.0F, eps))
        a0 = aa0 = 0;
    else
#endif
    {
        aa0 = Abs(a0);
    }

#if FPFIXES
    if (CompareApproximately(a1, 0.0F, eps))
        a1 = aa1 = 0;
    else
#endif
    {
        aa1 = Abs(a1);
    }

#if FPFIXES
    if (CompareApproximately(a2, 0.0F, eps))
        a2 = aa2 = 0;
    else
#endif
    {
        aa2 = Abs(a2);
    }

    if (aa1 > aa0)
    {
        if (aa2 > aa1)
        {
            a0 /= aa2;
            a1 /= aa2;
            l = InvSqrt(a0 * a0 + a1 * a1 + 1.0F);
            div = aa2;
            return Vector3f(a0 * l, a1 * l, CopySignf(l, a2));
        }
        else
        {
            // aa1 is largest
            a0 /= aa1;
            a2 /= aa1;
            l = InvSqrt(a0 * a0 + a2 * a2 + 1.0F);
            div = aa1;
            return Vector3f(a0 * l, CopySignf(l, a1), a2 * l);
        }
    }
    else
    {
        if (aa2 > aa0)
        {
            // aa2 is largest
            a0 /= aa2;
            a1 /= aa2;
            l = InvSqrt(a0 * a0 + a1 * a1 + 1.0F);
            div = aa2;
            return Vector3f(a0 * l, a1 * l, CopySignf(l, a2));
        }
        else
        {
            // aa0 is largest
            if (aa0 <= 0)
            {
                l = 0;
                div = 1;
                return Vector3f(0.0F, 1.0F, 0.0F);
            }

            a1 /= aa0;
            a2 /= aa0;
            l = InvSqrt(a1 * a1 + a2 * a2 + 1.0F);
            div = aa0;
            return Vector3f(CopySignf(l, a0), a1 * l, a2 * l);
        }
    }
}

Vector3f NormalizeRobust(const Vector3f& a)
{
    float l, div;
    return NormalizeRobustInternal(a, l, div, Vector3f::epsilon);
}

Vector3f NormalizeRobust(const Vector3f& a, float &invOriginalLength, float eps)
{
    float l, div;
    const Vector3f &n = NormalizeRobustInternal(a, l, div, eps);
    invOriginalLength = l / div;
    // guard for NaNs
    Assert(n == n);
    Assert(invOriginalLength == invOriginalLength);
    Assert(IsNormalized(n));
    return n;
}