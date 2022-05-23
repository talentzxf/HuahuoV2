//
// Created by VincentZhang on 5/23/2022.
//

#ifndef HUAHUOENGINE_LINE2D_H
#define HUAHUOENGINE_LINE2D_H
#include "Base2DObject.h"
#include "Math/Vector3f.h"

class Line2D : public Base2DObject{
    REGISTER_CLASS(Line2D);
    DECLARE_OBJECT_SERIALIZE();
public:
    Line2D(MemLabelId memLabelId, ObjectCreationMode creationMode)
        :Super(memLabelId, creationMode)
    {

    }

private:
    Vector3f p1;
    Vector3f p2;
};


#endif //HUAHUOENGINE_LINE2D_H
