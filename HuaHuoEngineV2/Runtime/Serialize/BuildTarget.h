#pragma once

#include "Serialize/SerializeUtility.h"

// #include "Runtime/Scripting/BindingsDefs.h"

/// This needs to be in Sync with BuildTarget in C#
// ADD_NEW_PLATFORM_HERE
enum BuildTargetPlatform
{
    kBuildNoTargetPlatform = -2,
    kBuildAnyPlayerData = -1,
    kBuildValidPlayer = 1,

    kFirstValidStandaloneTarget = 2,
    // We don't support building for these any more, but we still need the constants for asset bundle
    // backwards compatibility.
    kBuildDeprecatedStandaloneOSXPPC = 3,

    kBuildDeprecatedStandaloneOSXIntel = 4,

    kBuildDeprecatedWebPlayerLZMA = 6,
    kBuildDeprecatedWebPlayerLZMAStreamed = 7,
    kBuildDeprecatedStandaloneOSXIntel64 = 27,
    kBuildStandaloneOSX = 2,
    kBuildStandaloneWinPlayer = 5,
    kBuild_iPhone = 9,
    kBuildDeprecatedPS3 = 10,
    // was kBuildXBOX360 = 11,
    // was kBuild_Broadcom = 12,
    kBuild_Android = 13,
    // was kBuildWinGLESEmu = 14,
    // was kBuildWinGLES20Emu = 15,
    // was kBuildNaCl = 16,
    kBuildDeprecatedStandaloneLinux = 17,
    // was kBuildFlash = 18,
    kBuildStandaloneWin64Player = 19,
    kBuildWebGL = 20,
    kBuildMetroPlayer = 21,
    kBuildStandaloneLinux64 = 24,
    kBuildDeprecatedStandaloneLinuxUniversal = 25,
    kBuildDeprecatedWP8Player = 26,
    kBuildDeprecatedBB10 = 28,
    kBuildDeprecatedTizen = 29,
    kBuildDeprecatedPSP2 = 30,
    kBuildPS4 = 31,
    kBuildDeprecatedPSM = 32,
    kBuildXboxOne = 33,
    kBuildDeprecatedSamsungTV = 34,
    kBuildDeprecatedN3DS = 35,
    kBuildDeprecatedWiiU = 36,
    kBuildtvOS = 37,
    kBuildSwitch = 38,
    kBuildLumin = 39,
    kBuildStadia = 40,
    kBuildCloudRendering = 41,
    kBuildGameCoreScarlett = 42,
    kBuildGameCoreXboxOne = 43,
    kBuildPS5 = 44,
    kBuildPlayerTypeCount
};

// BIND_MANAGED_TYPE_NAME(BuildTargetPlatform, UnityEditor_BuildTarget)

struct EXPORT_COREMODULE BuildTargetSelection
{
    //DEFINE_GET_TYPESTRING(BuildTargetSelection);

    BuildTargetPlatform platform;
    int subTarget;
    int extendedPlatform; // When building AssetBundles, treat `platform' akin to BuildTargetGroup and create assets that are compatible with multiple build-targets.
    int isEditor;

    BuildTargetSelection() : platform(kBuildNoTargetPlatform), subTarget(0), extendedPlatform(0), isEditor(0) {}

    BuildTargetSelection(BuildTargetPlatform platform_, int subTarget_, bool extended = false) : platform(platform_), subTarget(subTarget_), extendedPlatform(extended ? 1 : 0), isEditor(0) {}

    BuildTargetSelection(BuildTargetPlatform platform_, int subTarget_, bool extended, int isEditor_) : platform(platform_), subTarget(subTarget_), extendedPlatform(extended ? 1 : 0), isEditor(isEditor_) {}

    bool operator==(const BuildTargetSelection& rhs) const
    {
        if (platform != rhs.platform)
            return false;
        if (subTarget != rhs.subTarget)
            return false;
        if (extendedPlatform != rhs.extendedPlatform)
            return false;
        if (isEditor != rhs.isEditor)
            return false;

        return true;
    }

    bool operator!=(const BuildTargetSelection& rhs) const
    {
        return !operator==(rhs);
    }

    static BuildTargetSelection NoTarget() { return BuildTargetSelection(kBuildNoTargetPlatform, 0, false, 0); }
    static BuildTargetSelection EditorTarget() { return BuildTargetSelection(kBuildNoTargetPlatform, 0, false, 1); }

    template<class TransferFunction>
    void Transfer(TransferFunction& transfer)
    {
        TRANSFER_ENUM(platform);
        TRANSFER(subTarget);
        TRANSFER(extendedPlatform);
        TRANSFER(isEditor);
    }
};


/// This needs to be in Sync with XboxRunMethod in C#
enum XboxBuildSubtarget
{
    kXboxBuildSubtargetDevelopment = 0,
    kXboxBuildSubtargetMaster = 1,
    kXboxBuildSubtargetDebug = 2
};
//BIND_MANAGED_TYPE_NAME(XboxBuildSubtarget, UnityEditor_XboxBuildSubtarget)

// This needs to be in Sync with XboxOneDeployMethod in C#
enum XboxOneDeployMethod
{
    kXboxOneDeployMethod_Push = 0,
    kXboxOneDeployMethod_RunFromPC = 2,
    kXboxOneDeployMethod_Package = 3,
    kXboxOneDeployMethod_PackageStreaming = 4,
};
//BIND_MANAGED_TYPE_NAME(XboxOneDeployMethod, UnityEditor_XboxOneDeployMethod)

// This needs to be in Sync with XboxOneDeployDrive in C#
enum XboxOneDeployDrive
{
    kXboxOneDeployDrive_Default = 0,
    kXboxOneDeployDrive_Retail = 1,
    kXboxOneDeployDrive_Development = 2,
    kXboxOneDeployDrive_Ext1 = 3,
    kXboxOneDeployDrive_Ext2 = 4,
    kXboxOneDeployDrive_Ext3 = 5,
    kXboxOneDeployDrive_Ext4 = 6,
    kXboxOneDeployDrive_Ext5 = 7,
    kXboxOneDeployDrive_Ext6 = 8,
    kXboxOneDeployDrive_Ext7 = 9,
};
//BIND_MANAGED_TYPE_NAME(XboxOneDeployDrive, UnityEditor_XboxOneDeployDrive)

/// This needs to be in Sync with MobileTextureSubtarget in C# and GetMobileTextureSubtargetByName()
enum MobileTextureSubtarget
{
    kMobileTextureSubtarget_Generic = 0,
    kMobileTextureSubtarget_DXT     = 1,
    kMobileTextureSubtarget_PVRTC   = 2,
    kMobileTextureSubtarget_ATC     = 3, // Deprecated in 2018.1
    kMobileTextureSubtarget_ETC     = 4,
    kMobileTextureSubtarget_ETC2    = 5,
    kMobileTextureSubtarget_ASTC    = 6,
};
//BIND_MANAGED_TYPE_NAME(MobileTextureSubtarget, UnityEditor_MobileTextureSubtarget)

/// This needs to be in sync with AndroidETC2Fallback in C#
enum AndroidETC2Fallback
{
    kAndroidETC2Fallback_Quality32Bit           = 0,
    kAndroidETC2Fallback_Quality16Bit           = 1,
    kAndroidETC2Fallback_Quality32BitDownscaled = 2
};
//BIND_MANAGED_TYPE_NAME(AndroidETC2Fallback, UnityEditor_AndroidETC2Fallback)

/// This needs to be in Sync with Compression in C#
enum Compression
{
    kCompression_None = 0,
    kCompression_Lz4 = 2,
    kCompression_Lz4HC = 3
};
//BIND_MANAGED_TYPE_NAME(Compression, UnityEditor_Compression)

struct EXPORT_COREMODULE MobileBuildSubTarget
{
    int subtarget;
    MobileBuildSubTarget(MobileTextureSubtarget tex, UInt32 graphicsAPIMask) : subtarget((tex & 0x3F) | ((graphicsAPIMask & 0xFFFFFF) << 8)) {}
    MobileBuildSubTarget(MobileTextureSubtarget tex, AndroidETC2Fallback fallback, UInt32 graphicsAPIMask) : subtarget((tex & 0x3F) | ((fallback & 0x3) << 6) | ((graphicsAPIMask & 0xFFFFFF) << 8)) {}
    explicit MobileBuildSubTarget(int val) : subtarget(val) {}

    MobileTextureSubtarget GetTextureSubtarget() const { return (MobileTextureSubtarget)(subtarget & 0x3F); }
    AndroidETC2Fallback GetAndroidETC2Fallback() const { return (AndroidETC2Fallback)((subtarget & 0xC0) >> 6); }
    UInt32 GetGraphicsAPIMask() const { return UInt32(subtarget >> 8) & 0xFFFFFF; }
    bool IsGraphicsAPIEnabled(int api) const { return (GetGraphicsAPIMask() & (1 << api)) != 0; }

    /*explicit*/ operator MobileTextureSubtarget()      { return GetTextureSubtarget(); }
};

// Keep in sync with WSASubtarget in EditorSettings.bindings
enum WSASubtarget
{
    kWSASubtarget_AnyDevice = 0,
    kWSASubtarget_PC        = 1,
    kWSASubtarget_Mobile    = 2,
    kWSASubtarget_HoloLens  = 3
};
//BIND_MANAGED_TYPE_NAME(WSASubtarget, UnityEditor_WSASubtarget)
