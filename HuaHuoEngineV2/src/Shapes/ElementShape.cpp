//
// Created by VincentZhang on 7/1/2022.
//

#include "ElementShape.h"
#include "ObjectStore.h"
#include "Layer.h"

IMPLEMENT_REGISTER_CLASS(ElementShape, 10015);

IMPLEMENT_OBJECT_SERIALIZE(ElementShape);
INSTANTIATE_TEMPLATE_TRANSFER(ElementShape);

template<class TransferFunction>
void ElementShape::Transfer(TransferFunction& transfer){
    Super::Transfer(transfer);
    TRANSFER(mStoreId);
}

void ElementShape::Apply(int frameId) {
    if(this->mStoreId < 0){
        printf("Store:%d is invalid.\n", this->mStoreId);
        return;
    }

    Super::Apply(frameId);

    ObjectStore* pStore = GetDefaultObjectStoreManager()->GetStoreByIndex(this->mStoreId);
    size_t layerCount = pStore->GetLayerCount();
    for(size_t layerId = 0 ; layerId < layerCount; layerId++){
        Layer* pLayer = pStore->GetLayer(layerId);
        auto shapes = pLayer->GetShapes();
        for(auto shape : shapes){
            shape->Apply(frameId);
        }
    }
}