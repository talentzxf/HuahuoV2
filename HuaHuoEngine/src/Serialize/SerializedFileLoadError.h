#pragma once

enum SerializedFileLoadError
{
    kSerializedFileLoadError_None = 0,
    kSerializedFileLoadError_HigherSerializedFileVersion = 1,
    kSerializedFileLoadError_OversizedFile = 2,
    kSerializedFileLoadError_MergeConflicts = 3,
    kSerializedFileLoadError_EmptyOrCorruptFile = 4,
    kSerializedFileLoadError_DuplicateFileIDs = 5,
    kSerializedFileLoadError_Unknown = -1
};
