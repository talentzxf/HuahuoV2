#pragma once

class Object;
struct ManagerContext;

#include "TypeSystem/Type.h"

EXPORT_COREMODULE bool IsManagerContextAvailable(int index);
EXPORT_COREMODULE Object& GetManagerFromContext(int index);
EXPORT_COREMODULE Object* GetManagerPtrFromContext(int index);
void SetManagerPtrInContext(int index, Object* ptr);
void ManagerContextInitializeClasses();

#define GET_MANAGER(x) x& Get##x () { return reinterpret_cast<x&> (GetManagerFromContext (ManagerContext::k##x)); }
#define GET_MANAGER_PTR(x) x* Get##x##Ptr () { return reinterpret_cast<x*> (GetManagerPtrFromContext (ManagerContext::k##x)); }

const ManagerContext& GetManagerContext();

struct ManagerContext
{
    enum Managers
    {
        // Global managers
        kPlayerSettings = 0,
        kInputManager,
        kTagManager,
        kAudioManager,
        kScriptMapper,
        kMonoManager,
        kGraphicsSettings,
        kTimeManager,
        kDelayedCallManager,
        kPhysicsManager,
        kBuildSettings,
        kQualitySettings,
        kResourceManager,
        kNavMeshProjectSettings,
        kPhysics2DSettings,
        kClusterInputManager,
        kRuntimeInitializeOnLoadManager,
        kUnityConnectSettings,
        kStreamingManager,

        kVFXManager,

        kGlobalManagerCount,

        // Level managers
        kFirstLevelManager = kGlobalManagerCount,
        kOcclusionCullingSettings = kFirstLevelManager,
        kRenderSettings,
        kLightmapSettings,
        kNavMeshSettings,
        kManagerCount,

        kLevelGameManagerCount = kManagerCount - kGlobalManagerCount
    };

    ManagerContext();
    Object* m_Managers[kManagerCount];
    const HuaHuo::Type*  m_ManagerTypes[kManagerCount];
    #if DEBUGMODE
    const char* m_ManagerNames[kManagerCount];
    #endif
    void InitializeClasses();
};
