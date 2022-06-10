#include "AssetContext.h"

#if ENABLE_PROFILER
const char* AssetContext::AssetSubsystemNames[] =
{
    "Other",
    "Texture",
    "VirtualTexture",
    "Mesh",
    "Audio",
    "Scripts",
    "EntitiesScene",
    "EntitiesStreamBinaryReader"
};
CompileTimeAssertArraySize(AssetContext::AssetSubsystemNames, (static_cast<UInt32>(AssetSubsystem::Max)));

const char* AssetContext::GetAssetSubsystemName(AssetSubsystem subsystem)
{
    Assert(static_cast<UInt32>(subsystem) < ARRAY_SIZE(AssetSubsystemNames));
    return AssetSubsystemNames[static_cast<UInt32>(subsystem)];
}

#endif
