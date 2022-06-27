//
// Created by VincentZhang on 6/27/2022.
//

#ifndef HUAHUOENGINEV2_IMAGESHAPE_H
#define HUAHUOENGINEV2_IMAGESHAPE_H


#include "BaseShape.h"

class ImageShape : public BaseShape{
    REGISTER_CLASS(ImageShape);
    DECLARE_OBJECT_SERIALIZE()
public:
    ImageShape(MemLabelId label, ObjectCreationMode mode)
    :Super(label, mode)
    ,mType("UnknownType")
    {
    }

    virtual char* GetName() override{
        return "ImageShape";
    }

    void SetImageMimeType(std::string type){
        this->mType = type;
    }

    char* GetImageMimeType(){
        return const_cast<char*>(this->mType.c_str());
    }

    void SetImageData(UInt8* pData, UInt32 dataSize);

    UInt8 GetImageDataAtIndex(UInt32 index);

    // TODO: This is not working for WebIDL, not sure why
    void LoadImageData(UInt8* pData);
    UInt32 GetImageDataSize(){
        return this->data.size();
    }

private:
    std::vector<UInt8> data;
    std::string mType;
};


#endif //HUAHUOENGINEV2_IMAGESHAPE_H
