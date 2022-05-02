//
// Created by VincentZhang on 4/8/2022.
//

#ifndef HUAHUOENGINE_MEMORYMACROS_H
#define HUAHUOENGINE_MEMORYMACROS_H
#include "MemoryMacrosDetails.h"
#include <limits>

#define NEW_AS_ROOT(type, areaName, objectName) new type
#define NEW(type)   new type
#define DELETE(ptr) {delete ptr; ptr = NULL;}
#define MEMCPY memcpy
#define FREE(ptr) free(ptr)
#define NEW_ARRAY(type, size) new type[size]
#define ALLOC_ARRAY(type, size) (type *)malloc(sizeof(type) * size)
//#define NEW_AS_ROOT(type, areaName, objectName) NewWithLabelConstructor<type>(alignof(type), __FILE__ , __LINE__).NEW_AS_ROOT_WITH_LABEL_CONSTRUCT

template<typename T>
inline bool DoesAdditionOverflow(T a, T b)
{
    return std::numeric_limits<T>::max() - a < b;
}
#endif //HUAHUOENGINE_MEMORYMACROS_H
