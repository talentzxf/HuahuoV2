//
// Created by VincentZhang on 5/20/2022.
//

#ifndef HUAHUOENGINE_CAMERAUTIL_H
#define HUAHUOENGINE_CAMERAUTIL_H

#include "Modules/ExportModules.h"
#include "Math/Matrix4x4.h"
#include "Geometry/Plane.h"
#include "Math/Rect.h"

// Extract frustum planes from Projection matrix.  Use ExtractProjectionPlanesRobust() whenever zNear/zFar can be expected
// to be extremely small.
void EXPORT_COREMODULE ExtractProjectionPlanes(const Matrix4x4f& projection, Plane* planes);
void EXPORT_COREMODULE ExtractProjectionPlanesRobust(const Matrix4x4f& projection, Plane* planes);
void EXPORT_COREMODULE ExtractProjectionNearPlane(const Matrix4x4f& projection, Plane* outPlane);

bool CameraProject(const Vector3f& p, const Matrix4x4f& cameraToWorld, const Matrix4x4f& worldToClip, const RectInt& viewport, Vector3f& outP, bool offscreen);

// Screen point to world point
// p = screen point (x, y = in pixels inside the viewport, z = world space distance from the camera)
//
// sets outP to (0,0,0) if fails.
bool CameraUnProject(const Vector3f& p, const Matrix4x4f& cameraToWorld, const Matrix4x4f& clipToWorld, const RectInt& viewport, Vector3f& outP, bool offscreen);

Rectf RectIntToRectf(const RectInt& r);
RectInt RectfToRectInt(const Rectf& r);
void SetGLViewport(const Rectf& pixelRect);

#endif //HUAHUOENGINE_CAMERAUTIL_H
