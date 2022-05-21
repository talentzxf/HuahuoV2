//
// Created by VincentZhang on 5/17/2022.
//

#include "ScriptableDrawRenderers.h"
#include "Memory/MemoryMacros.h"
#include "SceneManager/SceneManager.h"
#include "Shaders/ShaderPassContext.h"
#include "GfxDevice/GfxDevice.h"
#include "Job/BlockRangeJob.h"
#include "Job/JobTypes.h"
#include "Camera/RenderNodeQueue.h"
#include "Camera/SharedRendererScene.h"
#include "Math/Simd/vec-math.h"
#include "Camera/SharedRenderData.h"

struct ScriptableRenderContextArg : public GfxDeviceAsyncCommand::Arg
{
    ScriptableRenderContextArg(MemLabelId memLabel)
    : GfxDeviceAsyncCommand::Arg(memLabel)
//            , objectDatas(memLabel)
            , sharedRendererScene(NULL)
            , isPassTagName(false)
            , overrideMaterial(NULL)
            , stateTagValues(nullptr)
            , stateBlocks(nullptr)
    {
    }

    ~ScriptableRenderContextArg()
    {
//        sharedRendererScene->Release();
//        if (overrideMaterial && overrideMaterial->sharedMaterialData)
//            overrideMaterial->sharedMaterialData->Release();
        HUAHUO_DELETE(overrideMaterial, kMemTempJobAlloc);
//        UNITY_FREE(kMemTempJobAlloc, stateTagValues);
//        UNITY_FREE(kMemTempJobAlloc, stateBlocks);
        ClearFenceWithoutSync(prepareFence);
    }

    JobFence                    prepareFence;

    ScriptableLoopObjectDatas   objectDatas;
    DrawRenderersCommand        command;
    const SharedRendererScene*  sharedRendererScene;

    BlockRange                  blockRanges[kMaximumBlockRangeCount];
    UInt32                      renderJobCount;
    SInt32                      motionVectorFrameIndex;
    ShaderTagID                 tagName;
    bool                        isPassTagName;

    OverrideMaterialInfo*       overrideMaterial;
    ShaderTagID*                stateTagValues;
    RenderStateBlock*           stateBlocks;
};

struct ScriptableRenderLoopScratch : public GfxDeviceAsyncCommand::ArgScratch
{
    ShaderPassContext           passContext;
    UInt32                      index;
    Vector4f                    lightmapDecodeValues;
    Vector4f                    realtimeLightmapDecodeValues;

    ScriptableRenderLoopScratch(MemLabelId label)
            : GfxDeviceAsyncCommand::ArgScratch(label)
//            , passContext(label)
    {}

    virtual void ThreadedCleanup()
    {
    }
};

void CleanupDrawRenderersCommand(DrawRenderersCommand& cmd)
{
    HUAHUO_FREE(kMemTempJobAlloc, cmd.stateTagValues);
    HUAHUO_FREE(kMemTempJobAlloc, cmd.stateBlocks);
}

bool NodeHasMotion(const RenderNode& node, SInt32 motionVectorFrameIndex)
{
    return node.rendererData.m_HasLastPositionStream ||
           node.rendererData.m_MotionVectors == kMotionVectorForceNoMotion ||
           node.rendererData.m_MotionVectors == kMotionVectorHybridBatch ||
           //Expensive bit here only gets evaluated if m_MotionVectors == kMotionVectorObject
           (node.rendererData.m_TransformInfo.motionVectorFrameIndex == motionVectorFrameIndex &&
            !CompareApproximately(node.rendererData.m_TransformInfo.worldMatrix, node.rendererData.m_TransformInfo.prevWorldMatrix));  // Did it move this frame and was it enough for us to care?
}


void PrepareScriptableLoopObjectData(
        const RenderNodeQueue& queue,
        const DrawRenderersCommand& cmd,
        const OverrideMaterialInfo* overrideMaterial,
        SInt32 motionVectorFrameIndex,
        size_t begin,
        size_t end,
        ScriptableLoopObjectDatas& objectDatas)
{
//    const Matrix4x4f worldToCameraMatrix = cmd.drawSettings.sorting.worldToCameraMatrix;
//    const RendererDistanceMetric distanceMetric = cmd.drawSettings.sorting.distanceMetric;
//    const Vector3f cameraPos = cmd.drawSettings.sorting.cameraPosition;
//    const Vector3f cameraCustomAxisSort = cmd.drawSettings.sorting.cameraCustomSortAxis;
//    const bool configurePerObjectMotionVectors = HasFlag(cmd.drawSettings.rendererConfiguration, kConfigurePerObjectMotionVectors);
//    const bool excludeMotionVectorObjects = cmd.filterSettings.excludeMotionVectorObjects != 0;
//    ShaderTagID motionPassName = ShaderTagID("MotionVectors");
//    const bool useSortingGroup = HasFlag(cmd.drawSettings.sorting.criteria, kSortBackToFront);
//
//    // We need to change the range. Unfortunately in some places the sorting layer range is signed (-32768, 32767) and in other (0-65535).
//    // Here we are adding 32768 (0x8000) to the upper and lower bound to convert from signed to unsigned
//    const UInt16 minSortingLayerFilter = cmd.filterSettings.sortingLayerRange.lowerBound + 0x8000;
//    const UInt16 maxSortingLayerFilter = cmd.filterSettings.sortingLayerRange.upperBound + 0x8000;
//
//    SortingGroupDataArray sortingGroupDataArray(kMemTempAlloc);
//    if (useSortingGroup)
//        GetSortingGroupManager().CopyTo(sortingGroupDataArray);
//
//    // figure out what the requested passes
//    // for this draw are. compress the list
//    // so we are not handling SRP_MAX_SHADER_PASSES_PER_DRAW
//    // in the inner loop
//
//    ShaderTagID validatedPasses[SRP_MAX_SHADER_PASSES_PER_DRAW];
//    int numValidatedPasses = 0;
//    bool motionVectorPassRequested = false;
//    for (int k = 0; k < SRP_MAX_SHADER_PASSES_PER_DRAW; ++k)
//    {
//        // do not use IsValid() here.
//        // we use value 0 for 'no lightmode specified'
//        if (cmd.drawSettings.shaderPassNames[k].id >= 0)
//        {
//            ShaderTagID passName = cmd.drawSettings.shaderPassNames[k];
//            validatedPasses[numValidatedPasses++] = passName;
//            motionVectorPassRequested |= (passName == motionPassName);
//        }
//    }
//
//    // go over visible render nodes and see which ones satisfy our command filter
//    for (size_t i = begin; i != end; ++i)
//    {
//        const RenderNode& node = queue.GetNode(i);
//
//        if (node.rendererData.m_CastShadows == kShadowCastingShadowsOnly)
//            continue;
//
//        // Filter out based on input culling options
//        if ((cmd.filterSettings.layerMask & (1 << node.layer)) == 0)
//            continue;
//
//        if ((node.rendererData.m_RenderingLayerMask & cmd.filterSettings.renderingLayerMask) == 0)
//            continue;
//
//        if (motionVectorPassRequested && configurePerObjectMotionVectors && node.rendererData.m_MotionVectors == kMotionVectorCamera)
//            continue;
//
//        Vector3f center = node.rendererData.m_TransformInfo.worldAABB.GetCenter();
//
//        UInt32 layerAndOrder = node.rendererData.m_GlobalLayeringData.layerAndOrder;
//        float sortingFudge = node.sortingFudge;
//        if (useSortingGroup
//            && node.rendererData.m_GlobalLayeringData.sortingGroup.id != GlobalLayeringData::kInvalidSortingGroupID
//            && node.rendererData.m_GlobalLayeringData.sortingGroup.id < sortingGroupDataArray.size())
//        {
//            const SortingGroupData& sortingGroupData = sortingGroupDataArray[node.rendererData.m_GlobalLayeringData.sortingGroup.id];
//            center = sortingGroupData.aabb.GetCenter();
//            layerAndOrder = sortingGroupData.layerAndOrder;
//            sortingFudge = 0.0f;
//        }
//
//        // Filter out based on sorting layer range.
//        UInt32 nodeLayer = (layerAndOrder >> 16);
//        if (nodeLayer < minSortingLayerFilter || nodeLayer > maxSortingLayerFilter)
//            continue;
//
//        const float distance = worldToCameraMatrix.MultiplyPoint3(center).z;
//        const float distanceAlongView = -distance;
//        float distanceForSort;
//        switch (distanceMetric)
//        {
//            case kDrawRendererSortModeOrthographic:
//            {
//                distanceForSort = distance - sortingFudge;
//            }
//                break;
//            case kDrawRendererSortModeCustomAxis:
//            {
//                distanceForSort = -Dot(cameraCustomAxisSort, center) - sortingFudge;
//            }
//                break;
//            case kDrawRendererSortModePerspective:
//            default:
//            {
//                // The more intuitive and simple version of the code is this one :
//                // distanceForSort = -(Magnitude(center - cameraPos) + sortingFudge);
//                // But it's possible to save a square root computation when sortingFudge = 0
//                // Which is why the actual code is doing a bit of gymnastic
//
//                distanceForSort = SqrMagnitude(center - cameraPos);
//                if (sortingFudge != 0.0f)
//                {
//                    distanceForSort = Sqrt(distanceForSort) + sortingFudge;
//
//                    // Must square the result back to be coherent with the other 'distanceForSort' values.
//                    // The 'distanceForSort' values calculated for objects with a 'sortingFudge' equal to 0 are squared.
//                    // The sign must also be preserved.
//                    distanceForSort = Sqr(distanceForSort) * Sign(distanceForSort);
//                }
//                distanceForSort = -distanceForSort;
//            }
//                break;
//        }
//
//        // Is this node tagged as camera motion only?
//        bool nodeIsCameraMotionOnly = node.rendererData.m_MotionVectors == kMotionVectorCamera;
//
//        //A node is skipped under these conditions:
//        //1. The node has moved and its Motion Pass is DISABLED
//        //2. The node has a Motion Pass that is ENABLED
//        //3. In the case of an object having multiple submeshes, only those with disabled motion passes will be skipped
//        //EXCEPTION:
//        //A node is not skipped if it is tagged as Camera Motion Only because it will be skipped in the Motion Vector Pass
//        bool nodeHasMotion = NodeHasMotion(node, motionVectorFrameIndex);
//
//        bool motionVecForceRender = false;
//        if (configurePerObjectMotionVectors)
//        {
//            motionVecForceRender = NodeHasMotion(node, motionVectorFrameIndex);
//        }
//
//        // A distance of NaN can cause inconsistent sorting results, if input order is inconsistent.
//        Assert(IsFinite(distanceForSort));
//        Assert(IsFinite(distanceAlongView));
//
//        MaterialInfo* materialInfos = (MaterialInfo*)node.materialInfos;
//        for (int mi = 0; mi < node.materialCount; ++mi)
//        {
//            const MaterialInfo* materialInstance = &materialInfos[mi];
//
//            Shader* shader = const_cast<Shader*>(materialInstance->sharedMaterialData->shader);
//            int queueIndex = GetActualRenderQueue(*materialInstance, shader);
//            if (queueIndex < cmd.filterSettings.renderQueueMin || queueIndex > cmd.filterSettings.renderQueueMax)
//                continue;
//
//            // Go over passes in the shader and see if we have one we need
//            const ShaderLab::IntShader* intShader = shader->GetShaderLabShader();
//            const ShaderLab::SubShader* subshader = &intShader->GetActiveSubShader();
//
//            // filter the passes to the ones that match
//            // the input params
//            int foundPasses[SRP_MAX_SHADER_PASSES_PER_DRAW];
//            int numFoundPasses = 0;
//
//            const SharedMaterialData& materialData = *materialInstance->sharedMaterialData;
//
//            // Does this sub-shader have a motion vector pass defined?
//            bool hasMotionVectorPass = subshader->LightModeToPassIndex(motionPassName) >= 0;
//
//            // This node has a motion vector pass and we should exclude sub-meshes that have motion vector, we then skip it
//            // When we want to exclude motion vector objects, we need to also exclude material that enable the motion vector pass directly in the material
//            if (!nodeIsCameraMotionOnly && hasMotionVectorPass && excludeMotionVectorObjects &&
//                (nodeHasMotion || IsMaterialShaderPassEnabled(materialData, motionPassName)))
//                continue;
//
//            for (int k = 0; k < numValidatedPasses; ++k)
//            {
//                ShaderTagID shaderPassName = validatedPasses[k];
//
//                if (IsMaterialShaderPassEnabled(materialData, shaderPassName) || (motionVecForceRender && shaderPassName == motionPassName))
//                {
//                    int passIndex = subshader->LightModeToPassIndex(shaderPassName);
//                    if (passIndex >= 0)
//                    {
//                        foundPasses[numFoundPasses++] = passIndex;
//                    }
//                }
//            }
//
//            // no passes found in active subshader...
//            // go to next object
//            if (numFoundPasses == 0)
//                continue;
//
//            // if we have an override material
//            // we need to remap the draw calls here!
//            if (overrideMaterial)
//            {
//                materialInstance = overrideMaterial;
//                shader = const_cast<Shader*>(materialInstance->sharedMaterialData->shader);
//                intShader = shader->GetShaderLabShader();
//                subshader = &intShader->GetActiveSubShader();
//
//                numFoundPasses = 0;
//                int passIndexToUse = math::clamp(overrideMaterial->passIndex, 0, std::numeric_limits<int>::max());
//
//                if (passIndexToUse < subshader->GetTotalPassCount())
//                    foundPasses[numFoundPasses++] = passIndexToUse;
//            }
//
//            // no passes found in active subshader...
//            // go to next object
//            if (numFoundPasses == 0)
//                continue;
//
//            AssertMsg(materialInstance, "Material Info is null in PrepareScriptableLoopObjectData");
//
//            ScriptableLoopObjectData odata;
//            odata.data.shader = shader;
//            if (cmd.drawSettings.useSRPBatcher)
//                odata.data.materialSortKey = GetMaterialSortKeySRPBatcher(materialInstance->sharedMaterialData->smallMaterialIndex, shader->GetInstanceID(), materialInstance->sharedMaterialData->keywordSetHash);
//            else
//                odata.data.materialSortKey = GetMaterialSortKey(materialInstance->sharedMaterialData->smallMaterialIndex, shader->GetInstanceID(), materialInstance->sharedMaterialData->shaderKeywordSet, materialInstance->sharedMaterialData->enableInstancing ? 0 : node.GetCustomPropsFromMaterialIndex(mi).hash);
//            odata.data.nodeIndex = i;
//            odata.data.queueIndex = queueIndex;
//            odata.data.rendererPriority = node.rendererData.m_RendererPriority;
//            odata.data.subsetIndex = node.rendererData.m_StaticBatchInfo.firstSubMesh + mi;
//            odata.data.subShaderIndex = intShader->GetActiveSubShaderIndex();
//            odata.data.sourceMaterialIndex = (UInt16)mi;
//            const bool SRPBatcherCompatible = cmd.drawSettings.useSRPBatcher ? IsSRPBatcherCompatible(node, *shader, intShader->GetActiveSubShaderIndex(), odata.data.subsetIndex, materialInstance->sharedMaterialData) : false;
//            odata.data.distance = distanceForSort;
//            odata.data.globalLayeringData = node.rendererData.m_GlobalLayeringData;
//            odata.data.globalLayeringData.layerAndOrder = layerAndOrder; // layerAndOrder may have been overridden through Sorting Group
//            odata.data.distanceAlongView = distanceAlongView;
//            // TODO: generate better sort data here to use in efficient sorting
//            // 32-bit geometry sort key, used by instancing:
//            // |       16 bits        |   16 bits  |
//            // | mesh ID (lowest 16b) | submesh ID |
//            odata.data.geometrySortKey = ((UInt32)(node.smallMeshIndex & 0xFFFF) << 16) | odata.data.subsetIndex;
//            odata.sharedMaterial = materialInstance->sharedMaterialData;
//
//            for (int j = 0; j < numFoundPasses; ++j)
//            {
//                odata.pass = subshader->GetPass(foundPasses[j]);
//                odata.data.srpBatcherCompatible = SRPBatcherCompatible;
//                odata.passIndex = foundPasses[j];
//                odata.passOrder = j;
//                objectDatas.push_back(odata);
//            }
//        }
//    }
}

static void PrepareScriptableDrawRenderersJob(ScriptableRenderContextArg* data)
{
    const RenderNodeQueue& queue = data->sharedRendererScene->queue;

    PrepareScriptableLoopObjectData(queue, data->command, data->overrideMaterial, data->motionVectorFrameIndex, 0, queue.GetRenderNodesCount(), data->objectDatas);

    if (data->objectDatas.size() == 0)
    {
        data->renderJobCount = 0;
        return;
    }

    // SortScriptableLoopObjectData(queue, data->command.drawSettings.sorting.criteria, data->objectDatas);

    data->renderJobCount = ConfigureBlockRanges(data->blockRanges, data->objectDatas.size(), data->renderJobCount);
}

ScriptableRenderContextArg* PrepareDrawRenderersCommand(const DrawRenderersCommand& cmd, JobBatchDispatcher& dispatcher)
{
//    if (cmd.cullResults == NULL)
//    {
//        ErrorString("A valid CullResults must be assigned to DrawRenderers");
//        return NULL;
//    }
//
//    const SharedRendererScene& rendererScene = *cmd.cullResults->cullResults.sharedRendererScene;
//
//    const RenderNodeQueue& renderNodes = rendererScene.queue;
//    UInt32 nodeCount = renderNodes.GetRenderNodesCount();
//    if (nodeCount == 0)
//    {
//        return NULL;
//    }
//
//    UInt32 renderJobCount = CalculateRenderJobCount(nodeCount);
//
    ScriptableRenderContextArg* arg = HUAHUO_NEW(ScriptableRenderContextArg, kMemTempJobAlloc)(kMemTempJobAlloc);
//    arg->command = cmd;
//    PPtr<Material> material =  PPtr<Material>(cmd.drawSettings.overrideMaterialInstanceId);
//    if (material.IsValid())
//    {
//        arg->overrideMaterial = UNITY_NEW(OverrideMaterialInfo, kMemTempJobAlloc);
//        arg->overrideMaterial->sharedMaterialData = material->AcquireSharedMaterialData();
//        arg->overrideMaterial->customRenderQueue = material->GetCustomRenderQueue();
//        arg->overrideMaterial->passIndex = cmd.drawSettings.overrideMaterialPassIdx;
//    }
//    rendererScene.AddRef();
//    arg->sharedRendererScene = &rendererScene;
//    arg->renderJobCount = renderJobCount;
//    arg->objectDatas.reserve(rendererScene.queue.GetRenderNodesCount() * 2);
//    // Transfer ownership of stateTagValues and stateBlocks to ScriptableRenderContextArg
//    arg->tagName = cmd.tagName;
//    arg->isPassTagName = cmd.isPassTagName;
//    arg->stateTagValues = cmd.stateTagValues;
//    arg->stateBlocks = cmd.stateBlocks;
//    cmd.stateTagValues = nullptr;
//    cmd.stateBlocks = nullptr;
//    arg->command.drawSettings.useSRPBatcher = ScriptableBatchRenderer::IsActive();
//    arg->motionVectorFrameIndex = GetRendererUpdateManager().GetMotionVectorFrameIndex();
//
    dispatcher.ScheduleJob(arg->prepareFence, PrepareScriptableDrawRenderersJob, arg);

    return arg;
}

void ScriptableRenderLoopDrawDispatch(const ScriptableLoopObjectData* objectDatas, size_t count, const SharedRendererScene& scene, const DrawRenderersCommand& cmd, ShaderPassContext& passContext, GfxDevice& device, SInt32 motioneVectorFrameIndex)
{
//    if ((cmd.drawSettings.useSRPBatcher) && (IsRendererConfigurationSRPBatcherCompatible(cmd.drawSettings.rendererConfiguration)))   // if any builtin feature is not supported by fast code path, use the normal code path
//    {
//        const ScriptableLoopObjectData* it = objectDatas;
//        const ScriptableLoopObjectData* end = it + count;
//        while (it < end)
//        {
//            bool IsCompatible;
//            int count = GetConsecutiveSRPBatcherCompatibleNodes(cmd, it, end, IsCompatible);
//            if (IsCompatible)
//                ScriptableRenderLoopDrawSRPBatcher(it, count, scene, cmd, passContext, device, motioneVectorFrameIndex);
//            else
//            {
//                FRAME_DEBUGGER_SET_NEXT_BATCH_BREAK_CAUSE(kSRPBatchNotCompatibleNode);
//                ScriptableRenderLoopDraw(it, count, scene, cmd, passContext, device, motioneVectorFrameIndex);
//            }
//            it += count;
//        }
//    }
//    else
//    {
//        ScriptableRenderLoopDraw(objectDatas, count, scene, cmd, passContext, device, motioneVectorFrameIndex);
//    }
}

static void ScriptableRenderLoopJob(GfxDeviceAsyncCommand::ArgScratch* i_scratch, const GfxDeviceAsyncCommand::Arg* i_arg)
{
    ScriptableRenderLoopScratch& scratch = *static_cast<ScriptableRenderLoopScratch*>(i_scratch);
    const ScriptableRenderContextArg& args = *static_cast<const ScriptableRenderContextArg*>(i_arg);

    // Since we schedule jobs before we know the exact amount of shadow casters, we might have scheduled too many jobs.
    // Simply ignore and don't render in those at all...
    if (scratch.index >= args.renderJobCount)
        return;

    BlockRange block = args.blockRanges[scratch.index];

    ShaderPassContext& passContext = scratch.passContext;
    GfxDevice& device = *scratch.device;

    Vector4f lightmapDecodeValues = scratch.lightmapDecodeValues;
    Vector4f realtimeLightmapDecodeValues = scratch.realtimeLightmapDecodeValues;
    // SetupLightmaps(lightmapDecodeValues, realtimeLightmapDecodeValues, passContext);

    // ScriptableRenderLoopDrawDispatch(args.objectDatas.begin() + block.startIndex, block.rangeSize, *args.sharedRendererScene, args.command, passContext, device, args.motionVectorFrameIndex);
}

static void ScheduleRenderJobs(UInt32 jobCount, const ScriptableRenderContextArg* renderArgs, const JobFence& dependency)
{
//    PROFILER_AUTO(gRenderLoopScheduleDraw);
//    GPU_AUTO_SECTION(kGPUSectionOther);

    GfxDevice& device = GetGfxDevice();
    ShaderPassContext& defaultPassContext = GetDefaultPassContext();

    // Extract pass contexts int per job scratch data
    ScriptableRenderLoopScratch* splitHeaders[kMaximumBlockRangeCount];
//    const LightmapSettings& lightmapSettings = GetLightmapSettings();
//    Vector4f lightmapDecodeValues = lightmapSettings.GetLightmapDecodeValues();
//    Vector4f realtimeLightmapDecodeValues = lightmapSettings.GetRealtimeLightmapDecodeValues();
//    for (UInt32 i = 0; i != jobCount; i++)
//    {
//        ScriptableRenderLoopScratch* scratch = splitHeaders[i] = UNITY_NEW(ScriptableRenderLoopScratch, kMemTempJobAlloc)(kMemTempJobAlloc);
//        scratch->passContext.CopyFrom(defaultPassContext);
//        scratch->passContext.properties.SetAllowAddProperty(false); // Don't allow adding new global properties
//        scratch->index = i;
//        scratch->lightmapDecodeValues = lightmapDecodeValues;
//        scratch->realtimeLightmapDecodeValues = realtimeLightmapDecodeValues;
//    }

    // Schedule rendering jobs
    device.ExecuteAsync(jobCount, ScriptableRenderLoopJob, reinterpret_cast<GfxDeviceAsyncCommand::ArgScratch**>(splitHeaders), renderArgs, dependency);

//    GPU_TIMESTAMP();
//
//    // cleanup the renderloop job header.
//    for (UInt32 i = 0; i != jobCount; i++)
//        splitHeaders[i]->Release();
}

void ExecuteDrawRenderersCommand(ScriptableRenderContextArg* args)
{
    // PROFILER_AUTO(gRenderLoopScheduleDraw);

    if (args == NULL)
        return;

    UInt32 renderJobCount = args->renderJobCount;
    if (renderJobCount != 0)
        ScheduleRenderJobs(renderJobCount, args, args->prepareFence);
#if ENABLE_UNITY_COLLECTIONS_CHECKS
    // The prepare job might set renderJobCount to zero if there
    // is nothing to render after filtering the nodes,
    // the PrepareJob depending on timing of jobs (indeterministic) might complete
    // before ExecuteDrawRenderersCommand is called on the main thread.
    // Thus nothing will have cleared the "args->prepareFence" in the JobsDebugger.
    // In that case we SyncFence in order to avoid leaks in the job debugger.
    else
        SyncFence(args->prepareFence);
#endif

    // VZ: This is inherited from ThreadSharedObject, Since we don't have that, ignore for now.
    // args->Release();
}