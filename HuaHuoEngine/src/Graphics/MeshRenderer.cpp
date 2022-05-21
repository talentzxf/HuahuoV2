//
// Created by VincentZhang on 5/13/2022.
//

#include "MeshRenderer.h"

IMPLEMENT_REGISTER_CLASS(MeshRenderer, 13);
IMPLEMENT_OBJECT_SERIALIZE(MeshRenderer);
INSTANTIATE_TEMPLATE_TRANSFER(MeshRenderer);


MeshRenderer::MeshRenderer(MemLabelId label, ObjectCreationMode mode)
        :   Super(kRendererMesh, label, mode)
        ,   m_CachedMesh(NULL)
//        ,   m_CachedAdditionalVertexStreams(NULL)
//        ,   m_CachedEnlightenVertexStream(NULL)
//        ,   m_DisableStaticAndDynamicBatching(false)
//        ,   m_MeshNode(this)
        ,   m_AdditionalVertexStreamsNode(this)
        ,   m_EnlightenVertexStreamNode(this)
{
    // SET_CACHED_SURFACE_AREA_DIRTY();
#if UNITY_EDITOR
    ClearCachedNormalizedLightmapArea();
#endif

    SharedRendererData& srd = m_RendererData;
    srd.m_RayTracingMode = kRayTracingModeDynamicTransform;

    UpdateLocalAABB();
}

void MeshRenderer::CalculateLocalAABB(AABB& outAABB, const MeshRenderer& renderer, const Mesh& mesh)
{
    if (renderer.IsPartOfStaticBatch())
    {
        const StaticBatchInfo& info = renderer.GetStaticBatchInfo();
        DebugAssertMsg(info.subMeshCount <= renderer.GetMaterialCount(), "Static batch has more submeshes than materials!");
        if (info.subMeshCount == 1)
            outAABB = mesh.GetBounds(info.firstSubMesh);
        else
        {
            MinMaxAABB minMaxAABB;
            for (int s = 0; s < info.subMeshCount; ++s)
                minMaxAABB.Encapsulate(mesh.GetBounds(info.firstSubMesh + s));
            outAABB = AABB(minMaxAABB);
        }
    }
    else
    {
        outAABB = mesh.GetBounds();
    }
}

void MeshRenderer::UpdateLocalAABB()
{
    if (m_CachedMesh)
    {
        DebugAssert(m_CachedMesh->GetInstanceID() == m_Mesh.GetInstanceID());
        CalculateLocalAABB(GetWritableTransformInfo().localAABB, *this, *m_CachedMesh);
    }
    else
        GetWritableTransformInfo().localAABB.SetCenterAndExtent(Vector3f::zero, Vector3f::zero);

    BoundsChanged();
}


template<class TransferFunction> inline
void MeshRenderer::Transfer(TransferFunction& transfer)
{
    Super::Transfer(transfer);

    // Changed in 2019.3: Always serialize user's additional vertex streams.
    transfer.Transfer(m_AdditionalVertexStreams, "m_AdditionalVertexStreams", kHideInEditorMask);

    if (transfer.IsSerializingForGameRelease() || transfer.GetFlags() & kSerializeForInspector)
        transfer.Transfer(m_EnlightenVertexStream, "m_EnlightenVertexStream", kHideInEditorMask);
}
