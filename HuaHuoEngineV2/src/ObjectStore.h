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
        :Super(label, mode)
    {

    }

    typedef std::vector<PPtr<BaseShape>> ShapePPtrVector;

    void addShape(BaseShape* newShape){
        shapes.push_back(newShape);

        GetPersistentManager().MakeObjectPersistent(newShape->GetInstanceID(), StoreFilePath);
    }

    ShapePPtrVector& GetShapes(){
        return shapes;
    }

private:
    ShapePPtrVector shapes;
};

class ObjectStore : public Object{
    REGISTER_CLASS(ObjectStore);
    DECLARE_OBJECT_SERIALIZE();
public:
    ObjectStore(MemLabelId label, ObjectCreationMode mode)
        :Super(label, mode)
    {

    }

    void CreateLayer(const char* uuid){
        Layer* layer = Object::Produce<Layer>();
        currentLayer = layer;
        layerMap.insert(std::pair<std::string, PPtr<Layer>>(uuid, layer));
        GetPersistentManager().MakeObjectPersistent(layer->GetInstanceID(), StoreFilePath);
    }

    Layer* GetCurrentLayer(){
        return currentLayer;
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
            currentStore = Object::Produce<ObjectStore>();
            GetPersistentManager().MakeObjectPersistent(currentStore.GetInstanceID(), StoreFilePath);
            allStores.push_back(currentStore);
        }
        return currentStore;
    }

private:
    std::vector<PPtr<ObjectStore>> allStores;
    PPtr<ObjectStore> currentStore;
};

ObjectStoreManager* GetDefaultObjectStoreManager();



#endif //HUAHUOENGINEV2_OBJECTSTORE_H
