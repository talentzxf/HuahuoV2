//
// Created by VincentZhang on 2022-06-16.
//

#include "ShapeColorFrameState.h"


IMPLEMENT_REGISTER_CLASS(ShapeColorFrameState, 10009);

IMPLEMENT_OBJECT_SERIALIZE(ShapeColorFrameState);

INSTANTIATE_TEMPLATE_TRANSFER(ShapeColorFrameState);

template<class TransferFunction>
void ShapeColorFrameState::Transfer(TransferFunction &transfer) {
    Super::Transfer(transfer);
    Transfer(m_KeyFrames);
}