//
// Created by VincentZhang on 6/30/2022.
//

#include "AbstractMediaShape.h"

IMPLEMENT_REGISTER_CLASS(AbstractMediaShape, 10014);

IMPLEMENT_OBJECT_SERIALIZE(AbstractMediaShape)

INSTANTIATE_TEMPLATE_TRANSFER(AbstractMediaShape);

template<class TransferFunction>
void AbstractMediaShape::Transfer(TransferFunction &transfer) {
    Super::Transfer(transfer);
    TRANSFER(this->mBinaryResource);
}

UInt8 AbstractMediaShape::GetDataAtIndex(UInt32 index) {
    std::vector<UInt8> &fileData = GetFileDataPointer();
    return fileData[index];
}