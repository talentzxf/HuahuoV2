//
// Created by VincentZhang on 7/17/2022.
//

#ifndef HUAHUOENGINEV2_CLONEOBJECT_H
#define HUAHUOENGINEV2_CLONEOBJECT_H

#include "TypeSystem/Object.h"
#include "Utilities/vector_map.h"

typedef UNITY_VECTOR_MAP (kMemTempAlloc, InstanceID, InstanceID) TempRemapTable;

static Object* CloneObject(Object& inObject);


#endif //HUAHUOENGINEV2_CLONEOBJECT_H
