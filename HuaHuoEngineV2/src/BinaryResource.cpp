//
// Created by VincentZhang on 2023-05-09.
//

#include "BinaryResource.h"
#include "Serialize/SerializeUtility.h"
#include "openssl/md5.h"
#include "Utilities/Hash128.h"

IMPLEMENT_REGISTER_CLASS(BinaryResource, 10027);

IMPLEMENT_OBJECT_SERIALIZE(BinaryResource);

INSTANTIATE_TEMPLATE_TRANSFER(BinaryResource);

template<class TransferFunction>
void BinaryResource::Transfer(TransferFunction &transfer) {
    Super::Transfer(transfer);

    TRANSFER(mFileName);
    TRANSFER(mFileContent);
    TRANSFER(mFileMime);
}

// Return the MD4 of the resource.
void BinaryResource::SetFileData(const char* fileName, const char* mimeType, UInt8* pData, UInt32 dataSize){
    mFileName = fileName;
    mFileContent.resize(dataSize);
    memcpy(mFileContent.data(), pData, dataSize);

    // Calculate MD4.
    MD5_CTX md4_ctx;
    MD5_Init(&md4_ctx);
    MD5_Update(&md4_ctx, mFileContent.data(), mFileContent.size());
    MD5_Final(mFileHash.hashData.bytes,&md4_ctx);
}