//
// Created by VincentZhang on 4/5/2022.
//

#ifndef PERSISTENTMANAGER_TRANSFORM_H
#define PERSISTENTMANAGER_TRANSFORM_H

#include "TypeSystem//Object.h"
#include "TypeSystem/ObjectDefines.h"
#include "BaseComponent.h"

class Transform : public BaseComponent {
    REGISTER_CLASS(Transform);
    DECLARE_OBJECT_SERIALIZE();
public:
    Transform(ObjectCreationMode mode)
        :Super(mode)
    {

    }
};


#endif //PERSISTENTMANAGER_TRANSFORM_H
