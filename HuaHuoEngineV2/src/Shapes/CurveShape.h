//
// Created by VincentZhang on 8/21/2022.
//

#ifndef HUAHUOENGINEV2_CURVESHAPE_H
#define HUAHUOENGINEV2_CURVESHAPE_H


#include "BaseShape.h"

class CurveShape: public BaseShape {
    REGISTER_CLASS(CurveShape);
    DECLARE_OBJECT_SERIALIZE()

public:
    CurveShape(MemLabelId label, ObjectCreationMode mode)
    :Super(label, mode)
    {
    }

    virtual const char* GetTypeName(){
        return "CurveShape";
    }

};


#endif //HUAHUOENGINEV2_CURVESHAPE_H
