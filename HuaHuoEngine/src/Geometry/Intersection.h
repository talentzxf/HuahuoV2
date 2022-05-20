#pragma once

#include "Math/Simd/vec-types.h"
#include "Math/Matrix4x4.h"

class Ray;
class OptimizedRay;
class Sphere;
class AABB;
class MinMaxAABB;
class Plane;
class Capsule;

class Vector2f;
class Vector3f;
class Vector4f;

// Intersects a Ray with a triangle.
bool IntersectRayTriangle(const Ray& ray, const Vector3f& a, const Vector3f& b, const Vector3f& c);
// t is to be non-Null and returns the first intersection point of the ray (ray.o + t * ray.dir)
bool IntersectRayTriangle(const Ray& ray, const Vector3f& a, const Vector3f& b, const Vector3f& c, float* t);

// Intersects a ray with a volume.
// Returns true if the ray stats inside the volume or in front of the volume
bool IntersectRaySphere(const Ray& ray, const Sphere& inSphere);
bool IntersectRayAABB(const Ray& ray, const AABB& inAABB);

// Intersects a ray with a volume.
// Returns true if the ray stats inside the volume or in front of the volume
// t0 is the first, t1 the second intersection. Both have to be non-NULL.
// (t1 is always positive, t0 is negative if the ray starts inside the volume)
bool IntersectRayAABB(const Ray& ray, const AABB& inAABB, float* t0, float* t1);
bool IntersectRayAABB(const Ray& ray, const AABB& inAABB, float* t0);
bool IntersectRaySphere(const Ray& ray, const Sphere& inSphere, float* t0, float* t1);

// Do these volumes intersect each other?
bool IntersectSphereSphere(const Sphere& s0, const Sphere& s1);
bool IntersectAABBAABB(const AABB& s0, const AABB& s1);
bool IntersectAABBAABB(const MinMaxAABB& a, const MinMaxAABB& b);
bool IntersectAABBSphere(const AABB& aabb, const Sphere& s);
bool IntersectCapsuleCapsule(const Capsule& c0, const Capsule& c1);
bool IntersectCapsuleSphere(const Capsule& capsule, const Sphere& sphere);

// If 'a' and 'b' overlap returns true and the bounds of the intersection in 'outBoxIntersect'
// otherwise returns false
bool IntersectionAABBAABB(const MinMaxAABB& a, const MinMaxAABB& b, MinMaxAABB* outBoxIntersect);

// Do these volumes intersect or touch each other?
bool IntersectSphereSphereInclusive(const Sphere& s0, const Sphere& s1);
bool EXPORT_COREMODULE IntersectAABBAABBInclusive(const AABB& s0, const AABB& s1);

// Does a transformed AABB intersect a sphere?
// Note that for skewed transforms, it will check the sphere's center vs. the box extended by the radius.
// This means that you may get false positives for skewed box/sphere intersections.
// Orthogonal transforms including non-uniform scale always give correct results.
bool IntersectTransformedAABBSphere(const AABB& aabb, const Matrix4x4f& transform, TransformType type, const Sphere& s);

// Tests if the aabb is inside any of the planes enabled by inClipMask
// The bitmask tells which planes have to be tested. (For 6 planes the bitmask is 63)
bool IntersectAABBFrustum(const AABB& a, const Plane* p, UInt32 inClipMask);
bool IntersectAABBFrustumFull(const AABB& a, const Plane p[6]);
bool IntersectAABBPlaneBounds(const AABB& a, const Plane* p, const int planeCount);

/// Fast AABBB plane intersection. Always processes 4 planes at a time using SIMD.
/// Requires an array of math::float4 that is generated via PrepareOptimizedPlanes.
/// The math::float4 array size must be calculated with ALIGN_4.
int PrepareOptimizedPlanes(const Plane* planes, const int planeCount, math::float4* outPlanesSOA);
void PrepareOptimizedPlanes(const Plane* planes, const int planeCount, math::float4* outPlanesSOA, int maxOutputPlaneCount);
bool IntersectAABBPlaneBoundsOptimized(const AABB& a, const math::float4* optimizedPlanes, const int planeCount);

bool IntersectSpherePlaneBoundsOptimized(const math::float4& sphere, const math::float4* optimizedPlanes, const int planeCount);

float RayDistanceToFrustumOriented(const Ray& ray, const Plane* p, const int planeCount, int& planeNumber);
float PointDistanceToFrustum(const Vector4f& point, const Plane* p, const int planeCount);

bool IntersectTriTri(const Vector3f& a0, const Vector3f& b0, const Vector3f& c0,
    const Vector3f& a1, const Vector3f& b1, const Vector3f& c1,
    Vector3f* intersectionLine0, Vector3f* intersectionLine1, bool* coplanar);

// Intersects a ray with a plane (The ray can hit the plane from front and behind)
// On return enter is the rays parameter where the intersection occurred.
bool IntersectRayPlane(const Ray& ray, const Plane& plane, float* enter);

// Intersects a ray with a plane (The ray can hit the plane only from front)
// On return enter is the rays parameter where the intersection occurred.
bool IntersectRayPlaneOriented(const Ray& ray, const Plane& plane, float* enter);

// Intersects a line segment with a plane (can hit the plane from front and behind)
// Fill result point if intersected.
bool IntersectSegmentPlane(const Vector3f& p1, const Vector3f& p2, const Plane& plane, Vector3f* result);

// Intersects a 2D line segment with a Ray2D. p1, p2 are end points on line segment, p3, p4 are points on line
bool IntersectLineSegmentWithLine(const Vector2f& p1, const Vector2f& p2, const Vector2f& p3, const Vector2f& p4, Vector2f& outResult);

// Returns true if the triangle touches or is inside the triangle (a, b, c)
bool IntersectSphereTriangle(const Sphere& s, const Vector3f& a, const Vector3f& b, const Vector3f& c);

/// Returns true if the bounding box is inside the planes or intersects any of the planes.
bool TestPlanesAABB(const Plane* planes, const int planeCount, const AABB& bounds);

/// Projects point on a line.
template<typename T>
T ProjectPointLine(const T& point, const T& lineStart, const T& lineEnd)
{
    T relativePoint = point - lineStart;
    T lineDirection = lineEnd - lineStart;
    float length = Magnitude(lineDirection);
    T normalizedLineDirection = lineDirection;
    if (length > T::epsilon)
        normalizedLineDirection /= length;

    float dot = Dot(normalizedLineDirection, relativePoint);
    dot = std::clamp(dot, 0.0F, length);

    return lineStart + normalizedLineDirection * dot;
}

/// Returns the distance to a line from a point.
template<typename T>
float DistancePointLine(const T& point, const T& lineStart, const T& lineEnd)
{
    return Magnitude(ProjectPointLine<T>(point, lineStart, lineEnd) - point);
}

static inline bool IsSkewedBasis(const Vector3f basisVectors[3], float epsilon)
{
    float maxDot = 0.0f;
    for (int axis = 0; axis < 3; axis++)
    {
        float dot = Dot(basisVectors[axis], basisVectors[axis < 2 ? axis + 1 : 0]);
        maxDot = FloatMax(Abs(dot), maxDot);
    }
    return maxDot > epsilon;
}
