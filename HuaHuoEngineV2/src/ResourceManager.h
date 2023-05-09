//
// Created by VincentZhang on 2023-02-28.
//

#ifndef HUAHUOENGINEV2_RESOURCEMANAGER_H
#define HUAHUOENGINEV2_RESOURCEMANAGER_H
#include "TypeSystem/Object.h"
#include "Utilities/Hash128.h"
#include "BinaryResource.h"

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

    void SetFileData(const char* fileName, const char* mimeType, UInt8* pData, UInt32 dataSize);
private:
    void Merge(ResourceManager* other);
    friend void SetDefaultResourceManager(ResourceManager* resourceManager);
private:
    std::map<Hash128, PPtr<BinaryResource>> mBinaryResources;

};

ResourceManager *GetDefaultResourceManager();

#endif //HUAHUOENGINEV2_RESOURCEMANAGER_H
