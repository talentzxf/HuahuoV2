//
// Created by VincentZhang on 6/29/2022.
//

#include "AudioShape.h"

IMPLEMENT_REGISTER_CLASS(AudioShape, 10013);
IMPLEMENT_OBJECT_SERIALIZE(AudioShape)
INSTANTIATE_TEMPLATE_TRANSFER(AudioShape);

template <class TransferFunction>
void AudioShape::Transfer(TransferFunction &transfer) {
    Super::Transfer(transfer);
}

