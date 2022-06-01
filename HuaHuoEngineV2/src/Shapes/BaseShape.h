//
// Created by VincentZhang on 6/1/2022.
//

#ifndef HUAHUOENGINEV2_BASESHAPE_H
#define HUAHUOENGINEV2_BASESHAPE_H
#include "TypeSystem/Object.h"

class BaseShape : public Object{
    REGISTER_CLASS_TRAITS(kTypeIsAbstract);
    REGISTER_CLASS(BaseShape);
    DECLARE_OBJECT_SERIALIZE();
public:
    BaseShape(MemLabelId label, ObjectCreationMode mode)
        :Super(label, mode)
    {}
};


#endif //HUAHUOENGINEV2_BASESHAPE_H
