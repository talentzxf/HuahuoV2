//
// Created by VincentZhang on 5/17/2022.
//

#include "Renderer.h"
#include "BaseClasses/GameObject.h"

Renderer::Renderer(RendererType type/*, MemLabelId label*/, ObjectCreationMode mode)
        :   Super(/*label,*/ mode)
        ,   BaseRenderer(type)
//        ,   m_Materials(GetMemoryLabel())
//        ,   m_SceneHandle(kInvalidSceneHandle)
//        ,   m_PerMaterialOverrideProperties(GetMemoryLabel())
//        ,   m_LastLightProbeTetIndex(-1)
//        ,   m_LODGroup(NULL)
        ,   m_Enabled(true)
//        ,   m_IsRenderable(true)
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

template<class TransferFunction>
void Renderer::Transfer(TransferFunction& transfer) {
    Super::Transfer(transfer);

    transfer.Transfer(m_Enabled, "m_Enabled", kHideInEditorMask);
}

int Renderer::GetLayer() const
{
    return GetGameObject().GetLayer();
}

IMPLEMENT_REGISTER_CLASS(Renderer, 12);
IMPLEMENT_OBJECT_SERIALIZE(Renderer);
INSTANTIATE_TEMPLATE_TRANSFER(Renderer);