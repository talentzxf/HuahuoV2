//
// Created by VincentZhang on 2023-05-09.
//

#ifndef HUAHUOENGINEV2_BINARYRESOURCE_H
#define HUAHUOENGINEV2_BINARYRESOURCE_H
#include "TypeSystem/Object.h"
#include "Utilities/Hash128.h"

class BinaryResource: public Object{
    REGISTER_CLASS(BinaryResource);
    DECLARE_OBJECT_SERIALIZE();
public:
    BinaryResource(MemLabelId label, ObjectCreationMode mode)
    :Super(label, mode){

    }

    void SetFileData(const char* fileName, const char* mimeType, UInt8* pData, UInt32 dataSize, Hash128 resultHash = Hash128());

    UInt32 GetFileSize(){
        return mFileData.size();
    }

    void SetFileName(const char* fileName){
        mFileName = fileName;
    }

    std::string& GetFileName(){
        return mFileName;
    }

    std::vector<UInt8>& GetFileData(std::string &fileName){
        return mFileData;
    }

    std::string& GetMimeType(){
        return mFileMime;
    }

    std::vector<UInt8>& GetFileDataPointer(){
        return mFileData;
    }

    const char* GetMD5(){
        mMD5String = Hash128ToString(mFileMD5);
        return mMD5String.c_str();
    }

private:
    std::string mFileName;
    std::string mFileMime;
    std::vector<UInt8> mFileData;
    Hash128 mFileMD5;
    std::string mMD5String;
};


#endif //HUAHUOENGINEV2_BINARYRESOURCE_H
