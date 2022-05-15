#pragma once

#include "BaseClasses/BaseTypes.h"
#include "Utilities/EnumFlags.h"

//----------------------------------------------------------------------------------------------------------------------
// What is this: Lists all known format versions that the SerializedFile has gone through.
//----------------------------------------------------------------------------------------------------------------------
enum class SerializedFileFormatVersion : UInt32
{
    kUnsupported = 1,           // format no longer readable.
    kUnknown_2 = 2,             // semantic lost to history, but tested against in code.
    kUnknown_3 = 3,             // semantic lost to history, but tested against in code.
    kUnknown_5 = 5,             // semantic lost to history, but tested against in code.
    kUnknown_6 = 6,             // semantic lost to history, but tested against in code.
    kUnknown_7 = 7,             // semantic lost to history, but tested against in code.
    kUnknown_8 = 8,             // semantic lost to history, but tested against in code.
    kUnknown_9 = 9,             // semantic lost to history, but tested against in code.
    kUnknown_10 = 10,           // Developed in parallel: Version 10 Blobified TypeTree
    kHasScriptTypeIndex = 11,   // Developed in parallel: Version 11 Script References
    kUnknown_12 = 12,           // Version: 12  Blobified TypeTree & Script References
    kHasTypeTreeHashes = 13,
    kUnknown_14 = 14,           // semantic lost to history, but tested against in code.
    kSupportsStrippedObject = 15,
    kRefactoredClassId = 16,    // 5.5: widened serialized ClassID to 32 bit.
    kRefactorTypeData = 17,     // 5.5: moved all other type-data from Object to Type
    kRefactorShareableTypeTreeData = 18, // 2019.1 : TypeTree's now reference a shareable/cachable data set
    kTypeTreeNodeWithTypeFlags = 19, // 2019.1: TypeTree's can contain nodes that express managed references
    kSupportsRefObject = 20,    // 2019.2: SerializeFile support managed references
    kStoresTypeDependencies = 21, // 2019.2: SerializeFile includes info on types that depend on other types
    kLargeFilesSupport = 22,    // 2020.1: Large file support

    kCurrentSerializeVersion = kLargeFilesSupport, // increment when changing the serialization format and add an enum above for previous version logic checks
};
ENUM_FLAGS(SerializedFileFormatVersion);

SerializedFileFormatVersion GetCurrentSerializeVersion();
