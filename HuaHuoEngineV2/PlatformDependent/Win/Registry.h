#pragma once

#include "Runtime/Core/Containers/String.h"
#include "Runtime/Scripting/BindingsDefs.h"
#include <windef.h>

#include <vector>

namespace registry
{
    enum RegistryView
    {
        kRegistryViewDefault = 0,
        kRegistryView32 = 1,
        kRegistryView64 = 2,
    };

    core::string getString(core::string_ref path, core::string_ref key, core::string_ref def, RegistryView view = kRegistryViewDefault);
    core::wstring getString(LPCWSTR path, LPCWSTR key, LPCWSTR def, RegistryView view = kRegistryViewDefault);
    bool setString(core::string_ref path, core::string_ref key, core::string_ref val, RegistryView view = kRegistryViewDefault);
    bool setString(LPCWSTR path, LPCWSTR key, LPCWSTR val, RegistryView view = kRegistryViewDefault);
    unsigned int getUInt32(core::string_ref path, core::string_ref key, unsigned int def, RegistryView view = kRegistryViewDefault);
    unsigned int getUInt32(LPCWSTR path, LPCWSTR key, unsigned int def, RegistryView view = kRegistryViewDefault);

    #if UNITY_EDITOR && !UNITY_EXTERNAL_TOOL
    void getKeyValues(core::string_ref key, dynamic_array<core::string>& outKeys, dynamic_array<core::string>& outValues);
    #endif
}
BIND_MANAGED_TYPE_NAME(registry::RegistryView, UnityEditorInternal_RegistryView)
