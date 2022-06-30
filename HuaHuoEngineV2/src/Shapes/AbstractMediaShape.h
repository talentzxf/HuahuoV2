//
// Created by VincentZhang on 6/30/2022.
//

#ifndef HUAHUOENGINEV2_ABSTRACTMEDIASHAPE_H
#define HUAHUOENGINEV2_ABSTRACTMEDIASHAPE_H
#include "BaseShape.h"

class AbstractMediaShape : public BaseShape{
    REGISTER_CLASS_TRAITS(kTypeIsAbstract);
    REGISTER_CLASS(AbstractMediaShape);
    DECLARE_OBJECT_SERIALIZE()
public:
    AbstractMediaShape(MemLabelId label, ObjectCreationMode mode)
    :Super(label, mode)
    ,mType("UnknownType")
    {
    }

    void SetData(UInt8* pData, UInt32 dataSize);

    UInt8 GetDataAtIndex(UInt32 index);

    // TODO: This is not working for WebIDL, not sure why
    void LoadData(UInt8* pData);
    UInt32 GetDataSize(){
        return this->data.size();
    }

    void SetMimeType(std::string type){
        this->mType = type;
    }

    char* GetMimeType(){
        return const_cast<char*>(this->mType.c_str());
    }

    void SetFileName(std::string fName){
        this->mFileName = fName;
    }

    char* GetFileName(){
        return const_cast<char*>(mFileName.c_str());
    }

private:
    std::vector<UInt8> data;
    std::string mType;
    std::string mFileName;
};


#endif //HUAHUOENGINEV2_ABSTRACTMEDIASHAPE_H
