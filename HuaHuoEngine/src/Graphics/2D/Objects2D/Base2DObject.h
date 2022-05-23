//
// Created by VincentZhang on 5/23/2022.
//

#ifndef HUAHUOENGINE_BASE2DOBJECT_H
#define HUAHUOENGINE_BASE2DOBJECT_H
#include "TypeSystem/Object.h"

class Base2DObject : public Object{
    REGISTER_CLASS_TRAITS(kTypeIsAbstract);
    REGISTER_CLASS(Base2DObject);
    DECLARE_OBJECT_SERIALIZE();
public:
    Base2DObject(MemLabelId memLabelId, ObjectCreationMode creationMode)
        :Super(memLabelId, creationMode)
    {

    }
};


#endif //HUAHUOENGINE_BASE2DOBJECT_H
