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
#include "Shapes/ElementShape.h"
#include "KeyFrames/KeyFrame.h"

#include <vector>

extern std::string StoreFilePath;

class ObjectStore;

class LayerUpdatedEventHandlerArgs: public ScriptEventHandlerArgs{
public:
    LayerUpdatedEventHandlerArgs(Layer *layer) {
        this->layer = layer;
    }

    Layer *GetLayer() {
        return this->layer;
    }

private:
    Layer *layer;
};

class KeyFrameChangedEventHandlerArgs : public ScriptEventHandlerArgs {
public:
    KeyFrameChangedEventHandlerArgs(Layer *layer, int frameId) {
        this->layer = layer;
        this->frameId = frameId;
    }

    Layer *GetLayer() {
        return this->layer;
    }

    int GetFrameId() {
        return this->frameId;
    }

private:
    Layer *layer;
    int frameId;
};

class ShapeRemovedEventHandlerArgs : public ScriptEventHandlerArgs{
public:
    ShapeRemovedEventHandlerArgs(Layer* layer, BaseShape* baseShape){
        this->layer = layer;
        this->baseShape = baseShape;
    }

    Layer* GetLayer(){
        return this->layer;
    }

    BaseShape* GetShape(){
        return this->baseShape;
    }

private:
    Layer* layer;
    BaseShape* baseShape;
};


class Layer : public Object {
REGISTER_CLASS(Layer);

DECLARE_OBJECT_SERIALIZE();
public:
    Layer(MemLabelId label, ObjectCreationMode mode)
            : Super(label, mode), name("Unknown Layer"), currentFrameId(0), isVisible(true), isSelected(false) {
    }

    typedef std::vector<PPtr<BaseShape>> ShapePPtrVector;

    void AddShapeInternal(BaseShape *newShape);

    void Init() {
        cellManager = Object::Produce<TimeLineCellManager>();
        GetPersistentManagerPtr()->MakeObjectPersistent(cellManager.GetInstanceID(), StoreFilePath);
        this->cellManager->SetLayer(this);
    }

    void SetObjectStore(const ObjectStore *store);

    virtual void SetName(const char *name) override {
        this->name = name;
    }

    virtual const char *GetName() const override {
        return this->name.c_str();
    }

    void SetCurrentFrame(int currentFrameId) {
        this->currentFrameId = currentFrameId;
        for (auto shape: shapes) {
//            // If we update an element shape, it will calculate it's local frameId.
//            // No need to update along with other shapes in the layer.
//            if(!shape->GetType()->IsDerivedFrom<ElementShape>()){
                shape->Apply(this->currentFrameId);
//            }
        }
    }

    int GetCurrentFrame() {
        return currentFrameId;
    }

    size_t GetShapeCount() {
        return shapes.size();
    }

    BaseShape *GetShapeAtIndex(size_t shapeIdx) {
        if(shapeIdx >= shapes.size())
            return NULL;

        return shapes[shapeIdx];
    }

    ShapePPtrVector &GetShapes() {
        return shapes;
    }

    void AwakeAllShapes(AwakeFromLoadMode awakeFromLoadMode);

    TimeLineCellManager *GetTimeLineCellManager() {
        return cellManager;
    }

    // void AddKeyFrame(int frameId, AbstractFrameState* keyFrame);
    void AddKeyFrame(KeyFrame* keyFrame);

    void DeleteKeyFrame(KeyFrame* keyFrame, bool notifyFrontEnd = true);

    bool IsKeyFrame(int frameId) {
        if (keyFrames.contains(frameId) && !keyFrames[frameId].empty())
            return true;

        return false;
    }

    ObjectStore *GetObjectStore();

    void SetIsVisible(bool isVisible);

    bool GetIsVisible() {
        return this->isVisible;
    }

    void SetIsSelected(bool isSelected){
        this->isSelected = isSelected;
    }

    bool GetIsSelected(){
        return this->isSelected;
    }

    void RemoveShape(BaseShape* shape);

    void SyncInfo();

    void MoveKeyFrameToKeyFrameId(KeyFrameIdentifier keyFrameIdentifier, int beforeFrameId, int afterFrameId);
private:
    bool InternalDeleteKeyFrame(KeyFrame* keyFrame);

private:
    // Frame Id-- starting from 0.
    int currentFrameId;
    ShapePPtrVector shapes;
    std::string name;
    PPtr<TimeLineCellManager> cellManager;

    // This is the keyframe identifier. Use this identifier we can get the real keyframe object stored in objectstoremanager.
    typedef std::vector<KeyFrameIdentifier> KeyFrameIdentifierSet;

    // Map from frameId -> Keyframe identifier set. Because each frameId might corresponds to different keyframe objects(identifiers).
    typedef std::map<int, KeyFrameIdentifierSet> KeyFrameIdentifierSetMap;
    KeyFrameIdentifierSetMap keyFrames;
    PPtr<ObjectStore> objectStore;
    bool isVisible;
    bool isSelected;
};

#endif //HUAHUOENGINEV2_LAYER_H
