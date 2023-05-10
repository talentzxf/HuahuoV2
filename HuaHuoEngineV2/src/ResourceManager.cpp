//
// Created by VincentZhang on 2023-02-28.
//

#include "ResourceManager.h"
#include "Serialize/PersistentManager.h"
#include "openssl/md5.h"

extern std::string StoreFilePath;

ResourceManager *gDefaultResourceManager = NULL;

ResourceManager *GetDefaultResourceManager() {
    if (gDefaultResourceManager == NULL) {
        gDefaultResourceManager = Object::Produce<ResourceManager>();
    }

    GetPersistentManager().MakeObjectPersistent(gDefaultResourceManager->GetInstanceID(), StoreFilePath);

    return gDefaultResourceManager;
}

void SetDefaultResourceManager(ResourceManager* resourceManager){
    if(gDefaultResourceManager){
        // Merge the resource manager into the current resource manager

        if(gDefaultResourceManager == resourceManager)
            return;

        gDefaultResourceManager->Merge(resourceManager);

        // Delete it, so it won't be persistent
        DestroySingleObject(resourceManager);
    }else{
        gDefaultResourceManager = resourceManager;
        printf("Set Default object store manager\n");
    }
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
    TRANSFER(mBinaryResources);
}

void ResourceManager::Merge(ResourceManager *other) {
    for(auto resourceItr: other->mBinaryResources){
        Hash128 resourceHash = resourceItr.first;
        if(!this->mBinaryResources.contains(resourceHash)){
            this->mBinaryResources[resourceHash] = resourceItr.second;
        }
    }
}

void ResourceManager::AwakeFromLoad(AwakeFromLoadMode awakeMode){
    Super::AwakeFromLoad(awakeMode);

    SetDefaultResourceManager(this);
}

bool ResourceManager::LoadBinaryResource(const char* fileName, const char* mimeType, UInt8* pData, long dataSize){
    MD5_CTX md5Ctx;
    MD5_Init(&md5Ctx);
    MD5_Update(&md5Ctx, pData, dataSize);
    Hash128 resultHash;
    MD5_Final(resultHash.hashData.bytes, &md5Ctx);

    if(this->mBinaryResources.contains(resultHash)){
        printf("Resource already existed with name:%s\n", fileName);
        return false;
    }

    BinaryResource* binaryResource = Object::Produce<BinaryResource>();
    GetPersistentManager().MakeObjectPersistent(binaryResource->GetInstanceID(), StoreFilePath);
    binaryResource->SetFileData(fileName, mimeType, pData, dataSize, resultHash);
    mBinaryResources[resultHash] = binaryResource;

    printf("Added new resource into ResourceManager, name:%s hash:%s\n", fileName, Hash128ToString(resultHash).c_str());
    return true;
}