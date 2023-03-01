//
// Created by VincentZhang on 2023-02-28.
//

#ifndef HUAHUOENGINEV2_RESOURCEMANAGER_H
#define HUAHUOENGINEV2_RESOURCEMANAGER_H


#include "TypeSystem/Object.h"

class ResourceManager: public Object {
    REGISTER_CLASS(ResourceManager);
    DECLARE_OBJECT_SERIALIZE();
public:
    ResourceManager(MemLabelId label, ObjectCreationMode mode)
        :Super(label, mode)
    {

    }

    static ResourceManager* GetDefaultResourceManager();
};

ResourceManager* GetDefaultResourceManager();


#endif //HUAHUOENGINEV2_RESOURCEMANAGER_H
