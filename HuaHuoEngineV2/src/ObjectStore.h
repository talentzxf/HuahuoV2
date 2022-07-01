//
// Created by VincentZhang on 6/1/2022.
//

#ifndef HUAHUOENGINEV2_OBJECTSTORE_H
#define HUAHUOENGINEV2_OBJECTSTORE_H
#include "TypeSystem/Type.h"
#include "TypeSystem/Object.h"
#include "TypeSystem/ObjectDefines.h"
#include "BaseClasses/PPtr.h"
#include "Shapes/BaseShape.h"
#include "Serialize/PersistentManager.h"
#include <vector>
#include <map>
#include <string>
#include "Utilities/MemoryFileSystem.h"
#include "Layer.h"

extern std::string StoreFilePath;

class ObjectStore : public Object{
    REGISTER_CLASS(ObjectStore);
    DECLARE_OBJECT_SERIALIZE();
public:
    ObjectStore(MemLabelId label, ObjectCreationMode mode)
        :Super(label, mode)
        ,maxFrameId(-1)
    {

    }

    void SetStoreId(UInt32 storeId){
        this->mStoreId = storeId;
    }

    UInt32 GetStoreId(){
        return this->mStoreId;
    }

    Layer* CreateLayer(const char* uuid){
        printf("Creating layer for uuid:%s\n", uuid);
        Layer* layer = Object::Produce<Layer>();
        layer->SetObjectStore(this);
        currentLayer = layer;
        layerMap.insert(std::pair<std::string, PPtr<Layer>>(uuid, layer));
        layers.push_back(layer);
        GetPersistentManager().MakeObjectPersistent(layer->GetInstanceID(), StoreFilePath);
        layer->Init();

        return layer;
    }

    void SetCurrentLayer(Layer* layer){
        this->currentLayer = layer;
    }

    Layer* GetCurrentLayer(){
        return currentLayer;
    }

    size_t GetLayerCount(){
        return layerMap.size();
    }

    Layer* GetLayer(int i){
        return layers[i];
    }

    void UpdateMaxFrameId(int frameId){
        this->maxFrameId = std::max(this->maxFrameId, frameId);
        printf("Update maxFrameId:%d\n", this->maxFrameId);
    }

    int GetMaxFrameId(){
        return this->maxFrameId;
    }

private:
    UInt32 mStoreId;
    int maxFrameId;
    std::vector<PPtr<Layer>> layers;
    std::map<std::string, PPtr<Layer>> layerMap;
    PPtr<Layer> currentLayer;
};

class ObjectStoreManager: public Object{
    REGISTER_CLASS(ObjectStoreManager);
    DECLARE_OBJECT_SERIALIZE();
private:
    bool m_IsGlobal;
public:
    ObjectStoreManager(MemLabelId label, ObjectCreationMode mode)
        :Super(label, mode)
        ,m_IsGlobal(false)
    {
    }

    void SetIsGlobal(bool isGlobal){
        m_IsGlobal = isGlobal;
    }

    bool IsGlobal(){
        return m_IsGlobal;
    }

    ObjectStore* GetCurrentStore(){
        if(!currentStore.IsValid()){
            printf("currentStore invalid, creating new store\n");
            CreateStore();
        }
        return currentStore;
    }

    // After creating a store, it will be set as the default store.
    ObjectStore* CreateStore(){
        UInt32 storeId = allStores.size() + 1;
        currentStore = Object::Produce<ObjectStore>();
        currentStore->SetStoreId(storeId);
        GetPersistentManager().MakeObjectPersistent(currentStore.GetInstanceID(), StoreFilePath);
        allStores.push_back(currentStore);
        return currentStore;
    }

    bool SetDefaultStoreByIndex(UInt32 index){
        if(allStores.size() < index){
            printf("StoreId:%d not found\n", index);
            return false;
        }

        currentStore = allStores[index - 1];

        return true;
    }

    static ObjectStoreManager* GetDefaultObjectStoreManager();

    void AwakeFromLoad(AwakeFromLoadMode awakeMode) override;

private:
    std::vector<PPtr<ObjectStore>> allStores;
    PPtr<ObjectStore> currentStore;
};

ObjectStoreManager* GetDefaultObjectStoreManager();
void SetDefaultObjectStoreManager(ObjectStoreManager* storeManager);

#if WEB_ENV
#include <emscripten/bind.h>
emscripten::val writeObjectStoreInMemoryFile();

#endif

#endif //HUAHUOENGINEV2_OBJECTSTORE_H
