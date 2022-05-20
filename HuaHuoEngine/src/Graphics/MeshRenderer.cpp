//
// Created by VincentZhang on 5/13/2022.
//

#include "MeshRenderer.h"

IMPLEMENT_REGISTER_CLASS(MeshRenderer, 13);
IMPLEMENT_OBJECT_SERIALIZE(MeshRenderer);
INSTANTIATE_TEMPLATE_TRANSFER(MeshRenderer);

template<class TransferFunction> inline
void MeshRenderer::Transfer(TransferFunction& transfer)
{
    Super::Transfer(transfer);

    // Changed in 2019.3: Always serialize user's additional vertex streams.
    transfer.Transfer(m_AdditionalVertexStreams, "m_AdditionalVertexStreams", kHideInEditorMask);

    if (transfer.IsSerializingForGameRelease() || transfer.GetFlags() & kSerializeForInspector)
        transfer.Transfer(m_EnlightenVertexStream, "m_EnlightenVertexStream", kHideInEditorMask);
}
