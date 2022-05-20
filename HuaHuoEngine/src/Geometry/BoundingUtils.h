//
// Created by VincentZhang on 5/20/2022.
//

#ifndef HUAHUOENGINE_BOUNDINGUTILS_H
#define HUAHUOENGINE_BOUNDINGUTILS_H
#include "Math/Vector3f.h"

class Capsule;
class Matrix4x4f;
class MinMaxAABB;
class Plane;
class Sphere;

struct SpotLightBounds
{
    enum { kPointCount = 5 };
    Vector3f points[kPointCount];
};

//    7-----6   far
//   /     /|
//  3-----2 |
//  | 4   | 5
//  |     |/
//  0-----1     near

void GetFrustumPoints(const Matrix4x4f& clipToWorld, Vector3f* frustum);
void GetFrustumPortion(const Vector3f* frustum, float nearSplit, float farSplit, Vector3f* outPortion);
void CalcHullBounds(const Vector3f* __restrict hullPoints, const UInt8* __restrict hullCounts, UInt8 hullFaces, const Matrix4x4f& cameraWorldToClip, MinMaxAABB& aabb);
void CalculateFocusedLightHull(const Vector3f* frustum, const Vector3f& lightDir, const MinMaxAABB& sceneAABB, dynamic_array<Vector3f>& points);
void CalculateBoundingSphereFromFrustumPoints(const Vector3f points[8], Vector3f& outCenter, float& outRadius);
void CalculateSphereFrom4Points(const Vector3f points[4], Vector3f& outCenter, float& outRadius);
void CalculateSpotLightBounds(const float range, const float cotanHalfSpotAngle, const Matrix4x4f& lightMatrix, SpotLightBounds& outBounds);
void CalculateSpotLightMinSphere(const float range, const float cotanHalfSpotAngle, const Matrix4x4f& lightMatrix, Sphere& outSphere);
bool IsCapsuleInsideSphere(const Capsule& capsule, const Sphere& sphere);


#endif //HUAHUOENGINE_BOUNDINGUTILS_H
