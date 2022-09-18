//
// Created by VincentZhang on 6/1/2022.
//

#include "ObjectStore.h"
#include "Memory/MemoryMacros.h"
#include "Serialize/SerializeUtility.h"

std::string StoreFilePath("mem://objectstore.data");

ObjectStoreManager* gDefaultObjectStoreManager = NULL;

ObjectStoreManager* GetDefaultObjectStoreManager(){
    if(gDefaultObjectStoreManager == NULL){
        gDefaultObjectStoreManager = Object::Produce<ObjectStoreManager>();
        gDefaultObjectStoreManager->SetIsGlobal(true);

        GetPersistentManager().MakeObjectPersistent(gDefaultObjectStoreManager->GetInstanceID(), StoreFilePath);
    }

    return gDefaultObjectStoreManager;
}

void SetDefaultObjectStoreManager(ObjectStoreManager* objectStoreManager){
    gDefaultObjectStoreManager = objectStoreManager;

    printf("Set Default object store manager\n");
}

void ObjectStoreManager::SetStoreFilePath(char* inStoreFilePath){
    StoreFilePath = inStoreFilePath;
}

char* ObjectStoreManager::GetStoreFilePath(){
    return const_cast<char*>(StoreFilePath.c_str());
}

IMPLEMENT_REGISTER_CLASS(ObjectStoreManager, 10004);

IMPLEMENT_OBJECT_SERIALIZE(ObjectStoreManager);
INSTANTIATE_TEMPLATE_TRANSFER(ObjectStoreManager);
template<class TransferFunction>
void ObjectStoreManager::Transfer(TransferFunction &transfer) {
    Super::Transfer(transfer);
    TRANSFER(m_IsGlobal);
    TRANSFER(allStores);
    TRANSFER(currentStore);
}

ObjectStoreManager* ObjectStoreManager::GetDefaultObjectStoreManager(){
    return ::GetDefaultObjectStoreManager();
}

void ObjectStoreManager::AwakeFromLoad(AwakeFromLoadMode awakeMode){
    Super::AwakeFromLoad(awakeMode);

    if(this->m_IsGlobal){
        SetDefaultObjectStoreManager(this);

        size_t layerCount = GetCurrentStore()->GetLayerCount();
        for(int i = 0 ; i < layerCount; i++){
            GetCurrentStore()->GetLayer(i)->AwakeAllShapes(awakeMode);
        }
    }
}

IMPLEMENT_REGISTER_CLASS(ObjectStore, 10000);

IMPLEMENT_OBJECT_SERIALIZE(ObjectStore);
INSTANTIATE_TEMPLATE_TRANSFER(ObjectStore);

template<class TransferFunction>
void ObjectStore::Transfer(TransferFunction &transfer) {
    Super::Transfer(transfer);

    printf("Writing layermap:%d\n", layerMap.size());
    TRANSFER(layerMap);

    TRANSFER(layers);

    printf("Writing currentlayer\n");
    TRANSFER(currentLayer);
    TRANSFER(maxFrameId);
    TRANSFER(mStoreId);
}

#if WEB_ENV
emscripten::val writeObjectStoreInMemoryFile(){
    int writeResult = GetPersistentManager().WriteFile(StoreFilePath);
    printf("%s,%d; file:%s\n writeResult:%d\n", __FILE__, __LINE__ , StoreFilePath.c_str(), writeResult);
    UInt8* bufferPtr = GetMemoryFileSystem()->GetDataPtr(StoreFilePath);
    printf("%s,%d\n", __FILE__, __LINE__ );
    if(bufferPtr == NULL){
        printf("%s,%d\n", __FILE__, __LINE__ );
    }
    size_t length = GetMemoryFileSystem()->GetFileLength(StoreFilePath);
    printf("%s,%d\n", __FILE__, __LINE__ );

    return emscripten::val(
                emscripten::typed_memory_view(length, bufferPtr)
                );
    }


EMSCRIPTEN_BINDINGS(HuaHuoEngineV2_OBJECTSTORE) {
    emscripten::function("writeObjectStoreInMemoryFile", &writeObjectStoreInMemoryFile);
}
#endif

