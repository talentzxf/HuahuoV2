//
// Created by VincentZhang on 2022-11-24.
//

#include "NailShape.h"


IMPLEMENT_REGISTER_CLASS(NailShape, 10019);

IMPLEMENT_OBJECT_SERIALIZE(NailShape);
INSTANTIATE_TEMPLATE_TRANSFER(NailShape);

template<class TransferFunction>
void NailShape::Transfer(TransferFunction &transfer) {
    Super::Transfer(transfer);
}