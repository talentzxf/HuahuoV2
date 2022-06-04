//
// Created by VincentZhang on 6/1/2022.
//

#include "ObjectStore.h"
#include "Memory/MemoryMacros.h"
#include "Serialize/SerializeUtility.h"

std::string StoreFilePath("C:\\Users\\vincentzhang\\MyProjects\\HuahuoV2\\HuaHuoEngineV2\\cmake-build-debug\\tmp\\objectstore.data");

ObjectStoreManager* gDefaultObjectStoreManager = NULL;

ObjectStoreManager* GetDefaultObjectStoreManager(){
    if(gDefaultObjectStoreManager == NULL)
        gDefaultObjectStoreManager = Object::Produce<ObjectStoreManager>();
    return gDefaultObjectStoreManager;
}

IMPLEMENT_REGISTER_CLASS(ObjectStoreManager, 10004);

IMPLEMENT_OBJECT_SERIALIZE(ObjectStoreManager);
INSTANTIATE_TEMPLATE_TRANSFER(ObjectStoreManager);
template<class TransferFunction>
void ObjectStoreManager::Transfer(TransferFunction &transfer) {
    Super::Transfer(transfer);
    TRANSFER(allStores);
    TRANSFER(currentStore);
}

IMPLEMENT_REGISTER_CLASS(ObjectStore, 10000);

IMPLEMENT_OBJECT_SERIALIZE(ObjectStore);
INSTANTIATE_TEMPLATE_TRANSFER(ObjectStore);

template<class TransferFunction>
void ObjectStore::Transfer(TransferFunction &transfer) {
    Super::Transfer(transfer);
    TRANSFER(layerMap);
    TRANSFER(currentLayer);
}


IMPLEMENT_REGISTER_CLASS(Layer, 10001);

IMPLEMENT_OBJECT_SERIALIZE(Layer);
INSTANTIATE_TEMPLATE_TRANSFER(Layer);

template<class TransferFunction>
void Layer::Transfer(TransferFunction &transfer) {
    Super::Transfer(transfer);
    TRANSFER(shapes);
}