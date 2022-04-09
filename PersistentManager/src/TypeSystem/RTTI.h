//
// Created by VincentZhang on 4/5/2022.
//

#ifndef PERSISTENTMANAGER_RTTI_H
#define PERSISTENTMANAGER_RTTI_H
#include <cstddef>
#include "BaseClasses/BaseTypes.h"
#include "Variant.h"
class Object;

struct RTTI{
    enum
    {
        UndefinedPersistentTypeID = -1
    };
    typedef Object* FactoryFunction ();

    enum
    {
        // These values are chosen so that IsDerivedFrom(a,b) always return false if a or b have default values (they are undefined types)
        DefaultTypeIndex = 0x80000000,
        DefaultDescendentCount = 0
    };

    struct DerivedFromInfo
    {
        RuntimeTypeIndex typeIndex;        // consecutive type index, assigned so that all descendant classes have a type index in the range [typeIndex,typeIndex+descendantCount[
        UInt32 descendantCount;
    };

    const RTTI* base;
    FactoryFunction* factory;
    const char* className;
    const char* classNamespace;
    const char* module;
    PersistentTypeID persistentTypeID;
    int size;
    DerivedFromInfo derivedFromInfo;
    bool isAbstract;
    bool isSealed;
    bool isEditorOnly;
    bool isStripped;
    const ConstVariantRef* attributes;
    size_t attributeCount;
    struct RuntimeTypeArray
    {
        static const UInt32 MAX_RUNTIME_TYPES = 1024;

        UInt32 Count;
        RTTI* Types[MAX_RUNTIME_TYPES];
    };

    static RuntimeTypeArray& GetRuntimeTypes()
    {
        return ms_runtimeTypes;
    }

private:
    static RuntimeTypeArray ms_runtimeTypes;
};


#endif //PERSISTENTMANAGER_RTTI_H
