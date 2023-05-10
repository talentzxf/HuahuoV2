//
// Created by VincentZhang on 2023-05-09.
//

#include "BinaryResource.h"
#include "Serialize/SerializeUtility.h"
#include "openssl/md5.h"
#include "Utilities/Hash128.h"
#include "ResourceManager.h"

IMPLEMENT_REGISTER_CLASS(BinaryResource, 10027);

IMPLEMENT_OBJECT_SERIALIZE(BinaryResource);

INSTANTIATE_TEMPLATE_TRANSFER(BinaryResource);

template<class TransferFunction>
void BinaryResource::Transfer(TransferFunction &transfer) {
    Super::Transfer(transfer);
    TRANSFER(mFileName);
    TRANSFER(mFileData);
    TRANSFER(mFileMime);
    TRANSFER(mFileMD5);
}

// Return the MD4 of the resource.
void BinaryResource::SetFileData(const char* fileName, const char* mimeType, UInt8* pData, UInt32 dataSize, Hash128 resultHash){
    mFileMime = mimeType;
    mFileName = fileName;
    mFileData.resize(dataSize);
    memcpy(mFileData.data(), pData, dataSize);

    if(!resultHash.IsValid()){
        MD5_CTX md5Ctx;
        MD5_Init(&md5Ctx);
        MD5_Update(&md5Ctx, mFileData.data(), mFileData.size());
        MD5_Final(mFileMD5.hashData.bytes, &md5Ctx);
    }else{
        mFileMD5 = resultHash;
    }
}

void BinaryResource::AwakeFromLoad(AwakeFromLoadMode awakeMode) {
    Super::AwakeFromLoad(awakeMode);

    GetDefaultResourceManager()->AddBinaryResource(this);
}
