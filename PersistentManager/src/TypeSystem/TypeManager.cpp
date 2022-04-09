//
// Created by VincentZhang on 4/6/2022.
//

#include "TypeManager.h"
#include "Object.h"

TypeManager* TypeManager::ms_Instance = NULL;

void GlobalRegisterType(const TypeRegistrationDesc& desc)
{
    TypeManager::Get().RegisterType(desc);
}

TypeManager::TypeManager(RTTI::RuntimeTypeArray& runtimeTypeArray)
        : m_RuntimeTypes(runtimeTypeArray)
{
    m_RuntimeTypes.Count = 0;
}


void TypeManager::FatalErrorOnPersistentTypeIDConflict(PersistentTypeID typeID, const char* name)
{
//    RTTIMap::const_iterator iRTTI = m_RTTI.find(typeID);
//    if (iRTTI != m_RTTI.end())
//        FatalErrorString(Format("ClassID %d (%s) conflicts with that of another class (%s). Please resolve the conflict.", typeID, name, iRTTI->second->className));
//
//    ReservedTypeIDMap::const_iterator iReserved = m_ReservedTypeIDs.find(typeID);
//    if (iReserved != m_ReservedTypeIDs.end())
//        FatalErrorString(Format("ClassID %d (%s) conflicts with that of another class (%s). Please resolve the conflict.", typeID, name, iReserved->second));
}

void TypeManager::RegisterType(const TypeRegistrationDesc& desc)
{
    Assert(desc.init.persistentTypeID != RTTI::UndefinedPersistentTypeID);
    FatalErrorOnPersistentTypeIDConflict(desc.init.persistentTypeID, desc.init.className);

    // Copy info from desc to rtti
    RTTI& destinationRTTI = *desc.type;
    destinationRTTI = desc.init;

    m_RTTI[destinationRTTI.persistentTypeID] = &destinationRTTI;

    // Store callbacks
    if (desc.initCallback || desc.postInitCallback || desc.cleanupCallback)
    {
        TypeCallbackStruct& callback = m_TypeCallbacks[destinationRTTI.persistentTypeID];
        callback.initType = desc.initCallback;
        callback.postInitType = desc.postInitCallback;
        callback.cleanupType = desc.cleanupCallback;
    }

    // Store String -> persistentTypeID
    if (!destinationRTTI.isStripped)
    {
        Assert(m_StringToType.find(destinationRTTI.className) == m_StringToType.end());
        m_StringToType[destinationRTTI.className] = &destinationRTTI;
    }

#if ENABLE_ASSERTIONS
    core::hash_map<const Unity::Type*, bool> uniqueCheck;
    for (size_t i = 0; i < desc.init.attributeCount; ++i)
    {
        const Unity::Type* attrType = desc.init.attributes[i].GetType();
        AssertFormatMsg(
            uniqueCheck.insert(attrType, true).second,
            "Only a single instance of a given attribute (%s) is permitted to be registered to a type (%s)",
            attrType->GetFullName().c_str(), destinationRTTI.GetFullName().c_str());
    }
#endif
}