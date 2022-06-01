//
// Created by VincentZhang on 5/18/2022.
//

#ifndef HUAHUOENGINE_AABB_H
#define HUAHUOENGINE_AABB_H

#include "Math/Vector3f.h"
#include "Math/Matrix4x4.h"

class MinMaxAABB;
class Quaternionf;
class Matrix3x3f;

#if DEBUGMODE
#   define ASSERT_VALID_AABB(bnd) AssertMsg((bnd).m_Center == (bnd).m_Center && (bnd).m_Extent == (bnd).m_Extent, "Invalid AABB " #bnd)
#else
#   define ASSERT_VALID_AABB(bnd) do{}while(0)
#endif


class AABB
{
public:

    Vector3f    m_Center;
    Vector3f    m_Extent;

    AABB()
    // TODO: we can get rid of this when Vector3f is initialized to NaN in DEBUGMODE
#if DEBUGMODE
    : m_Center(std::numeric_limits<float>::signaling_NaN()
                   , std::numeric_limits<float>::signaling_NaN()
                   , std::numeric_limits<float>::signaling_NaN())
        , m_Extent(std::numeric_limits<float>::signaling_NaN()
                   , std::numeric_limits<float>::signaling_NaN()
                   , std::numeric_limits<float>::signaling_NaN())
#endif
    {}

    AABB(const Vector3f& center, const Vector3f& extent) { m_Center = center; m_Extent = extent; }
    DECLARE_SERIALIZE_OPTIMIZE_TRANSFER(AABB)

    explicit AABB(const MinMaxAABB& aabb) { FromMinMaxAABB(aabb); }

    bool operator==(const AABB& b) const { return m_Center == b.m_Center && m_Extent == b.m_Extent; }

    void SetCenterAndExtent(const Vector3f& center, const Vector3f& extent) { m_Center = center; m_Extent = extent; }

    Vector3f CalculateMin() const { return m_Center - m_Extent; }
    Vector3f CalculateMax() const { return m_Center + m_Extent; }

    void Expand(float inValue);
    void Scale(const Vector3f& scale);
    void Offset(const Vector3f& offset);

    // Encapsulate using AABB is bad for performance and precision
    // Use MinMaxAABB for growing volumes by e.g. encapsulating a set of points
    void SlowLossyEncapsulate(const Vector3f& inPoint);

    bool IsInside(const Vector3f& inPoint) const;
    void CalculateVertices(Vector3f outVertices[8]) const;
    bool IsFinite() const { return ::IsFinite(m_Center) && ::IsFinite(m_Extent); }

    // TODO: Get rid of these getters - members are public
    Vector3f&       GetCenter()        { return m_Center; }
    Vector3f&       GetExtent()        { return m_Extent; }
    const Vector3f& GetCenter() const  { return m_Center; }
    const Vector3f& GetExtent() const  { return m_Extent; }

    // TODO: Get rid of 'zero' - it's ambiguous at best.
    // Zero bounds are used to specify 'invalid' bounds in a few places.
    // But in fact it's valid under many operations - and will likely be contained by e.g. another AABB
    static const AABB zero;

private:

    void FromMinMaxAABB(const MinMaxAABB& aabb);
};

//BIND_MANAGED_TYPE_NAME(AABB, UnityEngine_Bounds);

bool IsContainedInAABB(const AABB& inside, const AABB& bigBounds);

// Transforms AABB.
// Can be thought of as Converting OBB to an AABB:
// rotate the center and extents of the OBB And find the smallest enclosing AABB around it.
void TransformAABB(const AABB& aabb, const Vector3f& position, const Quaternionf& rotation, AABB& result);

/// This is not mathematically correct for non-uniform scaled objects. But it seems to work well enough.
/// If you use it with non-uniform scale make sure to verify it extensively.
EXPORT_COREMODULE void TransformAABB(const AABB& aabb, const Matrix4x4f& transform, AABB& result);

/// This version is much slower but works correctly with non-uniform scale
void TransformAABBSlow(const AABB& aabb, const Matrix4x4f& transform, AABB& result);

/// This version is much slower but works correctly with non-uniform scale
void TransformAABBSlow(const MinMaxAABB& aabb, const Matrix4x4f& transform, MinMaxAABB& result);

void InverseTransformAABB(const AABB& aabb, const Vector3f& position, const Quaternionf& rotation, AABB& result);


/// The closest distance to the surface or inside the aabb.
float CalculateSqrDistance(const Vector3f& rkPoint, const AABB& rkBox);


/// Returns the sqr distance and the closest point inside or on the surface of the aabb.
/// If inside the aabb, distance will be zero and rkPoint will be returned.
EXPORT_COREMODULE void CalculateClosestPoint(const Vector3f& rkPoint, const AABB& rkBox, Vector3f& outPoint, float& outSqrDistance);

#if ENABLE_UNIT_TESTS
UnitTest::MemoryOutStream& operator<<(UnitTest::MemoryOutStream& stream, const AABB& bounds);
#endif

class MinMaxAABB
{
public:

    Vector3f    m_Min;
    Vector3f    m_Max;

    MinMaxAABB()                                       { Init(); }
    MinMaxAABB(const Vector3f& min, const Vector3f& max) : m_Min(min), m_Max(max) {}
    explicit MinMaxAABB(const AABB& aabb)              { FromAABB(aabb); }
    DECLARE_SERIALIZE_OPTIMIZE_TRANSFER(MinMaxAABB)

    void Init();

    Vector3f CalculateCenter() const   { return 0.5F * (m_Max + m_Min); }
    Vector3f CalculateExtent() const   { return 0.5F * (m_Max - m_Min); }
    Vector3f CalculateSize() const     { return (m_Max - m_Min); }

    void Expand(float inValue);
    void Expand(const Vector3f& inOffset);

    void Encapsulate(const Vector3f& inPoint);
    void Encapsulate(const AABB& aabb);
    void Encapsulate(const MinMaxAABB& other);

    bool IsInside(const Vector3f& inPoint) const;
    void CalculateVertices(Vector3f outVertices[8]) const;
    bool IsValid() const;

    // TODO: Get rid of these getters - members are public
    const Vector3f& GetMin() const     { return m_Min; }
    const Vector3f& GetMax() const     { return m_Max; }

private:

    void FromAABB(const AABB& inAABB);
};

inline void AABB::Expand(float inValue)
{
    ASSERT_VALID_AABB(*this);
    m_Extent += Vector3f(inValue, inValue, inValue);
}

inline void AABB::Scale(const Vector3f& scale)
{
    ASSERT_VALID_AABB(*this);
    m_Center.Scale(scale);
    m_Extent.Scale(scale);
}

inline void AABB::Offset(const Vector3f& offset)
{
    ASSERT_VALID_AABB(*this);
    m_Center += offset;
}

inline void AABB::SlowLossyEncapsulate(const Vector3f& inPoint)
{
    ASSERT_VALID_AABB(*this);

    Vector3f bmin = min(CalculateMin(), inPoint);
    Vector3f bmax = max(CalculateMax(), inPoint);
    m_Center = 0.5f * (bmin + bmax);
    m_Extent = 0.5f * (bmax - bmin);
}

inline void AABB::FromMinMaxAABB(const MinMaxAABB& inAABB)
{
    AssertMsg(inAABB.IsValid(), "Converting invalid MinMaxAABB");

    m_Center = (inAABB.m_Min + inAABB.m_Max) * 0.5F;
    m_Extent = (inAABB.m_Max - inAABB.m_Min) * 0.5F;
}

inline void MinMaxAABB::Encapsulate(const Vector3f& inPoint)
{
    m_Min = min(m_Min, inPoint);
    m_Max = max(m_Max, inPoint);
}

inline void MinMaxAABB::Encapsulate(const AABB& aabb)
{
    ASSERT_VALID_AABB(aabb);

    m_Min = min(m_Min, aabb.CalculateMin());
    m_Max = max(m_Max, aabb.CalculateMax());
}

inline void MinMaxAABB::Encapsulate(const MinMaxAABB& other)
{
    m_Min = min(m_Min, other.m_Min);
    m_Max = max(m_Max, other.m_Max);
}

inline void MinMaxAABB::Expand(float inValue)
{
    AssertMsg(IsValid(), "Expanding invalid MinMaxAABB");

    Vector3f offset = Vector3f(inValue, inValue, inValue);
    m_Min -= offset;
    m_Max += offset;
}

inline void MinMaxAABB::Expand(const Vector3f& inOffset)
{
    AssertMsg(IsValid(), "Expanding invalid MinMaxAABB");

    m_Min -= inOffset;
    m_Max += inOffset;
}

inline bool MinMaxAABB::IsValid() const
{
    // __FAKEABLE_METHOD__(MinMaxAABB, IsValid, ());
    return !(m_Min == Vector3f::infinityVec || m_Max == -Vector3f::infinityVec);
}

inline void MinMaxAABB::Init()
{
    m_Min = Vector3f::infinityVec;
    m_Max = -Vector3f::infinityVec;
}

inline void MinMaxAABB::FromAABB(const AABB& inAABB)
{
    ASSERT_VALID_AABB(inAABB);

    m_Min = inAABB.m_Center  - inAABB.m_Extent;
    m_Max = inAABB.m_Center  + inAABB.m_Extent;
}

template<class TransferFunction> inline
void AABB::Transfer(TransferFunction& transfer)
{
    TRANSFER(m_Center);
    TRANSFER(m_Extent);
}

template<class TransferFunction> inline
void MinMaxAABB::Transfer(TransferFunction& transfer)
{
    TRANSFER(m_Min);
    TRANSFER(m_Max);
}

//BIND_MANAGED_TYPE_NAME(AABB, UnityEngine_Bounds)


#endif //HUAHUOENGINE_AABB_H
