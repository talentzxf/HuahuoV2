//
// Created by VincentZhang on 5/20/2022.
//

#include "CullResults.h"

void SetCullingPlanes(CullingParameters& parameters, const Plane* planes, int planeCount)
{
    parameters.cullingPlaneCount = planeCount;

    for (int i = 0; i < planeCount; i++)
        parameters.cullingPlanes[i] = planes[i];
}

void CalculateCustomCullingParameters(CullingParameters& cullingParameters, const LODParameters& lodParams, UInt32 cullingMask, UInt64 sceneCullingMask, const Plane* planes, int planeCount)
{
    cullingParameters.lodParams = lodParams;

    // Shadow code handles per-layer cull distances itself

    Assert(planeCount <= CullingParameters::kMaxPlanes);
    SetCullingPlanes(cullingParameters, planes, planeCount);
    cullingParameters.cullingPlaneCount = planeCount;
    cullingParameters.layerCull = CullingParameters::kLayerCullNone;
    cullingParameters.cullingMask = cullingMask;
    cullingParameters.sceneMask = sceneCullingMask;
}

CullResults::CullResults() //:
//        needsCullCallback(kMemTempJobAlloc),
//        isShadowCastingLight(kMemTempJobAlloc),
//        shadowedLights(kMemTempJobAlloc),
//        lodDataArrays(kMemTempJobAlloc),
//        terrainCullData(NULL),
//        customCullSceneNodes(kMemTempJobAlloc),
//        customCullBoundingBoxes(kMemTempJobAlloc),
//        sceneCustomCullResults(kMemTempJobAlloc),
//        shadowCullData(NULL),
//        visibleSceneIndexListCombined(kMemTempJobAlloc),
//        dynamicBounds(kMemTempJobAlloc),
//        lightIndexMap(kMemTempJobAlloc),
//        reflectionProbeIndexMap(kMemTempJobAlloc),
//        isValid(false),
//        hybridLights(NULL),
//        hybridLightsCount(0),
//        sharedRendererScene(NULL)
{
//    for (size_t i = 0; i < kRendererTypeCount; i++)
//        rendererCullCallbacks[i].set_memory_label(kMemTempJobAlloc);
}
