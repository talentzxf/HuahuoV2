//
// Created by VincentZhang on 2022-11-02.
//

#ifndef HUAHUOENGINEV2_FIELDSHAPEARRAY_H
#define HUAHUOENGINEV2_FIELDSHAPEARRAY_H

typedef vector<PPtr<BaseShape>> ShapeArray;
#include <vector>

bool operator==(BaseShape* p1, PPtr<BaseShape> p2){
    return p1->GetInstanceID() == p2->GetInstanceID();
}

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

private:
    ShapeArray shapeArray;
};


#endif //HUAHUOENGINEV2_FIELDSHAPEARRAY_H
