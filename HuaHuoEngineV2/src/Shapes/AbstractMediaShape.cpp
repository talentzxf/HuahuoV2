//
// Created by VincentZhang on 6/30/2022.
//

#include "AbstractMediaShape.h"

IMPLEMENT_REGISTER_CLASS(AbstractMediaShape, 10014);
IMPLEMENT_OBJECT_SERIALIZE(AbstractMediaShape)
INSTANTIATE_TEMPLATE_TRANSFER(AbstractMediaShape);

template <class TransferFunction>
void AbstractMediaShape::Transfer(TransferFunction &transfer) {
    Super::Transfer(transfer);
    TRANSFER(this->data);
    TRANSFER(this->mType);
    TRANSFER(this->mFileName);
}

void AbstractMediaShape::SetData(UInt8 *pData, UInt32 dataSize){
    this->data.resize(dataSize);
    memcpy(this->data.data(), pData, dataSize);
}

UInt8 AbstractMediaShape::GetDataAtIndex(UInt32 index){
    return this->data[index];
}

void AbstractMediaShape::LoadData(UInt8 *pData) {
    int size = this->data.size();
    memcpy(pData, this->data.data(), size);
}