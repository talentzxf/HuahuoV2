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
    {
    }

    UInt8 GetDataAtIndex(UInt32 index);

    UInt32 GetDataSize(){
        return mBinaryResource->GetFileSize();
    }

    const char* GetMimeType(){
        return mBinaryResource->GetMimeType().c_str();
    }

    void SetResourceByMD5(const char* resourceMD5){
        mBinaryResource = GetDefaultResourceManager()->GetResourceByMD5(resourceMD5);
    }

    const char* GetFileName(){
        return mBinaryResource->GetFileName().c_str();
    }

    std::vector<UInt8>& GetFileDataPointer(){
        return mBinaryResource->GetFileDataPointer();
    }

    const char* GetResourceMD5(){
        return mBinaryResource->GetMD5();
    }

private:
    PPtr<BinaryResource> mBinaryResource;
};


#endif //HUAHUOENGINEV2_ABSTRACTMEDIASHAPE_H
