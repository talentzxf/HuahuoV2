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
        ,mStoreId(0)
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

    void UpdateMaxFrameId(int frameId, bool force = false){
        if(force)
            this->maxFrameId = frameId;
        else
            this->maxFrameId = std::max(this->maxFrameId, frameId);

        GetScriptEventManager()->TriggerEvent("OnMaxFrameIdUpdated", NULL);
    }

    int GetMaxFrameId(){
        return this->maxFrameId;
    }

    void SyncLayersInfo(){
        maxFrameId = -1;
        for(PPtr<Layer> layer : layers){
            layer->SyncInfo();
            for(auto shape: layer->GetShapes()){
                if(shape->GetMaxFrameId() > maxFrameId)
                    maxFrameId = shape->GetMaxFrameId();
            }
        }
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
        : Super(label, mode)
        , m_IsGlobal(false)
        , canvasWidth(-1)
        , canvasHeight(-1)
        , maxKeyFrameIdentifier(0)
    {
        printf("Creating new store manager!!!!\n");
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
            printf("New store created, current storeId:%d\n", currentStore->GetStoreId());
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

        printf("CurrentStore instance id:%d at file:%s\n", currentStore.GetInstanceID(), StoreFilePath.c_str());
        return currentStore;
    }

    ObjectStore* GetStoreById(UInt32 storeId){
        if(allStores.size() < storeId){
            printf("StoreId:%d not found\n", storeId);
            return NULL;
        }

        return allStores[storeId -1];
    }

    bool SetDefaultStoreByIndex(UInt32 index){
        if(allStores.size() < index || index <=0){
            printf("StoreId:%d not found or is invalid\n", index);
            return false;
        }

//        printf("Setting default store:%d\n", index);
        currentStore = allStores[index - 1];

        return true;
    }

    ObjectStore* GetStoreByIndex(UInt32 index){
        if(allStores.size() < index){
            printf("StoreId:%d not found\n", index);
            return NULL;
        }

        return allStores[index - 1];
    }

    static ObjectStoreManager* GetDefaultObjectStoreManager();

    void AwakeFromLoad(AwakeFromLoadMode awakeMode) override;

    void SetStoreFilePath(char* inStoreFilePath);
    char* GetStoreFilePath();

    void SetCanvasWH(int canvasWidth, int canvasHeight){
        this->canvasWidth = canvasWidth;
        this->canvasHeight = canvasHeight;
    }

    int GetCanvasWidth(){
        return this->canvasWidth;
    }

    int GetCanvasHeight(){
        return this->canvasHeight;
    }

    KeyframeIdentifier ProduceKeyFrame(){
        allKeyFrames[maxKeyFrameIdentifier] = KeyFrame();
        return maxKeyFrameIdentifier++;
    }

    KeyFrame& GetKeyFrameById(KeyframeIdentifier keyFrameId){
        return allKeyFrames[keyFrameId];
    }

private:
    std::vector<PPtr<ObjectStore>> allStores;
    KeyframeIdentifier maxKeyFrameIdentifier; // This is NOT the frameId of the keyframes. It's just an ID for all the KeyFrame objects.
    std::map<KeyframeIdentifier, KeyFrame> allKeyFrames; // All key frames in the store. Map from keyframe identifier to keyframe object.

    PPtr<ObjectStore> currentStore;
    int canvasWidth;
    int canvasHeight;
};

ObjectStoreManager* GetDefaultObjectStoreManager();
void SetDefaultObjectStoreManager(ObjectStoreManager* storeManager);

#if WEB_ENV
#include <emscripten/bind.h>
emscripten::val writeObjectStoreInMemoryFile();

#endif

#endif //HUAHUOENGINEV2_OBJECTSTORE_H
