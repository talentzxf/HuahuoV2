//
// Created by VincentZhang on 5/17/2022.
//

#ifndef HUAHUOENGINE_SCRIPTABLEDRAWRENDERERS_H
#define HUAHUOENGINE_SCRIPTABLEDRAWRENDERERS_H
#include "Shaders/ShaderTags.h"
#include "Configuration/IntegerDefinitions.h"
#include "RenderLoopEnums.h"
#include "Math/Matrix4x4.h"
#include "GfxDevice/GfxDeviceAsyncCommand.h"
#include "Job/JobTypes.h"
#include "Job/Jobs.h"
#include "ScriptableDrawRenderersPrivate.h"
#include "Job/JobBatchDispatcher.h"

enum RenderStateMask
{
    kRenderStateNothing = 0,
    kRenderStateBlend = (1 << 0),
    kRenderStateRaster = (1 << 1),
    kRenderStateDepth = (1 << 2),
    kRenderStateStencil = (1 << 3)
};

struct SortingLayerRange
{
    SInt16 lowerBound;
    SInt16 upperBound;
};

// Must match RenderStateBlock on C# side
struct RenderStateBlock
{
//    GfxBlendState blendState;
//    GfxRasterState rasterState;
//    GfxDepthState depthState;
//    GfxStencilState stencilState;
    int stencilRef;
    RenderStateMask mask;
};

// match layout of FilterRenderersSettings on C# side
struct FilteringSettings
{
    SInt32 renderQueueMin;
    SInt32 renderQueueMax;
    SInt32 layerMask;
    UInt32 renderingLayerMask;
    SInt32 excludeMotionVectorObjects;
    SortingLayerRange sortingLayerRange;

    void InitDefault()
    {
        renderQueueMin = kQueueIndexMin;
        renderQueueMax = kQueueIndexMax;
        layerMask = ~0;
        renderingLayerMask = ~0;
        excludeMotionVectorObjects = 0;
        sortingLayerRange.lowerBound = INT16_MIN;
        sortingLayerRange.upperBound = INT16_MAX;
    }
};

enum RendererDistanceMetric
{
    kDrawRendererSortModePerspective,
    kDrawRendererSortModeOrthographic,
    kDrawRendererSortModeCustomAxis
};

enum RendererSortingCriteria
{
    kSortNone = 0,
    kSortSortingLayer = (1 << 0), // by global sorting layer
    kSortRenderQueue = (1 << 1), // by material render queue
    kSortBackToFront = (1 << 2), // distance back to front, sorting group order, same distance sort priority, material index on renderer
    kSortQuantizedFrontToBack = (1 << 3), // front to back by quantized distance
    kSortOptimizeStateChanges = (1 << 4), // combination of: static batching, lightmaps, material sort key, geometry ID
    kSortCanvasOrder = (1 << 5), // same distance sort priority (used in Canvas)
    kSortRendererPriority = (1 << 6), // by renderer priority (if render queues are not equal)

    kSortCommonOpaque = kSortSortingLayer | kSortRenderQueue | kSortQuantizedFrontToBack | kSortOptimizeStateChanges | kSortCanvasOrder,
    kSortCommonTransparent = kSortSortingLayer | kSortRenderQueue | kSortBackToFront | kSortOptimizeStateChanges,
};
ENUM_FLAGS(RendererSortingCriteria);

// match layout of DrawRendererSortSettings on C# side
struct RendererSortingSettings
{
    Matrix4x4f              worldToCameraMatrix;
    Vector3f                cameraPosition;
    Vector3f                cameraCustomSortAxis;
    RendererSortingCriteria criteria;
    RendererDistanceMetric  distanceMetric;

    // Hijack this struct to store previousVP & nonJitteredVP matrices as this is the only place in
    // the pipeline where you have access to the camera. The Foundation team is working on a refactor
    // to make this better.
    Matrix4x4f              previousVPMatrix;
    Matrix4x4f              nonJitteredVPMatrix;

    void InitDefault()
    {
        worldToCameraMatrix.SetIdentity();
        cameraPosition.SetZero();
        cameraCustomSortAxis = Vector3f(0.0f, 0.0f, 1.0f);
        criteria = kSortNone;
        distanceMetric = kDrawRendererSortModePerspective;
        previousVPMatrix.SetIdentity();
        nonJitteredVPMatrix.SetIdentity();
    }
};

enum DrawRendererFlags
{
    kDrawRendererNone = 0,
    kDrawRendererEnableDynamicBatching = (1 << 0),
    kDrawRendererEnableInstancing = (1 << 1),
};

enum PerObjectData
{
    kConfigureNothing = 0,
    kConfigurePerObjectLightProbe = (1 << 0),
    kConfigurePerObjectReflectionProbes = (1 << 1),
    kConfigurePerObjectLightProbeProxyVolume = (1 << 2),
    kConfigurePerObjectLightmaps = (1 << 3),
    kConfigurePerObjectLightData = (1 << 4),
    kConfigurePerObjectMotionVectors = (1 << 5),
    kConfigurePerObjectLightIndices = (1 << 6),
    kConfigurePerObjectReflectionProbeData = (1 << 7),
    kConfigurePerObjectOcclusionProbe = (1 << 8),
    kConfigurePerObjectOcclusionProbeProxyVolume = (1 << 9),
    kConfigurePerObjectShadowMask = (1 << 10),
};
ENUM_FLAGS(PerObjectData)

#define SRP_MAX_SHADER_PASSES_PER_DRAW 16
// match layout of DrawingSettings on C# side
struct DrawingSettings
{
    RendererSortingSettings    sorting;
    ShaderTagID                 shaderPassNames[SRP_MAX_SHADER_PASSES_PER_DRAW];
    PerObjectData       rendererConfiguration;
    DrawRendererFlags           flags;
    int                         overrideMaterialInstanceId;
    int                         overrideMaterialPassIdx;
    int                         mainLightIndex;
    bool                        useSRPBatcher;

    void InitDefault()
    {
        sorting.InitDefault();
        for (int i = 0; i < SRP_MAX_SHADER_PASSES_PER_DRAW; ++i)
            shaderPassNames[i] = ShaderTagID();
        rendererConfiguration = kConfigureNothing;
        flags = kDrawRendererNone;
        overrideMaterialInstanceId = 0;
        overrideMaterialPassIdx = -1;
        mainLightIndex = -1;
        useSRPBatcher = false;
    }
};

struct DrawRenderersCommand
{
    DrawingSettings drawSettings;
    // ScriptableCullResults *cullResults;
    FilteringSettings filterSettings;
    ShaderTagID tagName;
    bool isPassTagName;
    mutable ShaderTagID *stateTagValues;
    mutable RenderStateBlock *stateBlocks;
    int stateCount;

    void InitDefault()
    {
        drawSettings.InitDefault();
        // cullResults = nullptr;
        filterSettings.InitDefault();
        isPassTagName = false;
        stateTagValues = nullptr;
        stateBlocks = nullptr;
        stateCount = 0;
    }
};

void CleanupDrawRenderersCommand(DrawRenderersCommand& cmd);

struct ScriptableRenderContextArg;

ScriptableRenderContextArg* PrepareDrawRenderersCommand(const DrawRenderersCommand& cmd, JobBatchDispatcher& dispatcher);
void ExecuteDrawRenderersCommand(ScriptableRenderContextArg* args);

#endif //HUAHUOENGINE_SCRIPTABLEDRAWRENDERERS_H
