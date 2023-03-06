//
// Created by VincentZhang on 6/30/2022.
//

#ifndef HUAHUOENGINEV2_ABSTRACTMEDIASHAPE_H
#define HUAHUOENGINEV2_ABSTRACTMEDIASHAPE_H
#include "BaseShape.h"
#include "ResourceManager.h"

// TODO: Some functionality duplicate with class BinaryResource
class AbstractMediaShape : public BaseShape{
    REGISTER_CLASS_TRAITS(kTypeIsAbstract);
    REGISTER_CLASS(AbstractMediaShape);
    DECLARE_OBJECT_SERIALIZE()
public:
    AbstractMediaShape(MemLabelId label, ObjectCreationMode mode)
    :Super(label, mode)
    ,mType("UnknownType")
    ,mFileDataPointer(NULL)
    {
    }

    void SetData(UInt8* pData, UInt32 dataSize);

    UInt8 GetDataAtIndex(UInt32 index);

    // TODO: This is not working for WebIDL, not sure why
    void LoadData(UInt8* pData);
    UInt32 GetDataSize(){
        return GetDefaultResourceManager()->GetDataSize(mFileName);
    }

    void SetMimeType(std::string type){
        this->mType = type;
    }

    char* GetMimeType(){
        return const_cast<char*>(this->mType.c_str());
    }

    void SetFileName(std::string fName){
        this->mFileName = fName;

        GetDefaultResourceManager()->RegisterFile(fName);
    }

    char* GetFileName(){
        return const_cast<char*>(mFileName.c_str());
    }

    std::vector<UInt8>& GetFileDataPointer(){
        if(mFileDataPointer == NULL){
            mFileDataPointer = &GetDefaultResourceManager()->GetFileData(mFileName);
        }

        return *mFileDataPointer;
    }

private:
    std::string mType;
    std::string mFileName;

    std::vector<UInt8>* mFileDataPointer;
};


#endif //HUAHUOENGINEV2_ABSTRACTMEDIASHAPE_H
