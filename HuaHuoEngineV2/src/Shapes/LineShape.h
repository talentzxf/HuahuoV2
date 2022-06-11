//
// Created by VincentZhang on 6/1/2022.
//

#ifndef HUAHUOENGINEV2_LINESHAPE_H
#define HUAHUOENGINEV2_LINESHAPE_H
#include "BaseShape.h"
#include "Math/Vector3f.h"

class LineShape : public BaseShape{
    REGISTER_CLASS(LineShape);
    DECLARE_OBJECT_SERIALIZE()
public:
    LineShape(MemLabelId label, ObjectCreationMode mode)
    :Super(label, mode)
    {

    }

    void AwakeFromLoad(AwakeFromLoadMode awakeMode) override;

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


#endif //HUAHUOENGINEV2_LINESHAPE_H
