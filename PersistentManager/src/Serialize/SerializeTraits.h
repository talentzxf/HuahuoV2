//
// Created by VincentZhang on 4/21/2022.
//

#ifndef PERSISTENTMANAGER_SERIALIZETRAITS_H
#define PERSISTENTMANAGER_SERIALIZETRAITS_H
#include "SerializeTraitsBase.h"
#include "Containers/CommonString.h"

#define DEFINE_GET_TYPESTRING_BASIC_TYPE(x)     \
    inline static const char* GetTypeString (void* p = NULL)    { return CommonString(x); } \
    inline static bool MightContainPPtr ()  { return false; }\
    inline static bool AllowTransferOptimization () { return true; }

template<>
struct SerializeTraits<float> : public SerializeTraitsBaseForBasicType<float>
{
    DEFINE_GET_TYPESTRING_BASIC_TYPE(float)
};


#endif //PERSISTENTMANAGER_SERIALIZETRAITS_H
