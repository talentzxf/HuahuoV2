#pragma once

#include "Configuration/IntegerDefinitions.h"
#include <string>

// When adding a subsystem, keep in sync with AsyncReadManager.bindings.cs
enum class AssetSubsystem
{
    Other = 0,
    Texture,
    VirtualTexture,
    Mesh,
    Audio,
    Scripts,
    EntitiesScene,
    EntitiesStreamBinaryReader,
    Max // For use alongside AssetSubsystemNames for getting the length ONLY. When adding a new subsystem, do it above this value (And be sure to add the name to the AssetSubsystemNames array)
};

struct AssetContext
{
#if ENABLE_PROFILER
    core::string assetName;
    UInt32 profilerFlowId;
    SInt32 assetTypeID;
    AssetSubsystem subsystem;

    const static char* AssetSubsystemNames[];

    AssetContext() :
        assetName(""),
        profilerFlowId(0),
        assetTypeID(0),
        subsystem(AssetSubsystem::Other)
    {}

    AssetContext(AssetSubsystem system) :
        assetName(""),
        profilerFlowId(0),
        assetTypeID(0),
        subsystem(system)
    {}

    AssetContext(const core::string& loadingAssetName, AssetSubsystem system) :
        assetName(loadingAssetName),
        profilerFlowId(0),
        assetTypeID(0),
        subsystem(system)
    {}

    AssetContext(SInt32 persistentTypeID, const core::string& loadingAssetName, UInt32 profilerFlow, AssetSubsystem system = AssetSubsystem::Other) :
        assetName(loadingAssetName),
        profilerFlowId(profilerFlow),
        assetTypeID(persistentTypeID),
        subsystem(system)
    {}

public:
    static const char* GetAssetSubsystemName(AssetSubsystem subsystem);

    inline const char* GetAssetSubsystemName() const
    {
        return GetAssetSubsystemName(subsystem);
    }

#else
    AssetContext() {}
    AssetContext(AssetSubsystem system) {}
    AssetContext(const std::string& loadingAssetName, AssetSubsystem system) {}
    AssetContext(SInt32 persistentTypeID, const std::string& loadingAssetName, UInt32 profilerFlow, AssetSubsystem system = AssetSubsystem::Other) {}
#endif // ENABLE_PROFILER
};
