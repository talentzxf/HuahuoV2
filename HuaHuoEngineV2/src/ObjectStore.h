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

extern std::string StoreFilePath;

class Layer: public Object{
    REGISTER_CLASS(Layer);
    DECLARE_OBJECT_SERIALIZE();
public:
    Layer(MemLabelId label, ObjectCreationMode mode)
        :Super(label, mode), name("Unknown Layer")
    {
    }

    typedef std::vector<PPtr<BaseShape>> ShapePPtrVector;

    void AddShapeInternal(BaseShape* newShape){
        shapes.push_back(newShape);

        GetPersistentManager().MakeObjectPersistent(newShape->GetInstanceID(), StoreFilePath);
    }

    virtual void SetName(const char* name) override{
        this->name = name;
    }

    virtual const char* GetName() const override{
        return this->name.c_str();
    }

    size_t GetShapeCount(){
        return shapes.size();
    }

    ShapePPtrVector& GetShapes(){
        return shapes;
    }

private:
    ShapePPtrVector shapes;
    std::string name;
};

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
        GetPersistentManager().MakeObjectPersistent(layer->GetInstanceID(), StoreFilePath);

        return layer;
    }

    Layer* GetCurrentLayer(){
        return currentLayer;
    }

    size_t GetLayerCount(){
        return layerMap.size();
    }

private:
    std::map<std::string, PPtr<Layer>> layerMap;
    PPtr<Layer> currentLayer;
};

class ObjectStoreManager: public Object{
    REGISTER_CLASS(ObjectStoreManager);
    DECLARE_OBJECT_SERIALIZE();
public:
    ObjectStoreManager(MemLabelId label, ObjectCreationMode mode)
        :Super(label, mode)
    {
    }

    ObjectStore* GetCurrentStore(){
        if(!currentStore.IsValid()){
            printf("currentStore invalid, creating new store\n");
            currentStore = Object::Produce<ObjectStore>();
            GetPersistentManager().MakeObjectPersistent(currentStore.GetInstanceID(), StoreFilePath);
            allStores.push_back(currentStore);
        }
        printf("Return of current store\n");
        return currentStore;
    }

    static ObjectStoreManager* GetDefaultObjectStoreManager();

private:
    std::vector<PPtr<ObjectStore>> allStores;
    PPtr<ObjectStore> currentStore;
};

ObjectStoreManager* GetDefaultObjectStoreManager();



#endif //HUAHUOENGINEV2_OBJECTSTORE_H
