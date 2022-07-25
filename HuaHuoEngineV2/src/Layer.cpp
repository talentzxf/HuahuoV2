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

void Layer::RemoveShape(BaseShape* shape){
    long instanceId = shape->GetInstanceID();
    long index = 0;
    for(;index < shapes.size(); index++){
        if(shapes[index].GetInstanceID() == instanceId){
            break;
        }
    }

    if(index == shapes.size()){
        printf("Can't find the shape in the layer\n");
        return;
    }

    shapes.erase(shapes.begin() + index);

    std::vector<int> toDeleteFrames;
    // Remove the shape from keyframes.
    for(auto keyframe : keyFrames){
        if(keyframe.second.contains(shape)){
            printf("Erasing shape!!!!\n");
            keyframe.second.erase(shape);
        }

        if(keyframe.second.empty()){
            printf("Begin to erase keyframe!!!!\n");
            toDeleteFrames.push_back(keyframe.first);
        }
    }

    for(int toDeleteFrameId: toDeleteFrames){
        printf("Erased keyframe:%d\n", toDeleteFrameId);
        keyFrames.erase(toDeleteFrameId);

        KeyFrameChangedEventHandlerArgs args(this, toDeleteFrameId);
        GetScriptEventManager()->TriggerEvent("OnKeyFrameChanged", &args);
    }

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

void Layer::AddKeyFrame(int frameId, BaseShape* shape) {
    if(keyFrames.contains(frameId) && keyFrames[frameId].contains(shape)){
        return;
    }

    if (!keyFrames.contains(frameId)){
        keyFrames.insert(std::pair<int, std::set<PPtr<BaseShape>>>(frameId, set<PPtr<BaseShape>>()));
    }

    objectStore->UpdateMaxFrameId(frameId);
    std::set<PPtr<BaseShape>>& shapeSet = keyFrames[frameId];
    shapeSet.insert(shape);

    KeyFrameChangedEventHandlerArgs args(this, frameId);
    GetScriptEventManager()->TriggerEvent("OnKeyFrameChanged", &args);
}

void Layer::SetObjectStore(ObjectStore *store) {
    this->objectStore = store;
}