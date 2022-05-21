//
// Created by VincentZhang on 4/6/2022.
//

#ifndef HUAHUOENGINE_TYPEMANAGER_H
#define HUAHUOENGINE_TYPEMANAGER_H

#include <cstdio>
#include <cstring>
#include "Memory/MemoryMacros.h"
#include "RTTI.h"
#include "ObjectDefines.h"
#include "Object.h"
#include "Utilities/HashFunctions.h"
#include "Utilities/HashStringFunctions.h"
#include <unordered_map>
#include <map>

class TypeManager {
public:
    TypeManager(RTTI::RuntimeTypeArray& runtimeTypes);
    ~TypeManager();

    struct HashFunctorPersistentTypeID
    {
        size_t operator()(PersistentTypeID data) const
        {
            return ComputeIntHash(data);
        }
    };

    static TypeManager& Get()
    {
        Assert(ms_Instance != NULL);
        return *ms_Instance;
    }

    static void InitializeGlobalInstance();
    void CallInitializeTypes();
    void CallPostInitializeTypes();
    static void CleanupGlobalInstance();
    void CleanupAllTypes();

    void RegisterType(const TypeRegistrationDesc& desc);

    // this is intended to be used by general code, so we're going to automatically pull RTTI from static data to keep general code needing to understand type system internals
    template<typename T>
    void RegisterType(PersistentTypeID typeID, const char* name, const char* nameSpace)
    {
        TypeRegistrationDesc desc = TYPEREGISTRATIONDESC_DEFAULT_INITIALIZER_LIST;
        desc.type = &TypeContainer<T>::rtti;
        desc.init.persistentTypeID = typeID;
        desc.init.className = name;
        desc.init.classNamespace = nameSpace;
        desc.init.attributes = RegisterAttributes<T>(desc.init.attributeCount);
        RegisterType(desc);
    }

    template<typename T, typename Base>
    void RegisterType(PersistentTypeID typeID, const char* name, const char* nameSpace)
    {
        TypeRegistrationDesc desc = TYPEREGISTRATIONDESC_DEFAULT_INITIALIZER_LIST;
        desc.type = &TypeContainer<T>::rtti;
        desc.init.base = &TypeContainer<Base>::rtti;
        desc.init.persistentTypeID = typeID;
        desc.init.className = name;
        desc.init.classNamespace = nameSpace;
        desc.init.attributes = RegisterAttributes<T>(desc.init.attributeCount);
        RegisterType(desc);
    }

    void RegisterNonObjectType(PersistentTypeID typeID, RTTI* destinationRTTI, const char* name, const char* nameSpace);

    void InitializeAllTypes();

    const RTTI* ClassNameToRTTI(const char* name, bool caseInsensitive = false) const;
private:
    class Builder;

    struct TypeCallbackStruct
    {
        TypeCallback* initType;
        TypeCallback* postInitType;
        TypeCallback* cleanupType;

        TypeCallbackStruct()
        {
            initType = postInitType = cleanupType = NULL;
        }
    };

    struct ConstCharPtrHashFunctor
    {
        size_t operator()(const char* str) const
        {
            return ComputeShortStringHash32(str);
        }
    };

    struct ConstCharPtrEqualTo
    {
        bool operator()(const char* a, const char* b) const
        {
            return a == b || (a != NULL && b != NULL && strcmp(a, b) == 0);
        }
    };

    typedef std::map<PersistentTypeID, TypeCallbackStruct> TypeCallbacks;
    typedef std::unordered_map<PersistentTypeID, RTTI*, HashFunctorPersistentTypeID> RTTIMap;
    typedef std::unordered_map<const char*, const RTTI*, ConstCharPtrHashFunctor, ConstCharPtrEqualTo> StringToTypeMap;

    RTTI::RuntimeTypeArray& m_RuntimeTypes;
    StringToTypeMap m_StringToType;
    TypeCallbacks m_TypeCallbacks;
    void FatalErrorOnPersistentTypeIDConflict(PersistentTypeID typeID, const char* name);
    RTTIMap m_RTTI;
    static TypeManager* ms_Instance;
};

#endif //HUAHUOENGINE_TYPEMANAGER_H
