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
    TRANSFER(this->data);
    TRANSFER(this->mType);
}

void ImageShape::SetImageData(UInt8 *pData, UInt32 dataSize) {
    this->data.resize(dataSize);
    memcpy(this->data.data(), pData, dataSize);
}

UInt8 ImageShape::GetImageDataAtIndex(UInt32 index){
    return this->data[index];
}

void ImageShape::LoadImageData(UInt8 *pData) {
    int size = this->data.size();

    printf("First byte:%d\n", this->data[0]);
    printf("Second byte:%d\n", this->data[1]);
    memcpy(pData, this->data.data(), size);

    printf("After First byte:%d\n", pData[0]);
    printf("After Second byte:%d\n", pData[1]);
}