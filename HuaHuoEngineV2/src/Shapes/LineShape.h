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
private:
    Vector3f p1;
    Vector3f p2;
};


#endif //HUAHUOENGINEV2_LINESHAPE_H
