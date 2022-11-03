//
// Created by VincentZhang on 2022-11-02.
//

#include "FieldShapeArray.h"
#include "Shapes/BaseShape.h"
#include "CustomFrameState.h"

bool operator==(BaseShape *p1, PPtr<BaseShape> p2) {
    return p1->GetInstanceID() == p2->GetInstanceID();
}

void FieldShapeArray::InsertShape(BaseShape *shapePtr) {
    shapeArray.push_back(shapePtr);

    this->frameState->Apply(); // ReApply the change so it can be reflected.
}

void FieldShapeArray::DeleteShape(BaseShape *shapePtr) {
    shapeArray.erase(std::remove(shapeArray.begin(), shapeArray.end(), shapePtr), shapeArray.end());

    this->frameState->Apply(); // ReApply the change so it can be reflected.
}