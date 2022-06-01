#pragma once

#include "Utilities/EnumFlags.h"

/// Meta flags can be used like this:
/// transfer.Transfer (someVar, "varname", kHideInEditorMask);
/// The GenerateTypeTreeTransfer for example reads the metaflag mask and stores it in the TypeTree
typedef enum
{
    kNoTransferFlags        = 0,
    /// Putting this mask in a transfer will make the variable be hidden in the property editor
    kHideInEditorMask       = 1 << 0,

    /// Makes a variable not editable in the property editor
    kNotEditableMask     = 1 << 4,

    /// Makes arrays reorderable in the property editor
    kReorderable = 1 << 5,

    /// There are 3 types of PPtrs: kStrongPPtrMask, default (weak pointer)
    /// a Strong PPtr forces the referenced object to be cloned.
    /// A Weak PPtr doesnt clone the referenced object, but if the referenced object is being cloned anyway (eg. If another (strong) pptr references this object)
    /// this PPtr will be remapped to the cloned object
    /// If an  object  referenced by a WeakPPtr is not cloned, it will stay the same when duplicating and cloning, but be NULLed when templating
    kStrongPPtrMask     = 1 << 6,
    // unused  = 1 << 7,

    /// kTreatIntegerValueAsBoolean makes an integer variable appear as a checkbox in the editor, be written as true/false to JSON, etc
    kTreatIntegerValueAsBoolean = 1 << 8,

    // unused = 1 << 9,
    // unused = 1 << 10,
    // unused = 1 << 11,

    /// When the options of a serializer tells you to serialize debug properties kSerializeDebugProperties
    /// All debug properties have to be marked kDebugPropertyMask
    /// Debug properties are shown in expert mode in the inspector but are not serialized normally
    kDebugPropertyMask = 1 << 12,

    // Used in TypeTree to indicate that a property is aligned to a 4-byte boundary. Do not specify this flag when
    // transferring variables; call transfer.Align() instead.
    kAlignBytesFlag = 1 << 14,

    // Used in TypeTree to indicate that some child of this typetree node uses kAlignBytesFlag. Do not use this flag.
    kAnyChildUsesAlignBytesFlag = 1 << 15,

    // unused = 1 << 16,
    // unused = 1 << 18,

    // Ignore this property when reading or writing .meta files
    kIgnoreInMetaFiles = 1 << 19,

    // When reading meta files and this property is not present, read array entry name instead (for backwards compatibility).
    kTransferAsArrayEntryNameInMetaFiles = 1 << 20,

    // When writing YAML Files, uses the flow mapping style (all properties in one line, with "{}").
    kTransferUsingFlowMappingStyle = 1 << 21,

    // Tells SerializedProperty to generate bitwise difference information for this field.
    kGenerateBitwiseDifferences = 1 << 22,

    // Makes a variable not be exposed to the animation system
    kDontAnimate     = 1 << 23,

    // Encodes a 64-bit signed or unsigned integer as a hex string in text serializers.
    kTransferHex64   = 1 << 24,

    // Use to differentiate between uint16 and C# Char.
    kCharPropertyMask = 1 << 25,

    //do not check if string is utf8 valid, (usually all string must be valid utf string, but sometimes we serialize pure binary data to string,
    //for example TextAsset files with extension .bytes. In this case this validation should be turned off)
    //Player builds will never validate data. In editor we validate correct encoding of strings by default.
    kDontValidateUTF8 = 1 << 26,

    // Fixed buffers are serialized as arrays, use this flag to differentiate between regular arrays and fixed buffers.
    kFixedBufferFlag = 1 << 27,

    // It is not allowed to modify this property's serialization data.
    kDisallowSerializedPropertyModification = 1 << 28
} TransferMetaFlags;

ENUM_FLAGS(TransferMetaFlags);

typedef enum
{
    kNoTransferInstructionFlags = 0,

    kReadWriteFromSerializedFile = 1 << 0, // Are we reading or writing a serialized file
    kAssetMetaDataOnly = 1 << 1, // Only serialize data needed for .meta files
    kHandleDrivenProperties = 1 << 2, // Do not serialize scene values for DrivenProperties, save backup values instead
#if UNITY_EDITOR
    kLoadAndUnloadAssetsDuringBuild = 1 << 3,
    kSerializeDebugProperties = 1 << 4, // Should we serialize debug properties (eg. Serialize mono private variables)
#endif
    kIgnoreDebugPropertiesForIndex = 1 << 5, // Should we ignore Debug properties when calculating the TypeTree index
#if UNITY_EDITOR
    kBuildPlayerOnlySerializeBuildProperties = 1 << 6, // Used by eg. build player to make materials cull any properties are aren't used anymore !
#endif
    // Specify that we are currently cloning an object and that remapping of PPtrs has no happend yet and
    // we should therefore not call ISerializationCallbackReceiver.OnAfterDeserialize without updated PPtrs.
    kIsCloningObject = 1 << 7,
    kSerializeGameRelease = 1 << 8, // Should Transfer classes use optimized reading. Allowing them to read memory directly that normally has a type using ReadDirect.
    kSwapEndianess = 1 << 9, // Should we swap endianess when reading / writing a file
    kResolveStreamedResourceSources = 1 << 10,
    kDontReadObjectsFromDiskBeforeWriting = 1 << 11,
    kSerializeMonoReload = 1 << 12, // Should we backupmono mono variables for an assembly reload?
    kDontRequireAllMetaFlags = 1 << 13, // Can we fast path calculating all meta data. This lets us skip a bunch of code when serializing mono data.
    kSerializeForPrefabSystem = 1 << 14,
#if UNITY_EDITOR
    kSerializeForSlimPlayer = 1 << 15,
    kLoadPrefabAsScene = 1 << 16,
    kSerializeCopyPasteTransfer = 1 << 17, // Used when duplicating object in the Editor via commands like Ctrl + D, etc.
    kTempFileOnMemoryFileSystem = 1 << 18,
    kBuildResourceImage = 1 << 19,
    kDontWriteUnityVersion = 1 << 20,
    kSerializeEditorMinimalScene = 1 << 21,
    kGenerateBakedPhysixMeshes = 1 << 22,
#endif
    kThreadedSerialization = 1 << 23,
    kIsBuiltinResourcesFile = 1 << 24,
    kPerformUnloadDependencyTracking        = 1 << 25,
    kDisableWriteTypeTree = 1 << 26,
    kAutoreplaceEditorWindow = 1 << 27,// Editor only
    kDontCreateMonoBehaviourScriptWrapper = 1 << 28,
    kSerializeForInspector = 1 << 29,
    kSerializedAssetBundleVersion = 1 << 30, // When writing (typetrees disabled), allow later Unity versions an attempt to read SerializedFile.
    kAllowTextSerialization = 1 << 31
} TransferInstructionFlags;
ENUM_FLAGS(TransferInstructionFlags);

typedef enum
{
    kAssetBundleNoFlag = 0,
    kAssetBundleUncompressed = 1 << 0,
    kAssetBundleCollectDependencies = 1 << 1,
    kAssetBundleIncludeCompleteAssets = 1 << 2,
    kAssetBundleDisableWriteTypeTree = 1 << 3,
    kAssetBundleDeterministic = 1 << 4,
    kAssetBundleForceRebuild = 1 << 5,
    kAssetBundleIgnoreTypeTreeChanges = 1 << 6,
    kAssetBundleAppendHashToName = 1 << 7,
    kAssetBundleChunkCompressed = 1 << 8,
    kAssetBundleStrictMode = 1 << 9,
    kAssetBundleDryRunBuild = 1 << 10,
    kAssetBundleExtendedPlatform = 1 << 11, // When serializing assets, make them compatible with addition platforms than for specified BuildTarget (using to build AssetBundles for A$ preview)
    kAssetBundleDisableLoadAssetByFileName = 1 << 12,
    kAssetBundleDisableLoadAssetByFileNameWithExtension = 1 << 13,
    kAssetBundleAllowEditorOnlyScriptableObjects = 1 << 14,
    kAssetBundleStripUnityVersion = 1 << 15
} BuildAssetBundleOptions;
ENUM_FLAGS(BuildAssetBundleOptions);
// BIND_MANAGED_TYPE_NAME(BuildAssetBundleOptions, UnityEditor_BuildAssetBundleOptions);

typedef enum
{
    kResourceImageNotSupported = -2,
    kResourceImageInactive = -1,
    kGPUResourceImage = 0,
    kResourceImage = 1,
    kStreamingResourceImage = 2,
    kNbResourceImages = 3
} ActiveResourceImage;


/// This needs to be in sync with ScriptingImplementation in C#
enum ScriptingBackend
{
    kMono2x = 0,
    kIL2CPP = 1,
    // kWinRTDotNET was 2,
};

/// This needs to be in sync with Il2CppCompilerConfiguration in C#
enum Il2CppCompilerConfiguration
{
    kDebug = 0,
    kRelease = 1,
    kMaster = 2,
};

/// This needs to be in Sync with BuildOptions in C#
enum BuildPlayerOptions
{
    kBuildPlayerOptionsNone = 0,
    kDevelopmentBuild = 1 << 0,
    kAutoRun = 1 << 2,
    kSelectBuiltPlayer = 1 << 3,
    kBuildAdditionalStreamedScenes = 1 << 4,
    kAcceptExternalModificationsToPlayer = 1 << 5,
    kInstallInBuildsFolder = 1 << 6,
    kStartProfilerOnStartup = 1 << 8,
    kAllowDebugging = 1 << 9,
    kSymlinkLibraries = 1 << 10,
    kBuildPlayerUncompressed = 1 << 11,
    kConnectToHost = 1 << 12,
    kHeadlessModeEnabled = 1 << 14,
    kBuildScriptsOnly = 1 << 15,
    kForceEnableAssertions = 1 << 17,
    kBuildPlayerCompressedWithLz4 = 1 << 18,
    kBuildPlayerCompressedWithLz4HC = 1 << 19,
    //OBSOLETE: kForceOptimizeScriptCompilation = 1 << 19,
    kComputeCRC = 1 << 20,
    kStrictMode = 1 << 21,
    kIncludeTestAssemblies = 1 << 22,
    kNoUniqueIdentifier = 1 << 23,
    kWaitForPlayerConnection = 1 << 25,
    kStripUnityVersion = 1 << 27,
    kEnableDeepProfilingSupport = 1 << 28,
    kDetailedBuildReport = 1 << 29,
    kShaderLivelinkSupport = 1 << 30,
};
ENUM_FLAGS(BuildPlayerOptions);
//BIND_MANAGED_TYPE_NAME(BuildPlayerOptions, UnityEditor_BuildOptions);
//BIND_MANAGED_TYPE_NAME(ScriptingBackend, UnityEditor_ScriptingImplementation);
