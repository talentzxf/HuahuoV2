#include "Intersection.h"
#include "Ray.h"
#include "Plane.h"
#include "Sphere.h"
#include "AABB.h"
#include "Capsule.h"
#include "Logging/LogAssert.h"
#include "TriTriIntersect.h"
#include "Camera/CullingParameters.h"
#include "Math/Simd/vec-math.h"
#include "Math/Vector2f.h"
#include "Utilities/Utility.h"

static const math::int4 kXYZMask_ = math::int4(0xffffffff, 0xffffffff, 0xffffffff, 0x00000000);

bool IntersectRayTriangle(const Ray& ray, const Vector3f& a, const Vector3f& b, const Vector3f& c)
{
    float t;
    return IntersectRayTriangle(ray, a, b, c, &t);
}

bool IntersectRayTriangle(const Ray& ray, const Vector3f& a, const Vector3f& b, const Vector3f& c, float* outT)
{
    const float kMinDet = 1e-6f;

    float  t, u, v;
    Vector3f edge1, edge2, tvec, pvec, qvec;
    float det, inv_det;

    /* find vectors for two edges sharing vert0 */
    edge1 = b - a;
    edge2 = c - a;

    /* begin calculating determinant - also used to calculate U parameter */
    pvec = Cross(ray.GetDirection(), edge2);

    /* if determinant is near zero, ray lies in plane of triangle */
    det = Dot(edge1, pvec);

    if (Abs(det) < kMinDet)
        return false;

    inv_det = 1.0F / det;

    /* calculate distance from vert0 to ray origin */
    tvec = ray.GetOrigin() - a;

    /* calculate U parameter and test bounds */
    u = Dot(tvec, pvec) * inv_det;
    if (u < 0.0F || u > 1.0F)
        return false;

    /* prepare to test V parameter */
    qvec = Cross(tvec, edge1);

    /* calculate V parameter and test bounds */
    v = Dot(ray.GetDirection(), qvec) * inv_det;
    if (v < 0.0F || u + v > 1.0F)
        return false;

    t = Dot(edge2, qvec) * inv_det;
    if (t < 0.0F)
        return false;
    *outT = t;

    return true;
}

bool IntersectRaySphere(const Ray& ray, const Sphere& inSphere)
{
    Vector3f dif = inSphere.GetCenter() - ray.GetOrigin();
    float d = Dot(dif, ray.GetDirection());
    float lSqr = Dot(dif, dif);
    float rSqr = Sqr(inSphere.GetRadius());

    if (d < 0.0F && lSqr > rSqr)
        return false;

    float mSqr = lSqr - Sqr(d);

    if (mSqr > rSqr)
        return false;
    else
        return true;
}

bool IntersectRaySphere(const Ray& ray, const Sphere& inSphere, float* t0, float* t1)
{
    Assert(t0 != NULL);
    Assert(t1 != NULL);

    Vector3f dif = inSphere.GetCenter() - ray.GetOrigin();
    float d = Dot(dif, ray.GetDirection());
    float lSqr = Dot(dif, dif);
    float rSqr = Sqr(inSphere.GetRadius());

    if (d < 0.0F && lSqr > rSqr)
        return false;

    float mSqr = lSqr - Sqr(d);

    if (mSqr > rSqr)
        return false;

    float q = sqrt(rSqr - mSqr);

    *t0 = d - q;
    *t1 = d + q;

    return true;
}

bool IntersectRayAABB(const Ray& ray, const AABB& inAABB)
{
    float t0, t1;
    return IntersectRayAABB(ray, inAABB, &t0, &t1);
}

bool IntersectRayAABB(const Ray& ray, const AABB& inAABB, float* outT0)
{
    float t1;
    return IntersectRayAABB(ray, inAABB, outT0, &t1);
}

bool IntersectRayAABB(const Ray& ray, const AABB& inAABB, float* outT0, float* outT1)
{
    ASSERT_VALID_AABB(inAABB);

    float tmin = -Vector3f::infinity;
    float tmax = Vector3f::infinity;

    float t0, t1, f;

    Vector3f p = inAABB.GetCenter() - ray.GetOrigin();
    Vector3f extent = inAABB.GetExtent();
    long i;
    for (i = 0; i < 3; i++)
    {
        // ray and plane are parallel so no valid intersection can be found
        {
            f = 1.0F / ray.GetDirection()[i];
            t0 = (p[i] + extent[i]) * f;
            t1 = (p[i] - extent[i]) * f;
            // Ray leaves on Right, Top, Back Side
            if (t0 < t1)
            {
                if (t0 > tmin)
                    tmin = t0;

                if (t1 < tmax)
                    tmax = t1;

                if (tmin > tmax)
                    return false;

                if (tmax < 0.0F)
                    return false;
            }
            // Ray leaves on Left, Bottom, Front Side
            else
            {
                if (t1 > tmin)
                    tmin = t1;

                if (t0 < tmax)
                    tmax = t0;

                if (tmin > tmax)
                    return false;

                if (tmax < 0.0F)
                    return false;
            }
        }
    }

    *outT0 = tmin;
    *outT1 = tmax;

    return true;
}

bool IntersectSphereSphere(const Sphere& s0, const Sphere& s1)
{
    float sqrDist = SqrMagnitude(s0.GetCenter() - s1.GetCenter());
    if (Sqr(s0.GetRadius() + s1.GetRadius()) > sqrDist)
        return true;
    else
        return false;
}

bool IntersectSphereSphereInclusive(const Sphere& s0, const Sphere& s1)
{
    float sqrDist = SqrMagnitude(s0.GetCenter() - s1.GetCenter());
    if (Sqr(s0.GetRadius() + s1.GetRadius()) >= sqrDist)
        return true;
    else
        return false;
}

bool IntersectAABBAABB(const AABB& b0, const AABB& b1)
{
    ASSERT_VALID_AABB(b0);
    ASSERT_VALID_AABB(b1);

    const Vector3f dif = (b1.GetCenter() - b0.GetCenter());

    return Abs(dif.x) < b0.m_Extent.x + b1.m_Extent.x
        && Abs(dif.y) < b0.m_Extent.y + b1.m_Extent.y
        && Abs(dif.z) < b0.m_Extent.z + b1.m_Extent.z;
}

bool IntersectAABBAABB(const MinMaxAABB& a, const MinMaxAABB& b)
{
    if (a.m_Min.x > b.m_Max.x)
        return false;
    if (a.m_Max.x < b.m_Min.x)
        return false;
    if (a.m_Min.y > b.m_Max.y)
        return false;
    if (a.m_Max.y < b.m_Min.y)
        return false;
    if (a.m_Min.z > b.m_Max.z)
        return false;
    if (a.m_Max.z < b.m_Min.z)
        return false;

    return true;
}

bool IntersectionAABBAABB(const MinMaxAABB& a, const MinMaxAABB& b, MinMaxAABB* outBoxIntersect)
{
    if (!IntersectAABBAABB(a, b))
        return false;

    outBoxIntersect->m_Min.x = std::max(a.m_Min.x, b.m_Min.x);
    outBoxIntersect->m_Max.x = std::min(a.m_Max.x, b.m_Max.x);
    outBoxIntersect->m_Min.y = std::max(a.m_Min.y, b.m_Min.y);
    outBoxIntersect->m_Max.y = std::min(a.m_Max.y, b.m_Max.y);
    outBoxIntersect->m_Min.z = std::max(a.m_Min.z, b.m_Min.z);
    outBoxIntersect->m_Max.z = std::min(a.m_Max.z, b.m_Max.z);

    return true;
}

bool IntersectAABBAABBInclusive(const AABB& b0, const AABB& b1)
{
    ASSERT_VALID_AABB(b0);
    ASSERT_VALID_AABB(b1);

    const Vector3f dif = (b1.GetCenter() - b0.GetCenter());

    return Abs(dif.x) <= b0.m_Extent.x + b1.m_Extent.x
        && Abs(dif.y) <= b0.m_Extent.y + b1.m_Extent.y
        && Abs(dif.z) <= b0.m_Extent.z + b1.m_Extent.z;
}

bool IntersectAABBSphere(const AABB& aabb, const Sphere& s)
{
    using namespace math;

    ASSERT_VALID_AABB(aabb);

    CompileTimeAssert(offsetof(AABB, m_Center) + sizeof(float4) <= sizeof(AABB), "It is not safe to use vload4f");
    const float4 point = vload4f(s.GetCenter().GetPtr());
    const float4 center = vload4f(aabb.m_Center.GetPtr());
    const float4 extent = float4(vload3f(aabb.m_Extent.GetPtr()), 0.0f);
    const float4 delta = max(abs(point - center), extent) - extent;
    const float4 delta3 = as_float4(as_int4(delta) & kXYZMask_);

    return csum(delta3 * delta3) < Sqr(s.GetRadius());
}

bool IntersectTransformedAABBSphere(const AABB& aabb, const Matrix4x4f& transform, TransformType type, const Sphere& s)
{
    ASSERT_VALID_AABB(aabb);

    if (!IsNonUniformScaleTransform(type))
    {
        // we can just transform the sphere into local coordinates
        float invScale = IsNoScaleTransform(type) ? 1.0f : 1.0f / Magnitude(transform.GetAxisX());
        Vector3f localSpherePos = transform.InverseMultiplyPoint3Affine(s.GetCenter()) * invScale;
        Sphere localSphere(localSpherePos * invScale, s.GetRadius() * invScale);
        return IntersectAABBSphere(aabb, localSphere);
    }

    // find sphere position relative to box center
    Vector3f boxCenter = transform.MultiplyPoint3(aabb.GetCenter());
    Vector3f relSpherePos = s.GetCenter() - boxCenter;

    // is center of the box inside the sphere?
    if (SqrMagnitude(relSpherePos) < Sqr(s.GetRadius()))
        return true;

    // take scale out of the transform and get normals
    Vector3f scale;
    Vector3f normals[3];
    for (int axis = 0; axis < 3; axis++)
    {
        normals[axis] = transform.GetAxis(axis);
        scale[axis] = Magnitude(normals[axis]);
        if (scale[axis] > 0.0f)
            normals[axis] *= 1.0f / scale[axis];
    }

    bool skewed = IsSkewedBasis(normals, 0.0001f);
    if (skewed)
    {
        // invert, transpose and renormalize
        Matrix4x4f normalMatrix;
        normalMatrix.SetBasis(normals[0], normals[1], normals[2]);
        Matrix4x4f normalInverse;
        Matrix4x4f::Invert_General3D(normalMatrix, normalInverse);
        normalInverse.Transpose();
        for (int axis = 0; axis < 3; axis++)
            normals[axis] = NormalizeSafe(normalInverse.GetAxis(axis));
    }

    // find shortest distance from box to sphere by looking at 1 dimension at a time
    Vector3f axisDistances;
    for (int axis = 0; axis < 3; axis++)
    {
        // find normal of transformed box plane
        const Vector3f& normal = normals[axis];

        // get box extent in direction of normal
        float boxExtent = aabb.GetExtent()[axis];
        float extentProjection;
        if (skewed)
        {
            // transform box bounds into new coordinates
            Vector3f extentVec = transform.GetAxis(axis) * boxExtent;

            // project transformed extents onto normal
            extentProjection = Dot(normal, extentVec);
        }
        else
            extentProjection = boxExtent * scale[axis];

        // project sphere center onto normal
        float sphereProjection = Dot(normal, relSpherePos);

        // distance that sphere projection is outside the extents
        axisDistances[axis] = FloatMax(Abs(sphereProjection) - Abs(extentProjection), 0.0f);
    }

    bool intersects;
    if (skewed)
    {
        // farthest distance from plane
        // this isn't the correct distance around edges and corners, may give false intersections
        float maxDist = FloatMax(FloatMax(axisDistances.x, axisDistances.y), axisDistances.z);
        intersects =  maxDist < s.GetRadius();
    }
    else
    {
        // shortest distance from box to sphere center
        float sqrDist = SqrMagnitude(axisDistances);
        intersects =  sqrDist < Sqr(s.GetRadius());
    }
    return intersects;
}

// Returns n/d clamped to [0,1].  Handles zero denominators.
float SafeClampRatio(float n, float d)
{
    return (n <= 0 ? 0 : (n >= d ? 1 : n / d));
}

bool IntersectCapsuleCapsule(const Capsule& c0, const Capsule& c1)
{
    Vector3f D1 = c0.GetEnd() - c0.GetStart();
    Vector3f D2 = c1.GetEnd() - c1.GetStart();
    Vector3f diff = c0.GetStart() - c1.GetStart();
    float s, t;

    // First, we'll find the minimum distance between the line segments of the capsules.  Let L1, L2 be two lines given
    // as L(t) = A + t D.  We seek the pair s,t such that d(s,t) = |L1(s) - L2(t)|^2 is minimized for (s,t) constrained
    // to the unit square.
    //
    // The code below is based on the "non-robust" version in Dave Eberly's "Robust Computation of Distance Between
    // Line Segments" (http://www.geometrictools.com/Documentation/DistanceLine3Line3.pdf).
    // TODO:  implement the robust version from the paper.
    //
    // Expanding,
    //   d(s,t) = |(A1 - A2) + s D1 - t D2|^2.
    //          = s^2 |D1|^2 - 2 s t D1^T D2 + t^2 |D2|^2 + 2 s D1^T (A1-A2) - 2 t D2^T (A1-A2) + |A1 - A2|^2
    //          = a s^2 - 2b s t + c t^2 + 2d s - 2e t + f,
    //
    // where
    float a = Dot(D1, D1);
    float b = Dot(D1, D2);
    float c = Dot(D2, D2);
    float d = Dot(D1, diff);
    float e = Dot(D2, diff);

    // This is a quadratic optimization problem.  Differentiating this wrt both s and t and  setting the result to zero
    // yields two equations in two unknowns:
    //   a s - b t + d = 0
    //   c t - b s - e = 0
    //
    // Solving this for s and t:
    //   s = (b e - c d) / det
    //   t = (a e - b d) / det,
    //
    // where
    float det = a * c - b * b;

    // Recall Lagrange's identity from vector calculus:
    //   (A^T C) (B^T D) - (A^T D) (B^T C) = (AxB)^T (CxD),
    // and note that the definition of det expands to
    //   |D2|^2 |D1|^2 - (D1^T D2)^2.
    //
    // Bringing the latter into the form of the left-hand side of the identity yields
    //   (D2^T D2) (D1^T D1) - (D2^T D1) (D1^T D2) = (D2 x D1)^T (D2 x D1)
    //   det = |D2 x D1|^2.
    //
    // In other words, det is always non-negative, and zero if and only if the line segments are parallel.
    if (det > Vector3f::epsilon)
    {
        // Don't divide by the determinant yet (and in general, move divisions to the very end to improve accuracy).
        s = b * e - c * d;
        t = a * e - b * d;

        // If (s,t) is in [0,1]^2, then we're done.  Otherwise, we do case analysis to determine the nearest points.
        //
        // To do that, we're going to compute line to endpoint distances.  These can be expressed in terms of
        // the values we've already computed:
        //
        //     Point     Line               Expr                 Optimized
        //       A1       L2       D2^T (A1-A2) / |D2|^2     =     e / c
        //      A1+D1     L2       D2^T (A1+D1-A2) / |D2|^2  =  (b + e) / c
        //       A2       L1       D1^T (A2-A1) / |D1|^2     =    -d / a
        //      A2+D2     L1       D1^T (A2+D2-A1) / |D1|^2  =  (b - d) / a
        //
        if (s <= 0)
        {
            // The projection of A1 onto L2 is t = e / c.  Figure out what region that lies in (negative, positive, or inside the interval).
            if (e <= 0)
            {
                t = 0;
                s = SafeClampRatio(-d, a);
            }
            else if (e < c) // t in (0,1)
            {
                s = 0;
                t = e / c;
            }
            else // t >= 1
            {
                t = 1;
                s = SafeClampRatio(b - d, a);
            }
        }
        else
        {
            if (s >= det) // s >= 1
            {
                // The projection of A1+D1 onto L2 is t = (b + e) / c.
                if (b + e <= 0) // t <= 0
                {
                    t = 0;
                    s = SafeClampRatio(-d , a);
                }
                else if (b + e < c) // t in (0,1)
                {
                    s = 1;
                    t = (b + e) / c;
                }
                else
                {
                    t = 1;
                    s = SafeClampRatio(b - d, a);
                }
            }
            else // s in (0,1)
            {
                if (t <= 0)
                {
                    t = 0;
                    s = SafeClampRatio(-d, a);
                }
                else
                {
                    if (t >= det) // t >= 1
                    {
                        t = 1;
                        s = SafeClampRatio(b - d, a);
                    }
                    else
                    {
                        // the minimum is inside the unit square, just compute the unconstrained version
                        s /= det;
                        t /= det;
                    }
                }
            }
        }
    }
    else
    {
        // Parallel axes, pick the endpoints
        if (e <= 0)  // Proj(A1,L2) = e / c
        {
            t = 0;
            s = SafeClampRatio(-d, a);
        }
        else if (e < c)  // t in (0,1)
        {
            s = 0;
            t = e / c;
        }
        else
        {
            t = 1;
            s = SafeClampRatio(b - d, a);
        }
    }

    // Finally, get the distance between points and compare to the sum of radiuses
    Vector3f comp = Lerp(c0.GetStart(), c0.GetEnd(), s) - Lerp(c1.GetStart(), c1.GetEnd(), t);
    float radSum = c0.GetRadius() + c1.GetRadius();
    return (SqrMagnitude(comp) <= radSum * radSum);
}

bool IntersectCapsuleSphere(const Capsule& capsule, const Sphere& sphere)
{
    // Project the point to the line:  if f(t) = |A + t D - P|^2, then the minimum of d occurs at df/dt=0, so
    // we have t = D^T (P-A) / |D|^2.  Then just clamp t to [0,1] and compute the distance to that point.
    Vector3f D = capsule.GetEnd() - capsule.GetStart();
    float magSq = SqrMagnitude(D);

    float t = 0;
    if (magSq >= Vector3f::epsilon)
    {
        t = clamp01(Dot(D, sphere.GetCenter() - capsule.GetStart()) / magSq);
    }

    float distSq = SqrMagnitude(sphere.GetCenter() - Lerp(capsule.GetStart(), capsule.GetEnd(), t));
    float radSum = capsule.GetRadius() + sphere.GetRadius();

    return (distSq <= radSum * radSum);
}

// possible optimization: Precalculate the Abs () of the plane. (3 fabs less per plane)
bool IntersectAABBFrustum(
    const AABB& a,
    const Plane* p,
    UInt32 inClipMask)
{
    ASSERT_VALID_AABB(a);

    const Vector3f& m       = a.GetCenter(); // center of AABB
    const Vector3f& extent  = a.GetExtent(); // half-diagonal
    UInt32 mk       = 1;

    // loop while there are active planes..
    while (mk <= inClipMask)
    {
        // if clip plane is active...
        if (inClipMask & mk)
        {
            const Vector3f& normal = p->GetNormal();
            float dist = p->GetDistanceToPoint(m);
            float radius = Dot(extent, Abs(normal));

            if (dist + radius < 0)
                return false;                    // behind clip plane
            //  if (dist - radius < 0) *outClipMask |= mk; // straddles clipplane
            // else in front of clip plane-> leave outClipMask bit off
            //          float m = (p->a () * b->v[p->nx].x) + (p->b * b->v[p->ny].y) + (p->c * b->v[p->nz].z);
            //          if (m > -p->d ()) return OUTSIDE;
            //          float r = Dot (m, normal) + p->d ();
            //          float n = (extent.x * Abs(normal.x)) + (extent.y * Abs(normal.y)) + (extent.z * Abs(normal.z));
            //          if (r + n < 0) return false;
        }
        mk += mk;
        p++; // next plane
    }
    return true; // AABB intersects frustum
}

// Optimize like this: http://www.cg.tuwien.ac.at/studentwork/CESCG/CESCG-2002/DSykoraJJelinek/

bool IntersectAABBFrustumFull(const AABB& a, const Plane p[6])
{
    return IntersectAABBPlaneBounds(a, p, 6);
}

bool IntersectAABBPlaneBounds(const AABB& a, const Plane* p, const int planeCount)
{
    ASSERT_VALID_AABB(a);

    const Vector3f& m       = a.GetCenter(); // center of AABB
    const Vector3f& extent  = a.GetExtent(); // half-diagonal

    for (int i = 0; i < planeCount; ++i, ++p)
    {
        const Vector3f& normal = p->GetNormal();
        float dist = p->GetDistanceToPoint(m);
        float radius = Dot(extent, Abs(normal));
        if (dist + radius < 0)
            return false;                    // behind clip plane
    }
    return true;
}

int PrepareOptimizedPlanes(const Plane* planes, const int planeCount, math::float4* outPlanesSOA)
{
    const Vector4f* plane4 = reinterpret_cast<const Vector4f*>(planes);
    int i = 0;
    while (i < planeCount)
    {
        int indices[4];
        indices[0] = i;
        indices[1] = std::min(i + 1, planeCount - 1);
        indices[2] = std::min(i + 2, planeCount - 1);
        indices[3] = std::min(i + 3, planeCount - 1);

        for (int k = 0; k < 4; k++)
            outPlanesSOA[i + k] = math::float4(plane4[indices[0]][k], plane4[indices[1]][k], plane4[indices[2]][k], plane4[indices[3]][k]);

        i += 4;
    }
    return i;
}

void PrepareOptimizedPlanes(const Plane* planes, const int planeCount, math::float4* outPlanesSOA, int maxOutputPlaneCount)
{
    int res = PrepareOptimizedPlanes(planes, planeCount, outPlanesSOA);
    Assert(res <= maxOutputPlaneCount);
}

bool IntersectAABBPlaneBoundsOptimized(const AABB& a, const math::float4* optimizedPlanes, const int planeCount)
{
    ASSERT_VALID_AABB(a);

    using namespace math;

    float4 dist4, radius4;

    const float4 zero = float4(ZERO);

    float3 centerXYZ = vload3f(a.GetCenter().GetPtr());
    float3 extentXYZ = vload3f(a.GetExtent().GetPtr());

    const float1 centerX4 = centerXYZ.x;
    const float1 centerY4 = centerXYZ.y;
    const float1 centerZ4 = centerXYZ.z;

    const float1 extentX4 = extentXYZ.x;
    const float1 extentY4 = extentXYZ.y;
    const float1 extentZ4 = extentXYZ.z;

    for (int i = 0; i < planeCount; i += 4)
    {
        const float4 planeX4 = optimizedPlanes[i];
        const float4 planeY4 = optimizedPlanes[i + 1];
        const float4 planeZ4 = optimizedPlanes[i + 2];
        const float4 planeW4 = optimizedPlanes[i + 3];

        const float4 absPlaneX4 = abs(planeX4);
        const float4 absPlaneY4 = abs(planeY4);
        const float4 absPlaneZ4 = abs(planeZ4);

        // get the distances to 4 planes
        dist4 =  centerX4 * planeX4 + planeW4;
        dist4 += centerY4 * planeY4;
        dist4 += centerZ4 * planeZ4;

        // get the radii to 4 planes
        radius4  = extentX4 * absPlaneX4;
        radius4 += extentY4 * absPlaneY4;
        radius4 += extentZ4 * absPlaneZ4;

        if (any(dist4 + radius4 < zero))
            return false;
    }

    return true;
}

bool IntersectSpherePlaneBoundsOptimized(const math::float4& sphere, const math::float4* optimizedPlanes, const int planeCount)
{
    using namespace math;

    float4 dist4;

    const float1 centerX4 = sphere.x;
    const float1 centerY4 = sphere.y;
    const float1 centerZ4 = sphere.z;
    const float1 neg_radius = -sphere.w;

    for (int i = 0; i < planeCount; i += 4)
    {
        const float4 planeX4 = optimizedPlanes[i];
        const float4 planeY4 = optimizedPlanes[i + 1];
        const float4 planeZ4 = optimizedPlanes[i + 2];
        const float4 planeW4 = optimizedPlanes[i + 3];

        // get the distances to 4 planes
        dist4 =  centerX4 * planeX4 + planeW4;
        dist4 += centerY4 * planeY4;
        dist4 += centerZ4 * planeZ4;

        if (any(dist4 < neg_radius))
            return false;
    }

    return true;
}

// Returns the shortest distance to the front facing plane from the ray.
// Return -1 if no plane intersect this ray.
// planeNumber will contain the index of the plane found or -1.
float RayDistanceToFrustumOriented(const Ray& ray, const Plane* p, const int planeCount, int& planeNumber)
{
    float maxDistance = std::numeric_limits<float>::infinity();
    planeNumber = -1;
    for (int i = 0; i < planeCount; ++i, ++p)
    {
        float dist;
        if (IntersectRayPlaneOriented(ray, *p, &dist) && (dist < maxDistance))
        {
            maxDistance = dist;
            planeNumber = i;
        }
    }
    if (planeNumber != -1)
        return maxDistance;
    else
        return -1.0f;
}

// Returns the shortest distance to planes if point is outside (positive float),
// and 0.0 if point is inside frustum planes
float PointDistanceToFrustum(const Vector4f& point, const Plane* p, const int planeCount)
{
    float maxDistanceNegative = -std::numeric_limits<float>::infinity();
    for (int i = 0; i < planeCount; ++i, ++p)
    {
        float dist = p->GetDistanceToPoint(point);
        if ((dist < 0.0f) && (dist > maxDistanceNegative))
            maxDistanceNegative = dist;
    }
    if (maxDistanceNegative != -std::numeric_limits<float>::infinity())
        return -maxDistanceNegative;
    else
        return 0.0f;
}

bool IntersectTriTri(const Vector3f& a0, const Vector3f& b0, const Vector3f& c0,
    const Vector3f& a1, const Vector3f& b1, const Vector3f& c1,
    Vector3f* intersectionLine0, Vector3f* intersectionLine1, bool* coplanar)
{
    int coplanarInt;
    bool ret;
    ret = tri_tri_intersect_with_isectline(const_cast<Vector3f&>(a0).GetPtr(), const_cast<Vector3f&>(b0).GetPtr(), const_cast<Vector3f&>(c0).GetPtr(),
        const_cast<Vector3f&>(a1).GetPtr(), const_cast<Vector3f&>(b1).GetPtr(), const_cast<Vector3f&>(c1).GetPtr(),
        &coplanarInt, intersectionLine0->GetPtr(), intersectionLine1->GetPtr());
    *coplanar = coplanarInt;
    return ret;
}

bool IntersectRayPlane(const Ray& ray, const Plane& plane, float* enter)
{
    Assert(enter != NULL);
    float vdot = Dot(ray.GetDirection(), plane.GetNormal());
    float ndot = -Dot(ray.GetOrigin(), plane.GetNormal()) - plane.d();

    // is line parallel to the plane? if so, even if the line is
    // at the plane it is not considered as intersection because
    // it would be impossible to determine the point of intersection
    if (CompareApproximately(vdot, 0.0F))
        return false;

    // the resulting intersection is behind the origin of the ray
    // if the result is negative ( enter < 0 )
    *enter = ndot / vdot;

    return *enter > 0.0F;
}

bool IntersectRayPlaneOriented(const Ray& ray, const Plane& plane, float* enter)
{
    Assert(enter != NULL);
    float vdot = Dot(ray.GetDirection(), plane.GetNormal());
    float ndot = -Dot(ray.GetOrigin(), plane.GetNormal()) - plane.d();

    //No collision if the ray it the plane from behind
    if (vdot > 0)
        return false;

    // is line parallel to the plane? if so, even if the line is
    // at the plane it is not considered as intersection because
    // it would be impossible to determine the point of intersection
    if (CompareApproximately(vdot, 0.0F))
        return false;

    // the resulting intersection is behind the origin of the ray
    // if the result is negative ( enter < 0 )
    *enter = ndot / vdot;

    return *enter > 0.0F;
}

bool IntersectSegmentPlane(const Vector3f& p1, const Vector3f& p2, const Plane& plane, Vector3f* result)
{
    Assert(result != NULL);
    Vector3f vec = p2 - p1;
    float vdot = Dot(vec, plane.GetNormal());

    // segment parallel to the plane
    if (CompareApproximately(vdot, 0.0f))
        return false;

    float ndot = -Dot(p1, plane.GetNormal()) - plane.d();
    float u = ndot / vdot;
    // intersection is out of segment
    if (u < 0.0f || u > 1.0f)
        return false;

    *result = p1 + vec * u;
    return true;
}

bool IntersectLineSegmentWithLine(const Vector2f& p1, const Vector2f& p2, const Vector2f& p3, const Vector2f& p4, Vector2f& result)
{
    // Tried a direct conversion to SIMD but runs almost twice as slow due to vloads
    float bx = p2.x - p1.x;
    float by = p2.y - p1.y;
    float dx = p4.x - p3.x;
    float dy = p4.y - p3.y;
    float bDotDPerp = bx * dy - by * dx;
    if (CompareApproximately(bDotDPerp, 0.0f))
    {
        return false;
    }
    float cx = p3.x - p1.x;
    float cy = p3.y - p1.y;
    float t = (cx * dy - cy * dx) / bDotDPerp;
    if ((t >= -Vector2f::epsilon) && (t <= 1.0f + Vector2f::epsilon))
    {
        result = Vector2f(p1.x + t * bx, p1.y + t * by);
        return true;
    }
    return false;
}

bool IntersectSphereTriangle(const Sphere& s, const Vector3f& vert0, const Vector3f& vert1, const Vector3f& vert2)
{
    const Vector3f& center = s.GetCenter();
    float radius = s.GetRadius();
    float radius2 = radius * radius;
    Vector3f Diff;

    // Early exit if one of the vertices is inside the sphere
    float sqrDiff;
    Diff = vert1 - center;
    sqrDiff = SqrMagnitude(Diff);
    if (sqrDiff <= radius2)
        return true;

    Diff = vert2 - center;
    sqrDiff = SqrMagnitude(Diff);
    if (sqrDiff <= radius2)
        return true;

    Diff = vert0 - center;
    sqrDiff = SqrMagnitude(Diff);
    if (sqrDiff <= radius2)
        return true;

    // Else do the full distance test
    Vector3f Edge0  = vert1 - vert0;
    Vector3f Edge1  = vert2 - vert0;

    float A00 = Dot(Edge0, Edge0);
    float A01 = Dot(Edge0, Edge1);
    float A11 = Dot(Edge1, Edge1);

    float B0 = Dot(Diff, Edge0);
    float B1 = Dot(Diff, Edge1);

    float C = Dot(Diff, Diff);

    float Det = Abs(A00 * A11 - A01 * A01);
    float u = A01 * B1 - A11 * B0;
    float v = A01 * B0 - A00 * B1;

    float DistSq;
    if (u + v <= Det)
    {
        if (u < 0.0F)
        {
            if (v < 0.0F)
            {
                // region 4
                if (B0 < 0.0F)
                {
                    if (-B0 >= A00)
                        DistSq = A00 + 2.0F * B0 + C;
                    else
                    {
                        u = -B0 / A00;
                        DistSq = B0 * u + C;
                    }
                }
                else
                {
                    if (B1 >= 0.0F)
                        DistSq = C;
                    else if (-B1 >= A11)
                        DistSq = A11 + 2.0F * B1 + C;
                    else
                    {
                        v = -B1 / A11;
                        DistSq = B1 * v + C;
                    }
                }
            }
            else
            {  // region 3
                if (B1 >= 0.0F)
                    DistSq = C;
                else if (-B1 >= A11)
                    DistSq = A11 + 2.0F * B1 + C;
                else
                {
                    v = -B1 / A11;
                    DistSq = B1 * v + C;
                }
            }
        }
        else if (v < 0.0F)
        {  // region 5
            if (B0 >= 0.0F)
                DistSq = C;
            else if (-B0 >= A00)
                DistSq = A00 + 2.0F * B0 + C;
            else
            {
                u = -B0 / A00;
                DistSq = B0 * u + C;
            }
        }
        else
        {  // region 0
           // minimum at interior point
            if (Det == 0.0F)
                DistSq = std::numeric_limits<float>::max();
            else
            {
                float InvDet = 1.0F / Det;
                u *= InvDet;
                v *= InvDet;
                DistSq = u * (A00 * u + A01 * v + 2.0F * B0) + v * (A01 * u + A11 * v + 2.0F * B1) + C;
            }
        }
    }
    else
    {
        double Tmp0, Tmp1, Numer, Denom;

        if (u < 0.0F)
        {
            // region 2
            Tmp0 = A01 + B0;
            Tmp1 = A11 + B1;
            if (Tmp1 > Tmp0)
            {
                Numer = Tmp1 - Tmp0;
                Denom = A00 - 2.0F * A01 + A11;
                if (Numer >= Denom)
                    DistSq = A00 + 2.0F * B0 + C;
                else
                {
                    u = Numer / Denom;
                    v = 1.0 - u;
                    DistSq = u * (A00 * u + A01 * v + 2.0F * B0) + v * (A01 * u + A11 * v + 2.0F * B1) + C;
                }
            }
            else
            {
                if (Tmp1 <= 0.0F)
                    DistSq = A11 + 2.0F * B1 + C;
                else if (B1 >= 0.0)
                    DistSq = C;
                else
                {
                    v = -B1 / A11;
                    DistSq = B1 * v + C;
                }
            }
        }
        else if (v < 0.0)
        {  // region 6
            Tmp0 = A01 + B1;
            Tmp1 = A00 + B0;
            if (Tmp1 > Tmp0)
            {
                Numer = Tmp1 - Tmp0;
                Denom = A00 - 2.0F * A01 + A11;
                if (Numer >= Denom)
                    DistSq = A11 + 2.0 * B1 + C;
                else
                {
                    v = Numer / Denom;
                    u = 1.0F - v;
                    DistSq =  u * (A00 * u + A01 * v + 2.0F * B0) + v * (A01 * u + A11 * v + 2.0F * B1) + C;
                }
            }
            else
            {
                if (Tmp1 <= 0.0F)
                    DistSq = A00 + 2.0F * B0 + C;
                else if (B0 >= 0.0F)
                    DistSq = C;
                else
                {
                    u = -B0 / A00;
                    DistSq = B0 * u + C;
                }
            }
        }
        else
        {
            // region 1
            Numer = A11 + B1 - A01 - B0;
            if (Numer <= 0.0F)
                DistSq = A11 + 2.0F * B1 + C;
            else
            {
                Denom = A00 - 2.0F * A01 + A11;
                if (Numer >= Denom)
                    DistSq = A00 + 2.0F * B0 + C;
                else
                {
                    u = Numer / Denom;
                    v = 1.0F - u;
                    DistSq = u * (A00 * u + A01 * v + 2.0F * B0) + v * (A01 * u + A11 * v + 2.0F * B1) + C;
                }
            }
        }
    }

    return Abs(DistSq) <= radius2;
}

bool TestPlanesAABB(const Plane* planes, const int planeCount, const AABB& bounds)
{
    UInt32 planeMask = 0;
    if (planeCount == 6)
        planeMask = 63;
    else
    {
        for (int i = 0; i < planeCount; ++i)
            planeMask |= 1 << i;
    }

    return IntersectAABBFrustum(bounds, planes, planeMask);
}
