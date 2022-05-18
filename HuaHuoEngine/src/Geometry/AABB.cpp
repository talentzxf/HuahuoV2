//
// Created by VincentZhang on 5/18/2022.
//

#include "AABB.h"
#include "Math/Simd/vec-affine.h"
#include "Math/Matrix3x3.h"
#include "Math/Quaternionf.h"

const AABB AABB::zero = AABB(Vector3f::zero, Vector3f::zero);
static const math::int4 kXYZMask = math::int4(0xffffffff, 0xffffffff, 0xffffffff, 0x00000000);

void CalculateClosestPoint(const Vector3f& rkPoint, const AABB& rkBox, Vector3f& outPoint, float& outSqrDistance)
{
    ASSERT_VALID_AABB(rkBox);

    // compute coordinates of point in box coordinate system
    Vector3f kClosest = rkPoint - rkBox.m_Center;

    // project test point onto box
    float fSqrDistance = 0.0f;
    float fDelta;

    for (int i = 0; i < 3; i++)
    {
        if (kClosest[i] < -rkBox.m_Extent[i])
        {
            fDelta = kClosest[i] + rkBox.m_Extent[i];
            fSqrDistance += fDelta * fDelta;
            kClosest[i] = -rkBox.m_Extent[i];
        }
        else if (kClosest[i] > rkBox.m_Extent[i])
        {
            fDelta = kClosest[i] - rkBox.m_Extent[i];
            fSqrDistance += fDelta * fDelta;
            kClosest[i] = rkBox.m_Extent[i];
        }
    }

    // Inside
    if (fSqrDistance == 0.0F)
    {
        outPoint = rkPoint;
        outSqrDistance = 0.0F;
    }
        // Outside
    else
    {
        outPoint = kClosest + rkBox.m_Center;
        outSqrDistance = fSqrDistance;
    }
}

// Sphere-AABB distance, Arvo's algorithm
float CalculateSqrDistance(const Vector3f& rkPoint, const AABB& rkBox)
{
    using namespace math;

    ASSERT_VALID_AABB(rkBox);

#if 1
    CompileTimeAssert(offsetof(AABB, m_Center) + sizeof(float4) <= sizeof(AABB), "It is not safe to use vload4f");
    const float4 point = float4(vload3f(rkPoint.GetPtr()), 0.0f);
    const float4 center = vload4f(rkBox.m_Center.GetPtr());
    const float4 extent = float4(vload3f(rkBox.m_Extent.GetPtr()), 0.0f);
    const float4 delta = max(abs(point - center), extent) - extent;
    const float4 delta3 = as_float4(as_int4(delta) & kXYZMask);
    return csum(delta3 * delta3);
#else
    float3 point = vload3f(rkPoint.GetPtr());
    float3 center = vload3f(rkBox.m_Center.GetPtr());
    float3 extent = vload3f(rkBox.m_Extent.GetPtr());

    // old SIMD implementation
    float3 closest = point - center;

    float1 sqrDistance = float1(ZERO);

    int3 testLt = (closest < -extent);
    int3 testGt = (closest > extent);

    float3 delta0 = closest + select(-closest, extent, testLt);
    float3 delta1 = closest - select(closest, extent, testGt);

    sqrDistance += csum(delta0 * delta0);
    sqrDistance += csum(delta1 * delta1);
    return sqrDistance;
#endif

#if 0
    // keeping scalar implementation as reference for SIMD implementation
    for (int i = 0; i < 3; ++i)
    {
        float clos = closest[i];
        float ext = rkBox.m_Extent[i];
        if (clos < -ext)
        {
            float delta = clos + ext;
            sqrDistance += delta * delta;
        }
        else if (clos > ext)
        {
            float delta = clos - ext;
            sqrDistance += delta * delta;
        }
    }
    return sqrDistance;
#endif
}

void AABB::CalculateVertices(Vector3f outVertices[8]) const
{
    ASSERT_VALID_AABB(*this);

    outVertices[0] = m_Center + Vector3f(-m_Extent.x, -m_Extent.y, -m_Extent.z);
    outVertices[1] = m_Center + Vector3f(+m_Extent.x, -m_Extent.y, -m_Extent.z);
    outVertices[2] = m_Center + Vector3f(-m_Extent.x, +m_Extent.y, -m_Extent.z);
    outVertices[3] = m_Center + Vector3f(+m_Extent.x, +m_Extent.y, -m_Extent.z);

    outVertices[4] = m_Center + Vector3f(-m_Extent.x, -m_Extent.y, +m_Extent.z);
    outVertices[5] = m_Center + Vector3f(+m_Extent.x, -m_Extent.y, +m_Extent.z);
    outVertices[6] = m_Center + Vector3f(-m_Extent.x, +m_Extent.y, +m_Extent.z);
    outVertices[7] = m_Center + Vector3f(+m_Extent.x, +m_Extent.y, +m_Extent.z);
}

void MinMaxAABB::CalculateVertices(Vector3f outVertices[8]) const
{
    //    7-----6
    //   /     /|
    //  3-----2 |
    //  | 4   | 5
    //  |     |/
    //  0-----1
    outVertices[0].Set(m_Min.x, m_Min.y, m_Min.z);
    outVertices[1].Set(m_Max.x, m_Min.y, m_Min.z);
    outVertices[2].Set(m_Max.x, m_Max.y, m_Min.z);
    outVertices[3].Set(m_Min.x, m_Max.y, m_Min.z);
    outVertices[4].Set(m_Min.x, m_Min.y, m_Max.z);
    outVertices[5].Set(m_Max.x, m_Min.y, m_Max.z);
    outVertices[6].Set(m_Max.x, m_Max.y, m_Max.z);
    outVertices[7].Set(m_Min.x, m_Max.y, m_Max.z);
}

bool AABB::IsInside(const Vector3f& inPoint) const
{
    ASSERT_VALID_AABB(*this);

    if (inPoint[0] < m_Center[0] - m_Extent[0])
        return false;
    if (inPoint[0] > m_Center[0] + m_Extent[0])
        return false;

    if (inPoint[1] < m_Center[1] - m_Extent[1])
        return false;
    if (inPoint[1] > m_Center[1] + m_Extent[1])
        return false;

    if (inPoint[2] < m_Center[2] - m_Extent[2])
        return false;
    if (inPoint[2] > m_Center[2] + m_Extent[2])
        return false;

    return true;
}

bool MinMaxAABB::IsInside(const Vector3f& inPoint) const
{
    if (inPoint[0] < m_Min[0])
        return false;
    if (inPoint[0] > m_Max[0])
        return false;

    if (inPoint[1] < m_Min[1])
        return false;
    if (inPoint[1] > m_Max[1])
        return false;

    if (inPoint[2] < m_Min[2])
        return false;
    if (inPoint[2] > m_Max[2])
        return false;

    return true;
}

inline Vector3f RotateExtents(const Vector3f& extents, const Matrix3x3f& rotation)
{
    Vector3f newExtents;
    for (int i = 0; i < 3; i++)
        newExtents[i] = Abs(rotation.Get(i, 0) * extents.x) + Abs(rotation.Get(i, 1) * extents.y) + Abs(rotation.Get(i, 2) * extents.z);
    return newExtents;
}

inline Vector3f RotateExtents(const Vector3f& extents, const Matrix4x4f& rotation)
{
    Vector3f newExtents;
    for (int i = 0; i < 3; i++)
        newExtents[i] = Abs(rotation.Get(i, 0) * extents.x) + Abs(rotation.Get(i, 1) * extents.y) + Abs(rotation.Get(i, 2) * extents.z);
    return newExtents;
}

void TransformAABB(const AABB& aabb, const Vector3f& position, const Quaternionf& rotation, AABB& result)
{
    ASSERT_VALID_AABB(aabb);

    Matrix3x3f m;
    QuaternionToMatrix(rotation, m);

    Vector3f extents = RotateExtents(aabb.m_Extent, m);
    Vector3f center = m.MultiplyPoint3(aabb.m_Center);
    center += position;
    result.SetCenterAndExtent(center, extents);

    ASSERT_VALID_AABB(result);
}

void TransformAABB(const AABB& aabb, const Matrix4x4f& transform, AABB& result)
{
    ASSERT_VALID_AABB(aabb);

    Vector3f extents = RotateExtents(aabb.m_Extent, transform);
    Vector3f center = transform.MultiplyPoint3(aabb.m_Center);
    result.SetCenterAndExtent(center, extents);

    ASSERT_VALID_AABB(result);
}

void TransformAABBSlow(const AABB& aabb, const Matrix4x4f& transform, AABB& result)
{
    ASSERT_VALID_AABB(aabb);

    MinMaxAABB transformed;

    Vector3f v[8];
    aabb.CalculateVertices(v);
    for (int i = 0; i < 8; i++)
    {
        Vector3f point = transform.MultiplyPoint3(v[i]);
        transformed.Encapsulate(point);
    }

    result = AABB(transformed);

    ASSERT_VALID_AABB(result);
}

void TransformAABBSlow(const MinMaxAABB& aabb, const Matrix4x4f& transform, MinMaxAABB& result)
{
    Vector3f v[8];
    aabb.CalculateVertices(v);

    result.Init();
    for (int i = 0; i < 8; i++)
    {
        Vector3f point = transform.MultiplyPoint3(v[i]);
        result.Encapsulate(point);
    }
}

void InverseTransformAABB(const AABB& aabb, const Vector3f& position, const Quaternionf& rotation, AABB& result)
{
    ASSERT_VALID_AABB(aabb);

    Matrix3x3f m;
    QuaternionToMatrix(Inverse(rotation), m);

    Vector3f extents = RotateExtents(aabb.m_Extent, m);
    Vector3f center = aabb.m_Center - position;
    center = m.MultiplyPoint3(center);

    result.SetCenterAndExtent(center, extents);

    ASSERT_VALID_AABB(result);
}

bool IsContainedInAABB(const AABB& inside, const AABB& bigBounds)
{
    ASSERT_VALID_AABB(inside);
    ASSERT_VALID_AABB(bigBounds);

    bool outside = false;
    outside |= inside.m_Center[0] - inside.m_Extent[0] < bigBounds.m_Center[0] - bigBounds.m_Extent[0];
    outside |= inside.m_Center[0] + inside.m_Extent[0] > bigBounds.m_Center[0] + bigBounds.m_Extent[0];

    outside |= inside.m_Center[1] - inside.m_Extent[1] < bigBounds.m_Center[1] - bigBounds.m_Extent[1];
    outside |= inside.m_Center[1] + inside.m_Extent[1] > bigBounds.m_Center[1] + bigBounds.m_Extent[1];

    outside |= inside.m_Center[2] - inside.m_Extent[2] < bigBounds.m_Center[2] - bigBounds.m_Extent[2];
    outside |= inside.m_Center[2] + inside.m_Extent[2] > bigBounds.m_Center[2] + bigBounds.m_Extent[2];

    return !outside;
}

#if ENABLE_UNIT_TESTS
UnitTest::MemoryOutStream& operator<<(UnitTest::MemoryOutStream& stream, const AABB& bounds)
{
    stream << "(" << bounds.m_Center.x << ", " << bounds.m_Center.y << ", " << bounds.m_Center.z << " | " << bounds.m_Extent.x << ", " << bounds.m_Extent.y << ", " << bounds.m_Extent.z << ")";
    return stream;
}

#endif