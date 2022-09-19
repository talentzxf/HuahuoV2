//
// Created by VincentZhang on 6/1/2022.
//

#include "ObjectStore.h"
#include "Memory/MemoryMacros.h"
#include "Serialize/SerializeUtility.h"
#include <ctime>
#include <sstream>

std::string StoreFilePath("mem://objectstore.data");

ObjectStoreManager* gDefaultObjectStoreManager = NULL;

const std::string currentTime(){
    std::time_t timeStamp = std::time(nullptr);
    std::stringstream ss;
    ss << timeStamp;
    return ss.str();
}

std::string gen_random(const int len) {
    static const char alphanum[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
    std::string tmp_s;
    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i) {
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    return tmp_s;
}

ObjectStoreManager* GetDefaultObjectStoreManager(){
    if(gDefaultObjectStoreManager == NULL){
        gDefaultObjectStoreManager = Object::Produce<ObjectStoreManager>();
        gDefaultObjectStoreManager->SetIsGlobal(true);

        StoreFilePath = "mem://" + gen_random(10) + currentTime();

        GetPersistentManager().MakeObjectPersistent(gDefaultObjectStoreManager->GetInstanceID(), StoreFilePath);
    }

    return gDefaultObjectStoreManager;
}

void SetDefaultObjectStoreManager(ObjectStoreManager* objectStoreManager){
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

