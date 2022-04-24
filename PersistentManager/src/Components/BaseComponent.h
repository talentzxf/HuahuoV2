//
// Created by VincentZhang on 4/10/2022.
//

#ifndef PERSISTENTMANAGER_BASECOMPONENT_H
#define PERSISTENTMANAGER_BASECOMPONENT_H


#include "TypeSystem/Object.h"

class BaseComponent: public Object{
    REGISTER_CLASS(BaseComponent);
    DECLARE_OBJECT_SERIALIZE();
public:
    BaseComponent(ObjectCreationMode mode)
        :Super(mode)
    {

    }
};


#endif //PERSISTENTMANAGER_BASECOMPONENT_H
