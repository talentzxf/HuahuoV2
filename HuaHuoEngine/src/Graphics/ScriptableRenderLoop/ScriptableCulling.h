//
// Created by VincentZhang on 5/19/2022.
//

#ifndef HUAHUOENGINE_SCRIPTABLECULLING_H
#define HUAHUOENGINE_SCRIPTABLECULLING_H


#include "Math/Vector3f.h"
#include "Math/Matrix4x4.h"
#include "Math/Rect.h"
#include "Camera/CullingParameters.h"
#include "Camera/CameraCullingParameters.h"
#include "ScritableCulling.h"
#include "ScriptableRenderContext.h"

class Camera;
struct CoreCameraValues
{
    int         filterMode;
    UInt32      cullingMask;
    InstanceID  instanceID; // the istance ID to identify the per-camera IntermediateRenderer storage and LOD fade values
};

enum ReflectionProbeSortingCriteria
{
    kProbeSortNone,
    kProbeSortImportance,
    kProbeSortSize,
    kProbeSortImportanceThenSize
};

struct ScriptableCullingParameters : public CullingParameters
{
    float                       shadowDistance;

    CullingOptions              cullingOptions;

    UInt32                      reflectionProbeSortOptions;

    struct CameraProperties
    {
        Rectf screenRect;
        Vector3f viewDir;
        float projectionNear;
        float projectionFar;
        float cameraNear;
        float cameraFar;
        float cameraAspect;

        Matrix4x4f cameraToWorld;
        Matrix4x4f actualWorldToClip;
        Matrix4x4f cameraClipToWorld;
        Matrix4x4f cameraWorldToClip;
        Matrix4x4f implicitProjection;
        Matrix4x4f stereoWorldToClip[2];
        Matrix4x4f worldToCamera;

        Vector3f up;
        Vector3f right;
        Vector3f transformDirection;
        Vector3f cameraEuler;
        Vector3f velocity;

        float farPlaneWorldSpaceLength;

        unsigned int rendererCount;

        Plane shadowCullPlanes[6];
        Plane cameraCullPlanes[6];

        float baseFarDistance;

        Vector3f shadowCullCenter;
        float layerCullDistances[kNumLayers];
        int layerCullSpherical;

        CoreCameraValues coreCameraValues;
        unsigned int cameraType;
        int projectionIsOblique;
        int isImplicitProjectionMatrix;
    } cameraProperties;

    float                       accurateOcclusionThreshold;
    int                         maximumPortalCullingJobs;


    Matrix4x4f                  cullStereoView;
    Matrix4x4f                  cullStereoProj;
    float                       cullStereoSeparation;
    int                         maximumVisibleLights;
    ScriptableCullingParameters() = default;
};

Camera& GetCullingCameraAndSetCullingFlag(Camera& camera, ScriptableCullingParameters& cullingParameters);
bool GetScriptableCullingParameters(Camera& camera, bool stereoAware, ScriptableCullingParameters& cullingParameters);
ScriptableCullResults* CullScriptable(const ScriptableRenderContext &context, const ScriptableCullingParameters& cullingParameters);

#endif //HUAHUOENGINE_SCRIPTABLECULLING_H
