//
// Created by VincentZhang on 2022-06-16.
//

#include "FrameState.h"
#include "Shapes/BaseShape.h"

IMPLEMENT_REGISTER_CLASS(AbstractFrameState, 10007);

IMPLEMENT_OBJECT_SERIALIZE(AbstractFrameState);

template<class TransferFunction>
void AbstractFrameState::Transfer(TransferFunction &transfer) {
    Super::Transfer(transfer);

    if (transfer.IsWriting()) {
        mBaseShapePPtr = baseShape;
        TRANSFER(mBaseShapePPtr);
    } else {
        TRANSFER(mBaseShapePPtr);
    }

    TRANSFER(typeName);
    TRANSFER(frameStateName);
}

void AbstractFrameState::SetBaseShape(BaseShape *pBaseShape) {
    if (pBaseShape == NULL)
        return;

    printf("Set base shape for:%s\n", this->GetTypeName());
    baseShape = pBaseShape;

    mBaseShapePPtr = pBaseShape;
}

BaseShape *AbstractFrameState::GetBaseShape() {
    if (this->baseShape != NULL)
        return this->baseShape;

    this->baseShape = &(*mBaseShapePPtr);

    return this->baseShape;
}

const char *AbstractFrameState::GetName() const {
    return frameStateName.c_str();
}

void AbstractFrameState::SetName(const char *name) {
    frameStateName = name;
}