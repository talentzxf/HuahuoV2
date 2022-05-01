#pragma once

enum BitMasks
{
    // Can't modify these without breaking backwards compatibility!
    kDefaultLayer = 0,
    kNoFXLayer = 1,
    kIgnoreRaycastLayer = 2,
    kIgnoreCollisionLayer = 3,
    kWaterLayer = 4,
    kUILayer = 5,
    kNumLayers = 32,

    kDefaultLayerMask = 1 << kDefaultLayer,
    kNoFXLayerMask = 1 << kNoFXLayer,
    kIgnoreRaycastMask = 1 << kIgnoreRaycastLayer,
    kIgnoreCollisionMask = 1 << kIgnoreCollisionLayer,
    kPreUnity2UnusedLayer = 1 << 5,

    kDefaultRaycastLayers = ~kIgnoreRaycastMask,

    kLegacyBuiltInLayerCount = 8,
};

enum Tags
{
    kUntagged = 0,
    kRespawnTag = 1,
    kFinishTag = 2,
    kEditorOnlyTag = 3,
    kMainCameraTag = 5,
    kPlayerTag = 6,
    kGameControllerTag = 7,
    kFirstUserTag = 20000,
    kLastUserTag = 30000,
    kUndefinedTag = -1
};

// Ensure in sync with EditorSceneManager.DefaultSceneCullingMask and SceneCullingMasks in C#
static const UInt64 kGameViewObjects_SceneCullingMask = 1ULL << 63;
static const UInt64 kMainStageExcludingPrefabInstanceObjectsOpenInPrefabMode_SceneCullingMask = 1ULL << 62;
static const UInt64 kMainStagePrefabInstanceObjectsOpenInPrefabMode_SceneCullingMask = 1ULL << 61;
static const UInt64 kMainStageSceneViewObjects_SceneCullingMask = kMainStagePrefabInstanceObjectsOpenInPrefabMode_SceneCullingMask | kMainStageExcludingPrefabInstanceObjectsOpenInPrefabMode_SceneCullingMask;
static const UInt64 kPrefabStagePrefabInstanceObjectsOpenInPrefabMode_SceneCullingMask = 1ULL << 60;
static const UInt64 kDefaultSceneCullingMask = kGameViewObjects_SceneCullingMask | kMainStageSceneViewObjects_SceneCullingMask;
