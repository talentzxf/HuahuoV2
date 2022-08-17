//
// Created by VincentZhang on 2022-08-12.
//

#include "TextShape.h"

IMPLEMENT_REGISTER_CLASS(TextShape, 10016);

IMPLEMENT_OBJECT_SERIALIZE(TextShape);
INSTANTIATE_TEMPLATE_TRANSFER(TextShape);

template<class TransferFunction>
void TextShape::Transfer(TransferFunction &transfer) {
    Super::Transfer(transfer);
    TRANSFER(text);
}