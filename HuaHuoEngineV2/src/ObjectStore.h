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
#include "ResourceManager.h"

extern std::string StoreFilePath;

class ObjectStore : public Object{
    REGISTER_CLASS(ObjectStore);
    DECLARE_OBJECT_SERIALIZE();
public:
    ObjectStore(MemLabelId label, ObjectCreationMode mode)
        :Super(label, mode)
        ,maxFrameId(-1)
        ,mIsRoot(false)
    {
        mStoreId.Init();
    }

    const char* GetStoreId(){
        storeIdString = GUIDToString(this->mStoreId);
        return storeIdString.c_str();
    }

    HuaHuoGUID GetStoreGuid(){
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

    void AwakeFromLoad(AwakeFromLoadMode awakeMode) override;

    bool GetIsRoot(){
        return mIsRoot;
    }

    void SetIsRoot(bool isRoot){
        mIsRoot = isRoot;
    }

private:
    HuaHuoGUID mStoreId;
    int maxFrameId;
    std::vector<PPtr<Layer>> layers;
    std::map<std::string, PPtr<Layer>> layerMap;
    PPtr<Layer> currentLayer;
    std::string storeIdString;
    bool mIsRoot; // This flag will be set when element authors mark uploaded the element and cleared when it's uploaded under another root.
};

class ObjectStoreAddedEvent : public ScriptEventHandlerArgs{
public:
    ObjectStoreAddedEvent(ObjectStore* pStore){
        this->mStore = pStore;
    }

    ObjectStore* GetStore(){
        return mStore;
    }

private:
    ObjectStore* mStore;
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
            printf("New store created, current storeId:%s\n", currentStore->GetStoreId());
        }
        return currentStore;
    }

    bool HasStore(ObjectStore* objectStore){
        return allStores.contains(objectStore->GetStoreGuid());
    }

    void AddStore(ObjectStore* objectStore){
        allStores[objectStore->GetStoreGuid()] = objectStore;
    }

    // After creating a store, it will be set as the default store.
    ObjectStore* CreateStore(){
        currentStore = Object::Produce<ObjectStore>();
        GetPersistentManager().MakeObjectPersistent(currentStore.GetInstanceID(), StoreFilePath);
        allStores[currentStore->GetStoreGuid()] = currentStore;

        return currentStore;
    }

    ObjectStore* GetStoreByGUID(HuaHuoGUID storeGuid){
        if(!allStores.contains(storeGuid)){
            return NULL;
        }

        return allStores[storeGuid];
    }

    ObjectStore* GetStoreById(const char * storeId){
        HuaHuoGUID storeGuid = StringToGUID(storeId);
        return GetStoreByGUID(storeGuid);
    }

    bool SetDefaultStoreByIndex(const char * storeId){
        HuaHuoGUID storeGuid = StringToGUID(storeId);
        if(!allStores.contains(storeGuid)){
            printf("StoreId:%s not found or is invalid\n", storeId);
            printf("Printing current stores:");
            for(auto storeId : allStores){
                printf("%s,", GUIDToString(storeId.first).c_str());
            }
            printf("\n");
            return false;
        }

        currentStore = allStores[storeGuid];

        return true;
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

    KeyFrameIdentifier ProduceKeyFrame(){
        allKeyFrames[maxKeyFrameIdentifier] = KeyFrame();
        allKeyFrames[maxKeyFrameIdentifier].SetKeyFrameIdentifier(maxKeyFrameIdentifier);
        return maxKeyFrameIdentifier++;
    }

    KeyFrame& GetKeyFrameById(KeyFrameIdentifier keyFrameId){
        return allKeyFrames[keyFrameId];
    }

private:
    // std::vector<PPtr<ObjectStore>> allStores;
    typedef std::map<HuaHuoGUID, PPtr<ObjectStore>> GUIDObjectStoreMap;
    GUIDObjectStoreMap allStores;

    KeyFrameIdentifier maxKeyFrameIdentifier; // This is NOT the frameId of the keyframes. It's just an ID for all the KeyFrame objects.
    std::map<KeyFrameIdentifier, KeyFrame> allKeyFrames; // All key frames in the store. Map from keyframe identifier to keyframe object.

    PPtr<ObjectStore> currentStore;
    int canvasWidth;
    int canvasHeight;
};

ObjectStoreManager* GetDefaultObjectStoreManager();
void SetDefaultObjectStoreManager(ObjectStoreManager* storeManager);

#if WEB_ENV
#include <emscripten/bind.h>
emscripten::val writeAllObjectsInMemoryFile();

#endif

#endif //HUAHUOENGINEV2_OBJECTSTORE_H
