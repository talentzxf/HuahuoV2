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

    printf("Transfering layername:%s\n", name.c_str());
    TRANSFER(name);
    printf("Writing shapes:%d\n", shapes.size());
    TRANSFER(shapes);

    printf("Writing cell mananger.\n");
    TRANSFER(cellManager);
}

void Layer::AwakeAllShapes(AwakeFromLoadMode awakeFromLoadMode){
    for( ShapePPtrVector::iterator itr = shapes.begin(); itr != shapes.end(); itr++){
        (*itr)->SetLayer(this);
    }
}