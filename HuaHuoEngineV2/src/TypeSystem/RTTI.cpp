//
// Created by VincentZhang on 4/5/2022.
//

#include "RTTI.h"
RTTI::RuntimeTypeArray RTTI::ms_runtimeTypes;

std::string RTTI::GetFullName() const
{
    if (classNamespace && *classNamespace)
    {
        std::string fullName;
        fullName += classNamespace;
        fullName += "::";
        fullName += className;
        return fullName;
    }

    return className;
}


