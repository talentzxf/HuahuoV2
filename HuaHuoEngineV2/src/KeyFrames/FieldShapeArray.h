//
// Created by VincentZhang on 2022-11-02.
//

#ifndef HUAHUOENGINEV2_FIELDSHAPEARRAY_H
#define HUAHUOENGINEV2_FIELDSHAPEARRAY_H

#include "Shapes/BaseShape.h"
#include <vector>

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

    void InsertShape(BaseShape* shapePtr){
        shapeArray.push_back(shapePtr);
    }

    void DeleteShape(BaseShape* shapePtr){
        shapeArray.erase(std::remove(shapeArray.begin(), shapeArray.end(), shapePtr), shapeArray.end());
    }

    DECLARE_SERIALIZE(FieldShapeArray)

private:
    ShapeArray shapeArray;
};

template <class TransferFunction> void FieldShapeArray::Transfer(TransferFunction &transfer) {
    TRANSFER(shapeArray);
}


#endif //HUAHUOENGINEV2_FIELDSHAPEARRAY_H
