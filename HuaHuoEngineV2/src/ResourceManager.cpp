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
    }

    GetPersistentManager().MakeObjectPersistent(gDefaultResourceManager->GetInstanceID(), StoreFilePath);

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

    TRANSFER(mFileNameDataMap);
    TRANSFER(mFileNameMimeMap);
}

void ResourceManager::SetFileData(const char* fileName, const char* mimeType, UInt8* pData, UInt32 dataSize){
    std::string fileNameStr(fileName);

    if(!mFileNameDataMap.contains(fileName)){
        RegisterFile(fileNameStr);
    }

    std::vector<UInt8> &fileData = mFileNameDataMap[fileNameStr];
    fileData.resize(dataSize);
    memcpy(fileData.data(), pData, dataSize);

    mFileNameMimeMap[fileNameStr] = mimeType;
}

bool ResourceManager::RegisterFile(std::string &fileName) {
    printf("ResourceManager: RegisterFile for:%s\n", fileName.c_str());
    if (mFileNameDataMap.contains(fileName)) {
        printf("ResourceManager: File:%s has already been registered before\n", fileName.c_str());
        return false;
    }

    printf("ResourceManager: File:%s has not been registered yet, register here\n", fileName.c_str());
    mFileNameDataMap[fileName] = std::vector<UInt8>();
    mFileNameMimeMap[fileName] = "unknown";
    return true;
}


void ResourceManager::AwakeFromLoad(AwakeFromLoadMode awakeMode){
    Super::AwakeFromLoad(awakeMode);

    SetDefaultResourceManager(this);
}