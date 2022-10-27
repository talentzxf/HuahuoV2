//
// Created by VincentZhang on 2022-10-27.
//

#include "MirrorShape.h"

IMPLEMENT_REGISTER_CLASS(MirrorShape, 10022);

IMPLEMENT_OBJECT_SERIALIZE(MirrorShape);
INSTANTIATE_TEMPLATE_TRANSFER(MirrorShape);

template<class TransferFunction>
void MirrorShape::Transfer(TransferFunction &transfer) {
    Super::Transfer(transfer);
    TRANSFER(this->p1);
    TRANSFER(this->p2);
}
