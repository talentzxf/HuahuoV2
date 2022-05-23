//
// Created by VincentZhang on 5/20/2022.
//

#ifndef HUAHUOENGINE_CULLRESULTS_H
#define HUAHUOENGINE_CULLRESULTS_H

#include "CullingParameters.h"
#include "Job/JobTypes.h"
#include <vector>
#include "Geometry/AABB.h"
#include "Utilities/NonCopyable.h"

struct CullResults : public NonCopyable
{
    CullResults();
    ~CullResults();

//    void Init(const Umbra::Tome* tome);
//    void InitDynamic(const RendererCullData* rendererCullData, int totalRendererListsCount);

    JobFence                occlusionBufferIsReady;
    JobFence                sceneCullingOutputIsReady;
    JobFence                addLocalLightsFence;
    JobFence                cullLocalLightsFence;
    CullingOutput           sceneCullingOutput;

    UInt32                  hybridLightsCount;
//    HybridLightPlanes*      hybridLights;

    JobFence                nodesHaveBeenPrepared;
//    BaseRendererArray       needsCullCallback;
//    BaseRendererArray       rendererCullCallbacks[kRendererTypeCount];

    // All lights that might affect any visible objects
    //@TODO: Ownership in SharedRendererScene. No need to copy...
//    ActiveLights           activeLights;
//    Threads<UInt8>   isShadowCastingLight;
//
//    // All lights that cast shadows on any objects in the scene
//    ShadowedLights         shadowedLights;

    // All reflection probes that might affect visible objects
    JobFence               addReflectionProbesFence;
    JobFence               cullReflectionProbeFence;
//    ActiveReflectionProbes activeReflectionProbes;

    SceneCullingParameters sceneCullParameters;

    std::vector<LODDataArray> lodDataArrays;

//    TerrainCullData* terrainCullData;

    std::vector<SceneNode> customCullSceneNodes;
    std::vector<AABB> customCullBoundingBoxes;
//    std::vector<CustomCullResult*> sceneCustomCullResults;

//    ShadowCullData* shadowCullData;

    JobFence generateCombinedDynamicListReady;
    std::vector<int> visibleSceneIndexListCombined;
    std::vector<Vector3f> dynamicBounds;

//    ShaderReplaceData shaderReplaceData;

    // light reindexing
    std::vector<SInt32> lightIndexMap;

    // reflection probe reindexing
    std::vector<SInt32> reflectionProbeIndexMap;

    /// Whether the instance have been fully initialized with results from a culling process.
    /// This is mostly a gross hack to work around the fact that because the camera's culling
    /// and rendering code is scattered all over the place and called with the same patterns
    /// from many places, we don't have one place where we can prevent recursive rendering on
    /// cameras from one single point.
    bool isValid;

//    SharedRendererScene* sharedRendererScene;
//
//    const SharedRendererScene* GetOrCreateSharedRendererScene();
    void DestroySharedRendererScene();
    void SyncJobFence();
};

void SetCullingPlanes(CullingParameters& parameters, const Plane* planes, int planeCount);
void CalculateCustomCullingParameters(CullingParameters& cullingParameters, const LODParameters& lodParams, UInt32 cullingMask, UInt64 sceneCullingMask, const Plane* planes, int planeCount);

#endif //HUAHUOENGINE_CULLRESULTS_H
