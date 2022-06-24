//
// Created by VincentZhang on 2022-06-24.
//

#include "RectangleShape.h"

IMPLEMENT_REGISTER_CLASS(RectangleShape, 10011);

IMPLEMENT_OBJECT_SERIALIZE(RectangleShape);
INSTANTIATE_TEMPLATE_TRANSFER(RectangleShape);

template<class TransferFunction>
void RectangleShape::Transfer(TransferFunction &transfer) {
    Super::Transfer(transfer);
    TRANSFER(this->p1);
    TRANSFER(this->p2);
}
