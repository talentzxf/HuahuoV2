//
// Created by VincentZhang on 6/27/2022.
//

#include "ImageShape.h"

IMPLEMENT_REGISTER_CLASS(ImageShape, 10012);
IMPLEMENT_OBJECT_SERIALIZE(ImageShape)
INSTANTIATE_TEMPLATE_TRANSFER(ImageShape);

template <class TransferFunction>
void ImageShape::Transfer(TransferFunction &transfer) {
    Super::Transfer(transfer);
    TRANSFER(this->mIsAnimation);
}
