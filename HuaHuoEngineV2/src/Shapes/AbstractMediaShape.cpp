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
    TRANSFER(this->mType);
    TRANSFER(this->mFileName);
}

void AbstractMediaShape::SetData(UInt8 *pData, UInt32 dataSize) {
    std::vector<UInt8> &fileData = GetDefaultResourceManager()->GetFileData(mFileName);
    fileData.resize(dataSize);
    memcpy(fileData.data(), pData, dataSize);
}

UInt8 AbstractMediaShape::GetDataAtIndex(UInt32 index) {
    std::vector<UInt8> &fileData = GetFileDataPointer();
    return fileData[index];
}

void AbstractMediaShape::LoadData(UInt8 *pData) {
    std::vector<UInt8> &fileData = GetDefaultResourceManager()->GetFileData(mFileName);
    int size = fileData.size();
    memcpy(pData, fileData.data(), size);
}