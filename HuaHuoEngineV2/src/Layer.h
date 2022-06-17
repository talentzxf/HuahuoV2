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
            :Super(label, mode), name("Unknown Layer"),currentFrameId(0)
    {
        cellManager = Object::Produce<TimeLineCellManager>();
        GetPersistentManagerPtr()->MakeObjectPersistent(cellManager.GetInstanceID(), StoreFilePath);
    }

    typedef std::vector<PPtr<BaseShape>> ShapePPtrVector;

    void AddShapeInternal(BaseShape* newShape){
        newShape->SetLayer(this);
        shapes.push_back(newShape);

        GetPersistentManager().MakeObjectPersistent(newShape->GetInstanceID(), StoreFilePath);
    }

    virtual void SetName(const char* name) override{
        this->name = name;
    }

    virtual char* GetName() const override{
        return const_cast<char*>(this->name.c_str());
    }

    void SetCurrentFrame(int currentFrameId){
        if(this->currentFrameId == currentFrameId)
            return;

        printf("Set current frame Id to:%d\n", currentFrameId);
        this->currentFrameId = currentFrameId;
        for(auto shape : shapes){
            shape->Apply(this->currentFrameId);
        }
    }

    int GetCurrentFrame(){
        return currentFrameId;
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
    // Frame Id-- starting from 0.
    int currentFrameId;
    ShapePPtrVector shapes;
    std::string name;
    PPtr<TimeLineCellManager> cellManager;
};

#endif //HUAHUOENGINEV2_LAYER_H
