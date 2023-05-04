//
// Created by VincentZhang on 4/30/2022.
//

#include "Type.h"
#include "TypeManager.h"

namespace HuaHuo
{
    const Type* Type::FindTypeByName(const char* name, CaseSensitivityOptions options)
    {
        return reinterpret_cast<const Type*>(TypeManager::Get().ClassNameToRTTI(name, options == kCaseInSensitive));
    }

    const Type* Type::GetDeserializationStubForPersistentTypeID(PersistentTypeID id)
    {
        return reinterpret_cast<const Type*>(TypeManager::Get().GetDeserializationRTTIStubForPersistentTypeID(id));
    }

    const Type* Type::FindTypeByPersistentTypeID(PersistentTypeID id)
    {
        return reinterpret_cast<const Type*>(TypeManager::Get().PersistentTypeIDToRTTI(id));
    }

    void Type::FindAllDerivedClasses(std::vector<const Type*>& result, TypeFilterOptions options) const
    {
        TypeManager::Get().FindAllRTTIDerivedTypes(&m_internal, reinterpret_cast<std::vector<const RTTI*>&>(result), options == kOnlyNonAbstract);
    }
}

