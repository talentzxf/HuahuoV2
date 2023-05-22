//
// Created by VincentZhang on 6/1/2022.
//

#include "ObjectStore.h"
#include "Memory/MemoryMacros.h"
#include "Serialize/SerializeUtility.h"

ObjectStoreManager *gDefaultObjectStoreManager = NULL;

ObjectStoreManager *GetDefaultObjectStoreManager() {
    if (gDefaultObjectStoreManager == NULL) {
        gDefaultObjectStoreManager = Object::Produce<ObjectStoreManager>();
        gDefaultObjectStoreManager->SetIsGlobal(true);

        GetPersistentManager().MakeObjectPersistent(gDefaultObjectStoreManager->GetInstanceID(), StoreFilePath);
    }

    return gDefaultObjectStoreManager;
}

void SetDefaultObjectStoreManager(ObjectStoreManager *objectStoreManager) {
    gDefaultObjectStoreManager = objectStoreManager;

    printf("Set Default object store manager\n");
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
    TRANSFER(canvasWidth);
    TRANSFER(canvasHeight);
    TRANSFER(maxKeyFrameIdentifier);
    TRANSFER(allKeyFrames);
}

ObjectStoreManager *ObjectStoreManager::GetDefaultObjectStoreManager() {
    return ::GetDefaultObjectStoreManager();
}

void ObjectStoreManager::AwakeFromLoad(AwakeFromLoadMode awakeMode) {
    Super::AwakeFromLoad(awakeMode);

    if (this->m_IsGlobal) {
        SetDefaultObjectStoreManager(this);

        size_t layerCount = GetCurrentStore()->GetLayerCount();
        for (int i = 0; i < layerCount; i++) {
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

    TRANSFER(layerMap);
    TRANSFER(layers);
    TRANSFER(currentLayer);
    TRANSFER(maxFrameId);
    TRANSFER(mStoreId);
    TRANSFER(mIsRoot);
}

void ObjectStore::AwakeFromLoad(AwakeFromLoadMode awakeMode) {
    printf("Object store awake!!!");

    if(!GetDefaultObjectStoreManager()->HasStore(this)){
        GetDefaultObjectStoreManager()->AddStore(this);

        if(this->GetIsRoot()){
            ObjectStoreAddedEvent args(this);
            GetScriptEventManager()->TriggerEvent("OnStoreAdded", &args);
        }
    }
}

#if WEB_ENV
std::string getStoreFilePath(){
    return StoreFilePath;
}

void setStoreFilePath(std::string storeFilePath){
    StoreFilePath = storeFilePath;
}

emscripten::val writeAllObjectsInMemoryFile(){
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

emscripten::val writeObjectStoreInMemoryFile(std::string storeId){
    std::string filePath = StoreFilePath;
    ObjectStore* pStore = GetDefaultObjectStoreManager()->GetStoreById(storeId.c_str());
    int writeResult = GetPersistentManager().WriteObject(filePath, pStore);
    printf("%s,%d; file:%s\n writeResult:%d\n", __FILE__, __LINE__ , filePath.c_str(), writeResult);
    UInt8* bufferPtr = GetMemoryFileSystem()->GetDataPtr(filePath);
    if(bufferPtr == NULL){
        printf("Error: Buffer is NULL: %s,%d\n", __FILE__, __LINE__ );
    }

    size_t length = GetMemoryFileSystem()->GetFileLength(filePath);

    return emscripten::val(
            emscripten::typed_memory_view(length, bufferPtr)
            );
}


EMSCRIPTEN_BINDINGS(HuaHuoEngineV2_OBJECTSTORE) {
    emscripten::function("writeAllObjectsInMemoryFile", &writeAllObjectsInMemoryFile);
    emscripten::function("getStoreFilePath", &getStoreFilePath);
    emscripten::function("setStoreFilePath", &setStoreFilePath);
    emscripten::function("writeObjectStoreInMemoryFile", &writeObjectStoreInMemoryFile);
}
#endif

