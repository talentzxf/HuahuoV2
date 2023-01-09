//
// Created by VincentZhang on 6/15/2022.
//

#include "Layer.h"
#include "ObjectStore.h"
#include <map>

IMPLEMENT_REGISTER_CLASS(Layer, 10001);

IMPLEMENT_OBJECT_SERIALIZE(Layer);

INSTANTIATE_TEMPLATE_TRANSFER(Layer);

template<class TransferFunction>
void Layer::Transfer(TransferFunction &transfer) {
    Super::Transfer(transfer);
    TRANSFER(name);
    TRANSFER(shapes);
    TRANSFER(cellManager);
    TRANSFER(keyFrames);
    TRANSFER(objectStore);
}

ObjectStore *Layer::GetObjectStore() {
    return objectStore;
}

void Layer::RemoveShape(BaseShape *shape) {
    long instanceId = shape->GetInstanceID();
    long index = 0;
    for (; index < shapes.size(); index++) {
        if (shapes[index].GetInstanceID() == instanceId) {
            break;
        }
    }

    if (index == shapes.size()) {
        printf("Can't find the shape in the layer\n");
        return;
    }

    shapes.erase(shapes.begin() + index);

    std::vector<int> toDeleteFrames;
    // Remove the shape from keyframes.
    for (auto keyframe: keyFrames) {

        std::erase_if(keyframe.second, [shape](KeyFrameIdentifier keyframeIdentifier){
            KeyFrame& keyFrameObj = GetDefaultObjectStoreManager()->GetKeyFrameById(keyframeIdentifier);
            return keyFrameObj.GetBaseShape()->GetInstanceID() == shape->GetInstanceID();
        });

        if (keyframe.second.empty()) {
            printf("Begin to erase keyframe!!!!\n");
            toDeleteFrames.push_back(keyframe.first);
        }
    }

    for (int toDeleteFrameId: toDeleteFrames) {
        keyFrames.erase(toDeleteFrameId);

        KeyFrameChangedEventHandlerArgs args(this, toDeleteFrameId);
        GetScriptEventManager()->TriggerEvent("OnKeyFrameChanged", &args);
    }

    shape->SetLayer(NULL);

    // Inform the script to remove the shape in JS side.
    ShapeRemovedEventHandlerArgs args(this, shape);
    GetScriptEventManager()->TriggerEvent("OnShapeRemoved", &args);
}

void Layer::AwakeAllShapes(AwakeFromLoadMode awakeFromLoadMode) {
    for (ShapePPtrVector::iterator itr = shapes.begin(); itr != shapes.end(); itr++) {
        (*itr)->SetLayer(this);
    }
}

void Layer::SetIsVisible(bool isVisible) {
    this->isVisible = isVisible;

    // Hide all shapes in this layer.
    for (auto shape: shapes) {
        shape->SetIsVisible(isVisible);
    }

    LayerUpdatedEventHandlerArgs args(this);
    GetScriptEventManager()->TriggerEvent("OnLayerUpdated", &args);
}

void Layer::AddKeyFrame(KeyFrame* keyFrame) {
    int frameId = keyFrame->GetFrameId();
    AbstractFrameState* frameState = keyFrame->GetFrameState();
    if (keyFrames.contains(frameId) && frameState != NULL) {
        auto keyFrameSet = keyFrames[frameId];
        auto foundKeyFrameObjItr = std::find_if(keyFrameSet.begin(), keyFrameSet.end(),[frameState](const int keyFrameIdentifier){
            KeyFrame& keyFrameObject = GetDefaultObjectStoreManager()->GetKeyFrameById(keyFrameIdentifier);
            if(keyFrameObject.GetFrameState() == NULL || !keyFrameObject.GetFrameState()->IsValid())
                return false;

            return keyFrameObject.GetFrameState()->GetInstanceID() == frameState->GetInstanceID();
        });
        if(foundKeyFrameObjItr != keyFrameSet.end()) // frameId and frameState both match. No need to add again.
            return;
    }

    if (!keyFrames.contains(frameId)) {
        keyFrames.insert(std::pair<KeyFrameIdentifier, KeyFrameIdentifierSet>(frameId, KeyFrameIdentifierSet()));
    }

    keyFrames.find(frameId)->second.push_back(keyFrame->GetKeyFrameIdentifier());
    if (this->GetObjectStore() != NULL) {
        this->GetObjectStore()->UpdateMaxFrameId(frameId);
    }

    BaseShape* shape = keyFrame->GetBaseShape();
    shape->RefreshKeyFrameCache();

    KeyFrameChangedEventHandlerArgs args(this, frameId);
    GetScriptEventManager()->TriggerEvent("OnKeyFrameChanged", &args);
}

void Layer::DeleteKeyFrame(KeyFrame *keyFrame) {
    int frameId = keyFrame->GetFrameId();
    AbstractFrameState* frameState = keyFrame->GetFrameState();
    if (keyFrames.contains(frameId) && frameState != NULL) {
        auto keyFrameSet = keyFrames[frameId];
        auto foundKeyFrameObjItr = std::find_if(keyFrameSet.begin(), keyFrameSet.end(),[frameState](const int keyFrameIdentifier){
            KeyFrame& keyFrameObject = GetDefaultObjectStoreManager()->GetKeyFrameById(keyFrameIdentifier);
            if(keyFrameObject.GetFrameState() == NULL || !keyFrameObject.GetFrameState()->IsValid())
                return false;

            return keyFrameObject.GetFrameState()->GetInstanceID() == frameState->GetInstanceID();
        });
        if(foundKeyFrameObjItr != keyFrameSet.end()) // frameId and frameState both match. No need to add again.
            return;
    }

    if (!keyFrames.contains(frameId)) {
        keyFrames.insert(std::pair<KeyFrameIdentifier, KeyFrameIdentifierSet>(frameId, KeyFrameIdentifierSet()));
    }

    keyFrames.find(frameId)->second.push_back(keyFrame->GetKeyFrameIdentifier());
    if (this->GetObjectStore() != NULL) {
        this->GetObjectStore()->UpdateMaxFrameId(frameId);
    }

    BaseShape* shape = keyFrame->GetBaseShape();
    shape->RefreshKeyFrameCache();

    KeyFrameChangedEventHandlerArgs args(this, frameId);
    GetScriptEventManager()->TriggerEvent("OnKeyFrameChanged", &args);
}

void Layer::SetObjectStore(const ObjectStore *store) {
    this->objectStore = store;
}

void Layer::AddShapeInternal(BaseShape *newShape) {
    newShape->SetLayer(this);

    if (newShape->GetBornFrameId() < 0) // If the born frame has been set already, keep it. This might happen if a shape is moved from one layer to another layer.
        newShape->SetBornFrameId(this->currentFrameId);

    shapes.push_back(newShape);

    GetPersistentManager().MakeObjectPersistent(newShape->GetInstanceID(), StoreFilePath);

    // Update the max length of the animation
    int maxFrameId = newShape->GetMaxFrameId();
    this->objectStore->UpdateMaxFrameId(maxFrameId);

    if (maxFrameId >= 0) {
        // Merge from 0 to current maxFrameId
        this->GetTimeLineCellManager()->MergeCells(0, maxFrameId);
    }
}

void Layer::SyncInfo() {
    keyFrames.clear();
    for (auto shapePtr: this->shapes) {
        shapePtr->RefreshKeyFrameCache();
        int keyFrameCount = shapePtr->GetKeyFrameCount();
        for (int keyFrameIdx = 0; keyFrameIdx < keyFrameCount; keyFrameIdx++) {
            KeyFrame* keyFrame = shapePtr->GetKeyFrameObjectAtIdx(keyFrameIdx);
            this->AddKeyFrame(keyFrame);
        }
    }
}