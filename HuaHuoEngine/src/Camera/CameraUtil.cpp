//
// Created by VincentZhang on 5/20/2022.
//

#include "CameraUtil.h"

template<bool RobustNearPlane>
struct ExtractProjectionPlanesHelper
{
    static void Apply(const Matrix4x4f &finalMatrix, Plane *outPlanes)
    {
        float tmpVec[4];
        float otherVec[4];

        tmpVec[0] = finalMatrix.Get(3, 0);
        tmpVec[1] = finalMatrix.Get(3, 1);
        tmpVec[2] = finalMatrix.Get(3, 2);
        tmpVec[3] = finalMatrix.Get(3, 3);

        otherVec[0] = finalMatrix.Get(0, 0);
        otherVec[1] = finalMatrix.Get(0, 1);
        otherVec[2] = finalMatrix.Get(0, 2);
        otherVec[3] = finalMatrix.Get(0, 3);

        // left & right
        outPlanes[kPlaneFrustumLeft].SetABCD(otherVec[0] + tmpVec[0], otherVec[1] + tmpVec[1], otherVec[2] + tmpVec[2], otherVec[3] + tmpVec[3]);
        outPlanes[kPlaneFrustumLeft].NormalizeUnsafe();
        outPlanes[kPlaneFrustumRight].SetABCD(-otherVec[0] + tmpVec[0], -otherVec[1] + tmpVec[1], -otherVec[2] + tmpVec[2], -otherVec[3] + tmpVec[3]);
        outPlanes[kPlaneFrustumRight].NormalizeUnsafe();

        // bottom & top
        otherVec[0] = finalMatrix.Get(1, 0);
        otherVec[1] = finalMatrix.Get(1, 1);
        otherVec[2] = finalMatrix.Get(1, 2);
        otherVec[3] = finalMatrix.Get(1, 3);

        outPlanes[kPlaneFrustumBottom].SetABCD(otherVec[0] + tmpVec[0], otherVec[1] + tmpVec[1], otherVec[2] + tmpVec[2], otherVec[3] + tmpVec[3]);
        outPlanes[kPlaneFrustumBottom].NormalizeUnsafe();
        outPlanes[kPlaneFrustumTop].SetABCD(-otherVec[0] + tmpVec[0], -otherVec[1] + tmpVec[1], -otherVec[2] + tmpVec[2], -otherVec[3] + tmpVec[3]);
        outPlanes[kPlaneFrustumTop].NormalizeUnsafe();

        otherVec[0] = finalMatrix.Get(2, 0);
        otherVec[1] = finalMatrix.Get(2, 1);
        otherVec[2] = finalMatrix.Get(2, 2);
        otherVec[3] = finalMatrix.Get(2, 3);

        // near & far
        outPlanes[kPlaneFrustumNear].SetABCD(otherVec[0] + tmpVec[0], otherVec[1] + tmpVec[1], otherVec[2] + tmpVec[2], otherVec[3] + tmpVec[3]);
        outPlanes[kPlaneFrustumNear].NormalizeUnsafe();

        outPlanes[kPlaneFrustumFar].SetABCD(-otherVec[0] + tmpVec[0], -otherVec[1] + tmpVec[1], -otherVec[2] + tmpVec[2], -otherVec[3] + tmpVec[3]);

        if (RobustNearPlane)
        {
            // for cases where zNear/zFar can be expected to be extremely small.
            outPlanes[kPlaneFrustumFar].NormalizeRobust(1.0e-16f);
        }
        else
        {
            outPlanes[kPlaneFrustumFar].NormalizeUnsafe();
        }
    }
};

void ExtractProjectionPlanes(const Matrix4x4f& finalMatrix, Plane* outPlanes)
{
    ExtractProjectionPlanesHelper<false>::Apply(finalMatrix, outPlanes);
}

void ExtractProjectionPlanesRobust(const Matrix4x4f& finalMatrix, Plane* outPlanes)
{
    ExtractProjectionPlanesHelper<true>::Apply(finalMatrix, outPlanes);
}

void ExtractProjectionNearPlane(const Matrix4x4f& finalMatrix, Plane* outPlane)
{
    float tmpVec[4];
    float otherVec[4];

    tmpVec[0] = finalMatrix.Get(3, 0);
    tmpVec[1] = finalMatrix.Get(3, 1);
    tmpVec[2] = finalMatrix.Get(3, 2);
    tmpVec[3] = finalMatrix.Get(3, 3);

    otherVec[0] = finalMatrix.Get(2, 0);
    otherVec[1] = finalMatrix.Get(2, 1);
    otherVec[2] = finalMatrix.Get(2, 2);
    otherVec[3] = finalMatrix.Get(2, 3);

    // near
    outPlane->SetABCD(otherVec[0] + tmpVec[0], otherVec[1] + tmpVec[1], otherVec[2] + tmpVec[2], otherVec[3] + tmpVec[3]);
    outPlane->NormalizeUnsafe();
}

bool CameraProject(const Vector3f& p, const Matrix4x4f& cameraToWorld, const Matrix4x4f& worldToClip, const RectInt& viewport, Vector3f& outP, bool offscreen)
{
    Vector3f clipPoint;
    if (worldToClip.PerspectiveMultiplyPoint3(p, clipPoint))
    {
        Vector3f cameraPos = cameraToWorld.GetPosition();
        Vector3f dir = p - cameraPos;
        // The camera/projection matrices follow OpenGL convention: positive Z is towards the viewer.
        // So negate it to get into Unity convention.
        Vector3f forward = -cameraToWorld.GetAxisZ();
        float dist = Dot(dir, forward);

        outP.x = viewport.x + (1.0f + clipPoint.x) * viewport.width * 0.5f;
        outP.y = viewport.y + (1.0f + clipPoint.y) * viewport.height * 0.5f;
        //outP.z = (1.0f + clipPoint.z) * 0.5f;
        outP.z = dist;

        return true;
    }

    outP.Set(0.0f, 0.0f, 0.0f);
    return false;
}