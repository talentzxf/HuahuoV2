//
// Created by VincentZhang on 4/5/2022.
//

#ifndef HUAHUOENGINE_RTTI_H
#define HUAHUOENGINE_RTTI_H
#include <cstddef>
#include "BaseClasses/BaseTypes.h"
#include "Variant.h"
#include "Memory/AllocatorLabels.h"
#include <string>
class Object;
enum ObjectCreationMode
{
    // Create the object from the main thread in a perfectly normal way
    kCreateObjectDefault = 0,
    // Create the object from another thread. Might assign an instance ID but will not register with IDToPointer map.
    // Objects created like this, need to call,  AwakeFromLoadThraded, and Object::RegisterInstanceID and AwakeFromLoad (kDidLoadThreaded); from the main thread
    kCreateObjectFromNonMainThread = 1,
    // Create the object and register the instance id but do not lock the object
    // creation mutex because the code calling it already called SetObjectLockForWrite().
    kCreateObjectDefaultNoLock = 2
};

struct RTTI{
    enum
    {
        UndefinedPersistentTypeID = -1
    };
    typedef Object* FactoryFunction (MemLabelId label, ObjectCreationMode mode);

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

    static inline bool IsDerivedFrom(RuntimeTypeIndex typeIndex, RuntimeTypeIndex baseTypeIndex, UInt32 baseDescendantCount)
    {
        return (typeIndex - baseTypeIndex) < baseDescendantCount;
    }

    static inline bool IsDerivedFrom(RuntimeTypeIndex typeIndex, const RTTI& baseType)
    {
        return (typeIndex - baseType.derivedFromInfo.typeIndex) < baseType.derivedFromInfo.descendantCount;
    }

    static inline bool IsDerivedFrom(const RTTI& derivedType, const RTTI& baseType)
    {
        return (derivedType.derivedFromInfo.typeIndex - baseType.derivedFromInfo.typeIndex) < baseType.derivedFromInfo.descendantCount;
    }

    std::string GetFullName() const;

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


#endif //HUAHUOENGINE_RTTI_H
