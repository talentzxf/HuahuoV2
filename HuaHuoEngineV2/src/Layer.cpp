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

void Layer::AwakeAllShapes(AwakeFromLoadMode awakeFromLoadMode) {
    for (ShapePPtrVector::iterator itr = shapes.begin(); itr != shapes.end(); itr++) {
        (*itr)->SetLayer(this);
    }
}

void Layer::AddKeyFrame(int frameId) {
    if(keyFrames.contains(frameId))
        return;
    
    objectStore->UpdateMaxFrameId(frameId);
    keyFrames.insert(frameId);

    KeyFrameAddedEventHandlerArgs args(this, frameId);
    GetScriptEventManager()->TriggerEvent("OnKeyFrameAdded", &args);
}