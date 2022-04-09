//
// Created by VincentZhang on 4/8/2022.
//

#ifndef PERSISTENTMANAGER_MEMORYMACROS_H
#define PERSISTENTMANAGER_MEMORYMACROS_H
#include "MemoryMacrosDetails.h"

#define NEW_AS_ROOT(type, areaName, objectName) new type
//#define NEW_AS_ROOT(type, areaName, objectName) NewWithLabelConstructor<type>(alignof(type), __FILE__ , __LINE__).NEW_AS_ROOT_WITH_LABEL_CONSTRUCT

#endif //PERSISTENTMANAGER_MEMORYMACROS_H
