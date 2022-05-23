//
// Created by VincentZhang on 5/17/2022.
//

#include "Renderer.h"
#include "BaseClasses/GameObject.h"
#include "Camera/RendererScene.h"

Renderer::Renderer(RendererType type, MemLabelId label, ObjectCreationMode mode)
        :   Super(label, mode)
        ,   BaseRenderer(type)
//        ,   m_Materials(GetMemoryLabel())
        ,   m_SceneHandle(kInvalidSceneHandle)
//        ,   m_PerMaterialOverrideProperties(GetMemoryLabel())
//        ,   m_LastLightProbeTetIndex(-1)
//        ,   m_LODGroup(NULL)
        ,   m_Enabled(true)
        ,   m_IsRenderable(true)
//        ,   m_ForceRenderingOff(false)
//#if ENABLE_TEXTURE_STREAMING
//        ,   m_StreamingIndex(-1)
//#endif
//        ,   m_SortingLayerID(0)
//        ,   m_SortingOrder(0)
//        ,   m_SortingLayer(0)
#if UNITY_EDITOR
,   m_SelectedEditorRenderState(static_cast<EditorSelectedRenderState>(kEditorSelectedWireframe | kEditorSelectedHighlight))
    ,   m_ScaleInLightmap(1.0f)
    ,   m_ReceiveGI(kLightmaps)
    ,   m_PreserveUVs(false)
    ,   m_IgnoreNormalsForChartDetection(false)
    ,   m_MinimumChartSize(kDefaultMinimumChartSize)
    ,   m_ImportantGI(false)
    ,   m_StitchLightmapSeams(true)
    ,   m_AutoUVMaxDistance(0.5f)
    ,   m_AutoUVMaxAngle(89.0f)
#endif
{
//    SharedRendererData& srd = m_RendererData;
//    srd.m_LightProbeUsage = kLightProbeUsageBlendProbes;
//    srd.m_ReflectionProbeUsage = kReflectionProbeBlendProbes;
//    srd.m_MotionVectors = kMotionVectorObject;
//    srd.m_TransformInfo.lateLatchIndex = kNotLateLatched;
}

void Renderer::AwakeFromLoad(AwakeFromLoadMode awakeMode) {
    Super::AwakeFromLoad(awakeMode);

    UpdateRenderer();
}

void Renderer::UpdateRenderer()
{
    // Be careful of the implications on RendererScene.ApplyPendingAddRemoveNodes() when changing this behavior
    bool shouldBeInScene = ShouldBeInScene();
    if (IsInScene() == shouldBeInScene)
        return;

    if (shouldBeInScene)
        AddToScene();
    else
        RemoveFromScene();
}



template<class TransferFunction>
void Renderer::Transfer(TransferFunction& transfer)
{
    Super::Transfer(transfer);

    transfer.Transfer(m_Enabled, "m_Enabled", kHideInEditorMask);

//    SharedRendererData& rendererData = m_RendererData;
//    TRANSFER_BITFIELD(rendererData.m_CastShadows, "m_CastShadows");
//    TRANSFER_BITFIELD_FLAGS(rendererData.m_ReceiveShadows, "m_ReceiveShadows", kTreatIntegerValueAsBoolean);
//    TRANSFER_BITFIELD_FLAGS(rendererData.m_DynamicOccludee, "m_DynamicOccludee", kTreatIntegerValueAsBoolean);
//    TRANSFER_BITFIELD(rendererData.m_MotionVectors, "m_MotionVectors");
//    TRANSFER_BITFIELD(rendererData.m_LightProbeUsage, "m_LightProbeUsage");
//    TRANSFER_BITFIELD(rendererData.m_ReflectionProbeUsage, "m_ReflectionProbeUsage");
//    TRANSFER_BITFIELD(rendererData.m_RayTracingMode, "m_RayTracingMode");
//    TRANSFER_BITFIELD_FLAGS(rendererData.m_RayTraceProcedural, "m_RayTraceProcedural", kTreatIntegerValueAsBoolean);
//    transfer.Align();
//
//    transfer.Transfer(rendererData.m_RenderingLayerMask, "m_RenderingLayerMask");
//    transfer.Transfer(rendererData.m_RendererPriority, "m_RendererPriority");
//
//    if ((transfer.GetFlags() & kSerializeForInspector) != 0 || transfer.IsSerializingForGameRelease())
//    {
//        transfer.Transfer(rendererData.m_LightmapIndex[kStaticLightmap], "m_LightmapIndex", kHideInEditorMask | kDontAnimate);
//        transfer.Transfer(rendererData.m_LightmapIndex[kDynamicLightmap], "m_LightmapIndexDynamic", kHideInEditorMask | kDontAnimate);
//
//        transfer.Transfer(rendererData.m_LightmapST[kStaticLightmap], "m_LightmapTilingOffset", kHideInEditorMask | kDontAnimate);
//        transfer.Transfer(rendererData.m_LightmapST[kDynamicLightmap], "m_LightmapTilingOffsetDynamic", kHideInEditorMask | kDontAnimate);
//    }
//
//    transfer.Transfer(m_Materials, "m_Materials", kReorderable);
//    transfer.Transfer(m_RendererData.m_StaticBatchInfo, "m_StaticBatchInfo", kHideInEditorMask);
//    transfer.Transfer(m_StaticBatchRoot, "m_StaticBatchRoot", kHideInEditorMask);
//
//    TRANSFER(m_ProbeAnchor);
//    TRANSFER(m_LightProbeVolumeOverride);
//
//#if UNITY_EDITOR
//    if (!transfer.IsSerializingForGameRelease())
//    {
//        transfer.Transfer(m_ScaleInLightmap, "m_ScaleInLightmap", kHideInEditorMask | kDontAnimate);
//        TRANSFER_ENUM_WITH_FLAGS(m_ReceiveGI, kHideInEditorMask | kDontAnimate);
//        transfer.Transfer(m_PreserveUVs, "m_PreserveUVs", kHideInEditorMask | kDontAnimate);
//        transfer.Transfer(m_IgnoreNormalsForChartDetection, "m_IgnoreNormalsForChartDetection", kHideInEditorMask | kDontAnimate);
//        transfer.Transfer(m_ImportantGI, "m_ImportantGI", kHideInEditorMask | kDontAnimate);
//        transfer.Transfer(m_StitchLightmapSeams, "m_StitchLightmapSeams", kHideInEditorMask | kDontAnimate);
//        transfer.Align();
//        TRANSFER_ENUM_WITH_NAME_AND_FLAGS(m_SelectedEditorRenderState, "m_SelectedEditorRenderState", kHideInEditorMask | kDontAnimate);
//        transfer.Transfer(m_MinimumChartSize, "m_MinimumChartSize", kHideInEditorMask | kDontAnimate);
//        transfer.Transfer(m_AutoUVMaxDistance, "m_AutoUVMaxDistance", kHideInEditorMask | kDontAnimate);
//        transfer.Transfer(m_AutoUVMaxAngle, "m_AutoUVMaxAngle", kHideInEditorMask | kDontAnimate);
//#if INCLUDE_DYNAMIC_GI
//        transfer.Transfer(m_LightmapParameters, "m_LightmapParameters", kDontAnimate);
//#endif
//    }
//#endif
//    transfer.Align();
//
//    transfer.Transfer(m_SortingLayerID, "m_SortingLayerID", kHideInEditorMask | kDontAnimate);
//    transfer.Transfer(m_SortingLayer, "m_SortingLayer", kHideInEditorMask);
//    transfer.Transfer(m_SortingOrder, "m_SortingOrder", kHideInEditorMask);
//    transfer.Align();
}


int Renderer::GetLayer() const
{
    return GetGameObject().GetLayer();
}

void Renderer::BoundsChanged()
{
    // VZ: Open this later.
//    if (IsInScene())
//        GetRendererUpdateManager().DirtyDispatchUpdate(*this);
}

void Renderer::AddToScene()
{
    Assert(!IsInScene());
    // Assert(!IsVisibleInScene());

    RendererScene& scene = GetRendererScene();

    m_SceneHandle = scene.AddRenderer(this);
//    if (m_SceneHandle != kInvalidSceneHandle)
//    {
//        DebugAssert(scene.GetRendererNode(m_SceneHandle).renderer == this);
//
//        bool needsCullCallback = GetGameObject().GetSupportedMessages() & kOnWillRenderObject.GetOptimizedMask();
//        scene.SetRendererNeedsCullCallback(m_SceneHandle, needsCullCallback);
//        scene.SetShadowCastingMode(m_SceneHandle, GetShadowCastingMode());
//        scene.SetIsDynamicOccludee(m_SceneHandle, m_RendererData.m_DynamicOccludee);
//
//        UpdateLODGroup();
//#if ENABLE_TEXTURE_STREAMING
//        GetTextureStreamingManager().AddRenderer(*this);
//#endif
//
//        SetMotionVectorFrameIndex(-1);
//
//        GetRendererUpdateManager().AddRenderer(*this);
//
//#if GFX_SUPPORTS_RAY_TRACING
//        if (GetGraphicsCaps().supportsRayTracing)
//            GetRayTracingAccelerationStructureManager().AddRenderer(*this);
//#endif
//
//        RendererCountOnGameObjects::iterator countIter = gRendererCountOnGameObjects->find(GetGameObjectInstanceID());
//        if (countIter == gRendererCountOnGameObjects->end())
//        {
//            gRendererCountOnGameObjects->insert(RendererCountOnGameObjects::value_type(GetGameObjectInstanceID(), 1));
//            Transform& transform = GetComponent<Transform>();
//            GetTransformHierarchyChangeDispatch().SetSystemInterested(transform.GetTransformAccess(), kSystemParentHierarchy, true);
//        }
//        else
//        {
//            countIter->second += 1;
//        }
//
//        RendererAddedToScene();
//    }
}

void Renderer::RemoveFromScene()
{
    Assert(IsInScene());

    RendererScene& scene = GetRendererScene();

#if ENABLE_TEXTURE_STREAMING
    GetTextureStreamingManager().RemoveRenderer(*this);
#endif

//    GetRendererUpdateManager().RemoveRenderer(*this);

#if GFX_SUPPORTS_RAY_TRACING
    if (GetGraphicsCaps().supportsRayTracing)
        GetRayTracingAccelerationStructureManager().RemoveRenderer(*this);
#endif

//    bool wasVisible = IsVisibleInScene();

    BaseRenderer* remRenderer = scene.RemoveRenderer(m_SceneHandle);
    Assert(remRenderer == this);

    // RendererScene::ApplyPendingAddRemoveNodes() relies on the immediate invalidation of the handle
    // even though the change hasn't been applied yet and is actually pending.
    m_SceneHandle = kInvalidSceneHandle;
//
//    // Important: Notify renderer after it was removed from scene.
//    // That way it knows if it became invisible through culling or removal.
//    // VisibilityAfterRendererRemoval in RendererSceneTests checks this path.
//    if (wasVisible)
//        RendererBecameInvisible();
//
//    RendererCountOnGameObjects::iterator countIter = gRendererCountOnGameObjects->find(GetGameObjectInstanceID());
//    countIter->second -= 1;
//    if (countIter->second == 0)
//    {
//        Transform& transform = GetComponent<Transform>();
//        GetTransformHierarchyChangeDispatch().SetSystemInterested(transform.GetTransformAccess(), kSystemParentHierarchy, false);
//        gRendererCountOnGameObjects->erase(countIter);
//    }
//
//    RendererRemovedFromScene();
}


IMPLEMENT_REGISTER_CLASS(Renderer, 12);
IMPLEMENT_OBJECT_SERIALIZE(Renderer);
INSTANTIATE_TEMPLATE_TRANSFER(Renderer);