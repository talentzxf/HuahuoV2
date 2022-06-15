//
// Created by VincentZhang on 6/15/2022.
//

#ifndef HUAHUOENGINEV2_LAYER_H
#define HUAHUOENGINEV2_LAYER_H
#include "TypeSystem/Object.h"
#include "Shapes/BaseShape.h"
#include "BaseClasses/PPtr.h"
#include "Serialize/PersistentManager.h"
#include "TimeLineCellManager.h"

#include <vector>

extern std::string StoreFilePath;

class Layer: public Object{
    REGISTER_CLASS(Layer);
    DECLARE_OBJECT_SERIALIZE();
public:
    Layer(MemLabelId label, ObjectCreationMode mode)
            :Super(label, mode), name("Unknown Layer")
    {
        cellManager = Object::Produce<TimeLineCellManager>();
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

    void AwakeAllShapes(AwakeFromLoadMode awakeFromLoadMode);

    TimeLineCellManager* GetTimeLineCellManager(){
        return cellManager;
    }

private:
    ShapePPtrVector shapes;
    std::string name;
    PPtr<TimeLineCellManager> cellManager;
};

#endif //HUAHUOENGINEV2_LAYER_H
