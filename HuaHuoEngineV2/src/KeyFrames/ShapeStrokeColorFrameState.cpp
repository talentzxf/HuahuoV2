//
// Created by VincentZhang on 2022-08-22.
//

#include "ShapeStrokeColorFrameState.h"


IMPLEMENT_REGISTER_CLASS(ShapeStrokeColorFrameState, 10018);

IMPLEMENT_OBJECT_SERIALIZE(ShapeStrokeColorFrameState);

INSTANTIATE_TEMPLATE_TRANSFER(ShapeStrokeColorFrameState);

template<class TransferFunction>
void ShapeStrokeColorFrameState::Transfer(TransferFunction &transfer) {
    Super::Transfer(transfer);
}