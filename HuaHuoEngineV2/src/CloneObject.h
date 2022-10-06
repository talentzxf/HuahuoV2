//
// Created by VincentZhang on 7/17/2022.
//

#ifndef HUAHUOENGINEV2_CLONEOBJECT_H
#define HUAHUOENGINEV2_CLONEOBJECT_H

#include "TypeSystem/Object.h"
#include "Utilities/vector_map.h"

class PreProcessor{
public:
    virtual bool PreProcessBeforeAwake(Object* clonedObj) = 0;
};

typedef UNITY_VECTOR_MAP (kMemTempAlloc, InstanceID, InstanceID) TempRemapTable;

Object* CloneObject(Object& inObject, PreProcessor* preProcessor = NULL);

#endif //HUAHUOENGINEV2_CLONEOBJECT_H
