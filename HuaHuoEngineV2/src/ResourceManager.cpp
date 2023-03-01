//
// Created by VincentZhang on 2023-02-28.
//

#include "ResourceManager.h"
#include "Serialize/PersistentManager.h"

extern std::string StoreFilePath;

ResourceManager *gDefaultResourceManager = NULL;

ResourceManager *GetDefaultResourceManager() {
    if (gDefaultResourceManager == NULL) {
        gDefaultResourceManager = Object::Produce<ResourceManager>();
        GetPersistentManager().MakeObjectPersistent(gDefaultResourceManager->GetInstanceID(), StoreFilePath);
    }

    return gDefaultResourceManager;
}

void SetDefaultResourceManager(ResourceManager* resourceManager){
    gDefaultResourceManager = resourceManager;

    printf("Set Default object store manager\n");
}

ResourceManager *ResourceManager::GetDefaultResourceManager() {
    return ::GetDefaultResourceManager();
}

IMPLEMENT_REGISTER_CLASS(ResourceManager, 10025);

IMPLEMENT_OBJECT_SERIALIZE(ResourceManager);

INSTANTIATE_TEMPLATE_TRANSFER(ResourceManager);

template<class TransferFunction>
void ResourceManager::Transfer(TransferFunction &transfer) {
    Super::Transfer(transfer);
}

bool ResourceManager::RegisterFile(std::string &fileName) {
    if (mFileNameDataMap.contains(fileName)) {
        return false;
    }

    mFileNameDataMap[fileName] = std::vector<UInt8>();
    return true;
}


void ResourceManager::AwakeFromLoad(AwakeFromLoadMode awakeMode){
    Super::AwakeFromLoad(awakeMode);

    SetDefaultResourceManager(this);
}