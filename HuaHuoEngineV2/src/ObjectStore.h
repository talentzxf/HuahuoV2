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
    {

    }

    Layer* CreateLayer(const char* uuid){
        printf("Creating layer for uuid:%s\n", uuid);
        Layer* layer = Object::Produce<Layer>();
        currentLayer = layer;
        layerMap.insert(std::pair<std::string, PPtr<Layer>>(uuid, layer));
        layers.push_back(layer);
        GetPersistentManager().MakeObjectPersistent(layer->GetInstanceID(), StoreFilePath);

        return layer;
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

private:
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
            currentStore = Object::Produce<ObjectStore>();
            printf("%s,%d\n", __FILE__, __LINE__);
            GetPersistentManager().MakeObjectPersistent(currentStore.GetInstanceID(), StoreFilePath);
            printf("%s,%d\n", __FILE__, __LINE__);
            allStores.push_back(currentStore);
            printf("%s,%d\n", __FILE__, __LINE__);
        }
        printf("Return of current store\n");
        return currentStore;
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
