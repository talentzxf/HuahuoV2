#pragma once

#include "Utilities/StringComparison.h"
#include <set>

class AllowNameConversions;

typedef UNITY_SET_CMP (kMemSerialization, char*, smaller_cstring) OldTransferNames;

/// Allows name conversion from oldName to newName. (The passed strings will not be copied so you can only pass in constant strings)
/// (Useful for deprecating names -> m_NewPosition will now load from m_DeprecatedPosition in an old serialized file
/// RegisterAllowNameConversion(TypeOf<MyClass>()->GetName(), "m_DeprecatedPosition", "m_NewPosition");

// All API's are available as global state & local state. Global state is used for all C++ components since and setup on startup.
// LocalState is used by scripting on a per monobehaviour basis.
EXPORT_COREMODULE void RegisterAllowNameConversion(const char* type, const char* oldName, const char* newName);
EXPORT_COREMODULE void RegisterAllowNameConversionInDerivedTypes(const char* type, const char* oldName, const char* newName);
EXPORT_COREMODULE void RegisterAllowNameConversion(AllowNameConversions& nameConversions, const char* type, const char* oldName, const char* newName);


// Used by SafeBinaryRead / YamlRead to check if a variable name can be converted.
const OldTransferNames* GetAllowNameConversions(const AllowNameConversions* nameConversion, const char* type, const char* name);
inline bool IsNameConversionAllowed(const OldTransferNames* nameConversion, const char* name) { return nameConversion != NULL && nameConversion->count(const_cast<char*>(name)); }

// Create a local AllowNameConversions
// AllowNameConversions can be passed to SafeBinaryRead/YamlRead, so that you can have a unique per object allow name conversion map
// Use RegisterAllowNameConversion to fill the data once it has been created.
AllowNameConversions* CreateAllowNameConversions();
void DestroyAllowNameConversions(AllowNameConversions* conversions);

const AllowNameConversions* GetGlobalAllowNameConversions();
