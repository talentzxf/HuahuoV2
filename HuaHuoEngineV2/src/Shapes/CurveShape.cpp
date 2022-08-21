//
// Created by VincentZhang on 8/21/2022.
//

#include "CurveShape.h"

IMPLEMENT_REGISTER_CLASS(CurveShape, 10017);

IMPLEMENT_OBJECT_SERIALIZE(CurveShape);

INSTANTIATE_TEMPLATE_TRANSFER(CurveShape);

template <class TransferFunction>
void CurveShape::Transfer(TransferFunction &transfer) {
    Super::Transfer(transfer);
}