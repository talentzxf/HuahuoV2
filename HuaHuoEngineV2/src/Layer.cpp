//
// Created by VincentZhang on 6/15/2022.
//

#include "Layer.h"
#include "ObjectStore.h"

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

    LayerUpdatedEventHanderArgs args(this);
    GetScriptEventManager()->TriggerEvent("OnLayerUpdated", &args);
}

void Layer::AddKeyFrame(int frameId) {
    if (keyFrames.contains(frameId))
        return;

    objectStore->UpdateMaxFrameId(frameId);
    keyFrames.insert(frameId);

    KeyFrameAddedEventHandlerArgs args(this, frameId);
    GetScriptEventManager()->TriggerEvent("OnKeyFrameAdded", &args);
}

void Layer::SetObjectStore(ObjectStore *store) {
    this->objectStore = store;
}