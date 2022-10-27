//
// Created by VincentZhang on 2022-10-27.
//

#ifndef HUAHUOENGINEV2_MIRRORSHAPE_H
#define HUAHUOENGINEV2_MIRRORSHAPE_H

#include "BaseShape.h"

class MirrorShape : public BaseShape{
    REGISTER_CLASS(MirrorShape);
    DECLARE_OBJECT_SERIALIZE()
public:
    MirrorShape(MemLabelId label, ObjectCreationMode mode)
    :Super(label, mode){

    }
    virtual const char* GetTypeName() override{
        return "MirrorShape";
    }

    void SetStartPoint(float x,float y,float z){
        p1.x = x;
        p1.y = y;
        p1.z = z;
    }

    void SetEndPoint(float x, float y, float z){
        p2.x = x;
        p2.y = y;
        p2.z = z;
    }

    Vector3f* GetStartPoint(){
        return &p1;
    }

    Vector3f* GetEndPoint(){
        return &p2;
    }
private:
    Vector3f p1;
    Vector3f p2;

};


#endif //HUAHUOENGINEV2_MIRRORSHAPE_H
