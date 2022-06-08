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
    }

    return gDefaultObjectStoreManager;
}

IMPLEMENT_REGISTER_CLASS(ObjectStoreManager, 10004);

IMPLEMENT_OBJECT_SERIALIZE(ObjectStoreManager);
INSTANTIATE_TEMPLATE_TRANSFER(ObjectStoreManager);
template<class TransferFunction>
void ObjectStoreManager::Transfer(TransferFunction &transfer) {
    Super::Transfer(transfer);
    printf("Writing allStores:%d\n", allStores.size());
    TRANSFER(allStores);
    printf("Writing current store:%d\n", allStores.size());
    TRANSFER(currentStore);
}

ObjectStoreManager* ObjectStoreManager::GetDefaultObjectStoreManager(){
    return ::GetDefaultObjectStoreManager();
}

IMPLEMENT_REGISTER_CLASS(ObjectStore, 10000);

IMPLEMENT_OBJECT_SERIALIZE(ObjectStore);
INSTANTIATE_TEMPLATE_TRANSFER(ObjectStore);

template<class TransferFunction>
void ObjectStore::Transfer(TransferFunction &transfer) {
    Super::Transfer(transfer);

    printf("Writing layermap:%d\n", layerMap.size());
    TRANSFER(layerMap);

    printf("Writing currentlayer\n");
    TRANSFER(currentLayer);
}


IMPLEMENT_REGISTER_CLASS(Layer, 10001);

IMPLEMENT_OBJECT_SERIALIZE(Layer);
INSTANTIATE_TEMPLATE_TRANSFER(Layer);

template<class TransferFunction>
void Layer::Transfer(TransferFunction &transfer) {
    Super::Transfer(transfer);

    printf("Transfering layername:%s\n", name.c_str());
    TRANSFER(name);
    printf("Writing shapes:%d\n", shapes.size());
    TRANSFER(shapes);
}

