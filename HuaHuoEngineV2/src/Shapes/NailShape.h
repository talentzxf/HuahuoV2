//
// Created by VincentZhang on 2022-11-24.
//

#ifndef HUAHUOENGINEV2_NAILSHAPE_H
#define HUAHUOENGINEV2_NAILSHAPE_H
#include "BaseShape.h"

class NailShape: public BaseShape{
    REGISTER_CLASS(NailShape);
    DECLARE_OBJECT_SERIALIZE()

public:
    NailShape(MemLabelId memLabelId, ObjectCreationMode mode)
    :Super(memLabelId, mode)
    {

    }

    virtual const char* GetTypeName() override{
        return "NailShape";
    }
};


#endif //HUAHUOENGINEV2_NAILSHAPE_H
