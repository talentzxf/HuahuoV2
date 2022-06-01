//
// Created by VincentZhang on 6/1/2022.
//

#include "LineShape.h"
#include "Serialize/SerializeUtility.h"

IMPLEMENT_REGISTER_CLASS(LineShape, 10003);

IMPLEMENT_OBJECT_SERIALIZE(LineShape);
INSTANTIATE_TEMPLATE_TRANSFER(LineShape);

template<class TransferFunction>
void LineShape::Transfer(TransferFunction &transfer) {
    Super::Transfer(transfer);
    TRANSFER(this->p1);
    TRANSFER(this->p2);
}