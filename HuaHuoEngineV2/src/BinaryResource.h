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

    void SetFileData(const char* fileName, const char* mimeType, UInt8* pData, UInt32 dataSize);

    UInt32 GetFileSize(){
        return mFileContent.size();
    }

    std::string& GetFileName(){
        return mFileName;
    }

    std::vector<UInt8>& GetFileData(std::string &fileName){
        return mFileContent;
    }

    std::string& GetMimeType(){
        return mFileMime;
    }

private:
    std::string mFileName;
    std::string mFileMime;
    std::vector<UInt8> mFileContent;
    Hash128 mFileHash;
};


#endif //HUAHUOENGINEV2_BINARYRESOURCE_H
