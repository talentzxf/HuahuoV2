//
// Created by VincentZhang on 6/15/2022.
//

#include "Layer.h"

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
}

void Layer::AwakeAllShapes(AwakeFromLoadMode awakeFromLoadMode){
    for( ShapePPtrVector::iterator itr = shapes.begin(); itr != shapes.end(); itr++){
        (*itr)->SetLayer(this);
    }
}