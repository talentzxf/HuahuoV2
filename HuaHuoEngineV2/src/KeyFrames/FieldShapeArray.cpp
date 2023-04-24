//
// Created by VincentZhang on 2022-11-02.
//

#include "FieldShapeArray.h"
#include "Shapes/BaseShape.h"

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

bool FieldShapeArray::ContainShape(BaseShape *shapePtr) {
    for (BaseShape *shape: this->shapeArray) {
        if (shape->GetInstanceID() == shapePtr->GetInstanceID())
            return true;
    }
    return false;
}

BaseShape *FieldShapeArray::GetShape(int idx) {
    if (idx >= shapeArray.size())
        return NULL;

    return shapeArray[idx];
}