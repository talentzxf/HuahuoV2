//
// Created by VincentZhang on 5/19/2022.
//

#include "ScriptableCulling.h"
#include "Utilities/EnumFlags.h"
#include "Camera/Camera.h"
#include "Components/Transform/Transform.h"
#include "Geometry/BoundingUtils.h"

static bool CanUseProjectionForShadows(const Matrix4x4f& clipToWorld, float farNearRatio, const Vector3f& cameraPos)
{
    // Check if eye position is the focal point of the camera projection
    // and we can scale near plane from the eye pos to get the far plane.
    // This works for asymmetric projections, not for oblique etc.
    Vector3f cameraFrustum[8];
    GetFrustumPoints(clipToWorld, cameraFrustum);
    for (int i = 0; i < 4; i++)
    {
        const Vector3f& nearPos = cameraFrustum[i];
        const Vector3f& farPos = cameraFrustum[i + 4];
        Vector3f derivedFar = cameraPos + (nearPos - cameraPos) * farNearRatio;
        float diff = SqrMagnitude(derivedFar - farPos);
        float length = SqrMagnitude(farPos - nearPos);
        if (!(diff <= length * 0.01f))
        {
            return false;
        }
    }
    return true;
}

Camera& GetCullingCameraAndSetCullingFlag(Camera& camera, ScriptableCullingParameters& cullingParameters)
{
    // If occlusion visualization is enabled replace scriptable camera for preview occlusion camera
    // to be able to show properly occlusion visualization window.
    Camera* cullingCamera = &camera;
    bool occlusionCull = camera.GetUseOcclusionCulling();
#if UNITY_EDITOR
    if (GetOcclusionCullingVisualization()->GetShowOcclusionCulling() && !GetOcclusionCullingVisualization()->GetShowPreVis())
    {
        Camera* previewOcclusionCamera = FindPreviewOcclusionCamera();
        if (previewOcclusionCamera != NULL)
        {
            if (previewOcclusionCamera->IsValidToRender())
            {
                cullingCamera = previewOcclusionCamera;
                occlusionCull = previewOcclusionCamera->GetUseOcclusionCulling() && GetOcclusionCullingVisualization()->GetShowGeometryCulling();
            }
            else
            {
                ErrorStringWithoutStacktrace("Camera set to be used as occlusion visualization camera was not valid for rendering.");
            }
        }
    }
#endif

    if (occlusionCull)
        cullingParameters.cullingOptions = SetFlags(cullingParameters.cullingOptions, kCullFlagOcclusionCull);

    return *cullingCamera;
}

void GetCoreCameraValues(const Camera& camera, CoreCameraValues& coreCameraValues)
{
#if UNITY_EDITOR
    coreCameraValues.filterMode = camera.GetFilterMode();
#endif
    coreCameraValues.cullingMask = camera.GetCullingMask();
    coreCameraValues.instanceID = camera.GetInstanceID();
}

// Before exposing the job count, the value was hardcoded at 6.
// Having the default at 6 then make no change to existing use cases.
enum { kDefaultCullingJobCount = 6 };

bool GetScriptableCullingParameters(Camera& camera, bool stereoAware, ScriptableCullingParameters& cullingParameters)
{
    // Please Note:
    // Some camera settings are shared between culling and rendering. Check SetupShadowCullData for instance.
    // That only becomes a problem when the rendering camera is different from culling camera
    // e.g, when occlusion visualization window is enabled. The best approach is to make a clear distinction
    // of render and culling settings in the future.
    // Meanwhile we recalculate culling parameters and rebuild SetupShadowCullData (check EditorCameraDrawing::RenderEditorCamera)
    // when occlusion visualization is enabled.
    if (!camera.IsValidToRender())
        return false;

    cullingParameters.accurateOcclusionThreshold = -1.f;
    cullingParameters.maximumPortalCullingJobs = kDefaultCullingJobCount;
    cullingParameters.maximumVisibleLights = -1;

    Camera::MatrixState matrixState;
    float stereoSeparation = 0.0f;
    // VZ: Do not consider VR now.
//    bool useStereoParameters = stereoAware && camera.GetStereoSingleCullEnabled();
//    if (useStereoParameters)
//    {
//        camera.SaveMatrixState(matrixState);
//        IVRDevice* vrDevice = GetIVRDevice();
//
//        //@TODO: Hololens is the only current client of PreCull, and uses
//        // the RenderManager currentCamera, which is not used with SRP,
//        // and invalid in other scenarios.  As a WAR, we could wrap
//        // PreCull by setting the current camera temporarily.
//        // Fogbugz 924686
//        vrDevice->PreCull(&camera, useStereoParameters);
//
//        Matrix4x4f stereoView, stereoProj;
//        //@TODO: Consider temporarily setting CameraType to kCameraTypeVR
//        // in order to ensure correct stereo paths are taken by Camera.
//        vrDevice->GetCullingParameters(&camera, stereoView, stereoProj, stereoSeparation);
//        camera.SetWorldToCameraMatrix(stereoView);
//        camera.SetProjectionMatrix(stereoProj);
//    }

    camera.CalculateCullingParameters(cullingParameters);
    cullingParameters.shadowDistance = camera.GetFar();
    cullingParameters.reflectionProbeSortOptions = kProbeSortNone;
    cullingParameters.cullingOptions = SetFlags(cullingParameters.cullingOptions, kCullFlagNeedsLighting | kCullFlagNeedsReflectionProbes);

    ScriptableCullingParameters::CameraProperties& cameraProperties = cullingParameters.cameraProperties;
    GetCoreCameraValues(camera, cameraProperties.coreCameraValues);
    cameraProperties.coreCameraValues.cullingMask = camera.GetCullingMask();

//    if (useStereoParameters)
//    {
//        cullingParameters.cullingOptions = SetFlags(cullingParameters.cullingOptions, kCullFlagStereo);
//        cullingParameters.cullStereoSeparation = stereoSeparation;
//        cullingParameters.cullStereoView = camera.GetWorldToCameraMatrix();
//        cullingParameters.cullStereoProj = camera.GetProjectionMatrix();
//
//        camera.RestoreMatrixState(matrixState);
//    }

    cameraProperties.screenRect = camera.GetScreenViewportRect();
    cameraProperties.viewDir = -NormalizeSafe(camera.GetCameraToWorldMatrix().GetAxisZ());
    cameraProperties.projectionNear = camera.GetProjectionNear();
    cameraProperties.projectionFar = camera.GetProjectionFar();
    cameraProperties.cameraNear = camera.GetNear();
    cameraProperties.cameraFar = camera.GetFar();

    auto cameraAspect = camera.GetAspect();
//    if (camera.GetUsePhysicalProperties() && camera.GetGateFit() == Camera::kGateFitNone)
//    {
//        auto& sensorSize = camera.GetSensorSize();
//        cameraAspect = sensorSize.x / sensorSize.y;
//    }

    cameraProperties.cameraAspect = cameraAspect;

//    if (useStereoParameters)
//    {
//        cameraProperties.worldToCamera = cullingParameters.cullStereoView;
//        MultiplyMatrices4x4(&cullingParameters.cullStereoProj, &cullingParameters.cullStereoView, &cameraProperties.actualWorldToClip);
//    }
//    else
    {
        cameraProperties.worldToCamera = camera.GetWorldToCameraMatrix();
        cameraProperties.actualWorldToClip = camera.GetWorldToClipMatrix();
    }
    Matrix4x4f::Invert_Full(cameraProperties.worldToCamera, cameraProperties.cameraToWorld);
    Matrix4x4f::Invert_Full(cameraProperties.actualWorldToClip, cameraProperties.cameraClipToWorld);

    Transform* transform = camera.QueryComponent<Transform>();
    cameraProperties.up = transform->InverseTransformDirection(Vector3f(0.0f, 1.0f, 0.0f));
    cameraProperties.up.z = 0;
    cameraProperties.up = transform->TransformDirection(cameraProperties.up);
    cameraProperties.up = NormalizeSafe(cameraProperties.up);

    cameraProperties.transformDirection = transform->TransformDirection(Vector3f::zAxis);

    cameraProperties.right = Cross(cameraProperties.up, cameraProperties.transformDirection);
    cameraProperties.right = NormalizeSafe(cameraProperties.right);


    cameraProperties.cameraEuler = QuaternionToEuler(transform->GetRotation());

    cameraProperties.farPlaneWorldSpaceLength = camera.CalculateFarPlaneWorldSpaceLength();

    cameraProperties.velocity = camera.GetVelocity();

    float farNearRatio = cameraProperties.projectionFar / cameraProperties.projectionNear;
    if (!CanUseProjectionForShadows(cameraProperties.cameraClipToWorld, farNearRatio, cullingParameters.position))
    {
        // Use implicit projection instead (which we used always before 3.5)
        Matrix4x4f proj;
        camera.GetImplicitProjectionMatrix(cameraProperties.cameraNear, cameraProperties.cameraFar,
                                           cullingParameters.lodParams.fieldOfView, cameraProperties.cameraAspect, proj);
        MultiplyMatrices4x4(&proj, &cameraProperties.worldToCamera, &cameraProperties.cameraWorldToClip);
        Matrix4x4f::Invert_Full(cameraProperties.cameraWorldToClip, cameraProperties.cameraClipToWorld);
    }
    else
    {
        cameraProperties.cameraWorldToClip = cameraProperties.actualWorldToClip;
    }

    camera.GetImplicitProjectionMatrix(cameraProperties.projectionNear, cameraProperties.projectionFar,
                                       cullingParameters.lodParams.fieldOfView, cameraProperties.cameraAspect, cameraProperties.implicitProjection);

    cameraProperties.projectionIsOblique = CameraScripting::IsObliqueProjection(camera.GetProjectionMatrix()) ? 1 : 0;
    cameraProperties.isImplicitProjectionMatrix = camera.IsImplicitProjectionMatrix();

    float shadowDistance = cullingParameters.shadowDistance;
    //Check if standard render pipeline is used:
    bool usingSRP = true; // !GetGraphicsSettings().GetCurrentRenderPipeline().IsNull();
#if UNITY_EDITOR
    usingSRP &= GetRenderManager().GetUseScriptableRenderPipeline();
#endif
//    if (!usingSRP)
//    {
//        float shadowDistFromQualSettings = QualitySettings::GetShadowDistanceForRendering();
//        shadowDistance = std::min(shadowDistFromQualSettings, cullingParameters.shadowDistance);
//
//        bool shadowsEnabled = (GetQualitySettings().GetCurrent().shadows != kQSShadowsDisable);
//
//        if (shadowsEnabled)
//            cullingParameters.cullingOptions = SetFlags(cullingParameters.cullingOptions, kCullFlagShadowCasters);
//    }
//    else
    {
        // ShadowCasters is set by default
        cullingParameters.cullingOptions = SetFlags(cullingParameters.cullingOptions, kCullFlagShadowCasters);
    }

    camera.CalculateFrustumPlanes(cameraProperties.shadowCullPlanes, cameraProperties.cameraWorldToClip,
                                  shadowDistance, cameraProperties.baseFarDistance, true);
    for (int i = 0; i < kPlaneFrustumNum; ++i)
        cameraProperties.cameraCullPlanes[i] = cameraProperties.shadowCullPlanes[i];

    const float* layerCullDistances = camera.GetLayerCullDistances();
    std::copy(layerCullDistances, layerCullDistances + kNumLayers, cameraProperties.layerCullDistances);
    cameraProperties.layerCullSpherical = camera.GetLayerCullSpherical();

//    //check for stereo? (sceneCullParameters.stereo always set to false in CullScriptable function)
//    VRDeviceRenderPassHelper renderPassHelper;
//    for (int eye = 0; eye < kStereoscopicEyeCount; ++eye)
//    {
//        renderPassHelper.SetRenderPass(eye);
//        cameraProperties.stereoWorldToClip[eye] = camera.GetStereoWorldToClipMatrix(StereoscopicEye(eye));
//    }

    cameraProperties.cameraType = camera.GetCameraType();

    return true;
}
