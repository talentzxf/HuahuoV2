//
// Created by VincentZhang on 2023-02-28.
//

#ifndef HUAHUOENGINEV2_RESOURCEMANAGER_H
#define HUAHUOENGINEV2_RESOURCEMANAGER_H
#include "TypeSystem/Object.h"
#include "Utilities/Hash128.h"
#include "BinaryResource.h"
#include "BaseClasses/PPtr.h"

class ResourceManager : public Object {
REGISTER_CLASS(ResourceManager);

DECLARE_OBJECT_SERIALIZE();
public:
    ResourceManager(MemLabelId label, ObjectCreationMode mode)
            : Super(label, mode){
        printf("Resource manager created\n");
    }

    static ResourceManager *GetDefaultResourceManager();

    void AwakeFromLoad(AwakeFromLoadMode awakeMode) override;

    PPtr<BinaryResource> GetResourceByMD5(const char* resourceMD5){
        Hash128 resourceMD5Hash = StringToHash128(resourceMD5);
        if(!mBinaryResources.contains(resourceMD5Hash)){
            return NULL;
        }

        return mBinaryResources[resourceMD5Hash];
    }

    bool IsBinaryResourceExist(const char *resourceMd5) {
        Hash128 resourceMD5Hash = StringToHash128(resourceMd5);
        return mBinaryResources.contains(resourceMD5Hash);
    }

    void AddBinaryResource(BinaryResource* pResource);
    bool LoadBinaryResource(const char* fileName, const char* mimeType, UInt8* pData, long dataSize);
private:
    void Merge(ResourceManager* other);
    friend void SetDefaultResourceManager(ResourceManager* resourceManager);
private:
    std::map<Hash128, PPtr<BinaryResource>> mBinaryResources;
};

ResourceManager *GetDefaultResourceManager();

#endif //HUAHUOENGINEV2_RESOURCEMANAGER_H
