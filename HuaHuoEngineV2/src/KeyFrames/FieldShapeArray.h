//
// Created by VincentZhang on 2022-11-02.
//

#ifndef HUAHUOENGINEV2_FIELDSHAPEARRAY_H
#define HUAHUOENGINEV2_FIELDSHAPEARRAY_H

typedef vector<PPtr<BaseShape>> ShapeArray;

class FieldShapeArray {
public:
    int GetShapeCount(){
        return shapeArray.size();
    }

    BaseShape* GetShape(int idx){
        return shapeArray[idx];
    }
private:
    ShapeArray shapeArray;
};


#endif //HUAHUOENGINEV2_FIELDSHAPEARRAY_H
