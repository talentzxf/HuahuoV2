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
}

