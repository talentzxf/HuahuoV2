//
// Created by VincentZhang on 5/23/2022.
//

#include "Canvas2D.h"

IMPLEMENT_REGISTER_CLASS(Canvas2DRenderer, 14);
IMPLEMENT_OBJECT_SERIALIZE(Canvas2DRenderer);
INSTANTIATE_TEMPLATE_TRANSFER(Canvas2DRenderer);

Canvas2DRenderer::Canvas2DRenderer(MemLabelId label, ObjectCreationMode mode)
    :Super(kRenderer2D, label, mode)
{
}

template<class TransferFunction> inline
void Canvas2DRenderer::Transfer(TransferFunction& transfer)
{
    Super::Transfer(transfer);
    TRANSFER(object2dArray);
}