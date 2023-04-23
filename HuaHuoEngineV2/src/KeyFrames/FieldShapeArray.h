//
// Created by VincentZhang on 2022-11-02.
//

#ifndef HUAHUOENGINEV2_FIELDSHAPEARRAY_H
#define HUAHUOENGINEV2_FIELDSHAPEARRAY_H

#include <vector>
#include "BaseClasses/PPtr.h"

class BaseShape;
class CustomFrameState;

typedef vector<PPtr<BaseShape>> ShapeArray;

bool operator==(BaseShape* p1, PPtr<BaseShape> p2);

class FieldShapeArray {
public:
    int GetShapeCount(){
        return shapeArray.size();
    }

    BaseShape* GetShape(int idx){
        if(idx >= shapeArray.size())
            return NULL;

        return shapeArray[idx];
    }

    bool ContainShape(BaseShape* shapePtr);

    void InsertShape(BaseShape* shapePtr);

    void DeleteShape(BaseShape* shapePtr);

    DECLARE_SERIALIZE(FieldShapeArray);

    void SetFrameState(CustomFrameState* frameState){
        this->frameState = frameState;
    }
private:
    ShapeArray shapeArray;
    CustomFrameState* frameState;
};

template <class TransferFunction> void FieldShapeArray::Transfer(TransferFunction &transfer) {
    TRANSFER(shapeArray);
}


#endif //HUAHUOENGINEV2_FIELDSHAPEARRAY_H
