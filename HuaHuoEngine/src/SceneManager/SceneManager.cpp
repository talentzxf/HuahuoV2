//
// Created by VincentZhang on 5/7/2022.
//

#include "SceneManager.h"
#include "Memory/MemoryMacros.h"
#include "Utilities/RegisterRuntimeInitializeAndCleanup.h"

static RuntimeSceneManager* g_RuntimeSceneManager = NULL;
static void StaticInitializeRuntimeSceneManager(void*)
{
    g_RuntimeSceneManager = NEW(RuntimeSceneManager);//, kMemSceneManager, "Managers", "RuntimeSceneManager")();
}

static void StaticDestroyRuntimeSceneManager(void*)
{
    // UNITY_DELETE(g_RuntimeSceneManager, kMemSceneManager);
    DELETE(g_RuntimeSceneManager)
    g_RuntimeSceneManager = NULL;
}

static RegisterRuntimeInitializeAndCleanup s_SceneManagerCallbacks(StaticInitializeRuntimeSceneManager, StaticDestroyRuntimeSceneManager);

RuntimeSceneManager& GetSceneManager()
{
    return *g_RuntimeSceneManager;
}

SceneManager* RuntimeSceneManager::GetSceneManager(){
    return g_RuntimeSceneManager;
}

RuntimeSceneManager::RuntimeSceneManager(/*MemLabelRef label*/)
        : m_ActiveScene(NULL)
        // , m_DontDestroyOnLoadScene(AllocateSceneHandle(), label, "DontDestroyOnLoad", "", UnityGUID(), -1, false)
{
//    GlobalCallbacks::Get().garbageCollect.Register(MarkLevelGameManagerDependencies);
//    GlobalCallbacks::Get().didUnloadScene.Register(SceneWasUnloaded);
//    GlobalCallbacks::Get().didChangeActiveScene.Register(ActiveSceneChanged);
//
//    m_DontDestroyOnLoadScene.SetLoadingState(HuaHuoScene::kLoaded);
}

RuntimeSceneManager::~RuntimeSceneManager()
{
//    GlobalCallbacks::Get().didChangeActiveScene.Unregister(ActiveSceneChanged);
//    GlobalCallbacks::Get().didUnloadScene.Unregister(SceneWasUnloaded);
//    GlobalCallbacks::Get().garbageCollect.Unregister(MarkLevelGameManagerDependencies);

    auto it = m_Scenes.begin();
    while (!m_Scenes.empty())
    {
        // (*it)->Release();
        it = m_Scenes.erase(it);
    }
}

HuaHuoScene* RuntimeSceneManager::CreateScene()
{
    HuaHuoScene* scene = NEW(HuaHuoScene);
    m_Scenes.push_back(scene);
    return scene;
}

HuaHuoScene* RuntimeSceneManager::GetActiveScene()
{
    // __FAKEABLE_METHOD__(RuntimeSceneManager, GetActiveScene, ());

    if (m_ActiveScene != NULL)
        return m_ActiveScene;

//    // The active scene could be null if it's being loaded, then we should use integrating scene instead.
//    // Please refer to bug https://fogbugz.unity3d.com/default.asp?730696 for more details.
//    return GetSceneIntegratingOnMainThread();
    return NULL;
}

bool RuntimeSceneManager::SetActiveScene(HuaHuoScene& scene)
{
#if UNITY_EDITOR
    if (scene.IsPreviewScene())
    {
        ErrorString("PreviewScene cannot be set as ActiveScene");
        return false;
    }
#endif

//    if (!scene.IsLoaded())
//        return false;

    if (m_ActiveScene == &scene)
        return false;


    HuaHuoScene* oldScene = m_ActiveScene;

    m_ActiveScene = &scene;
//    m_ActiveScene->RegisterLevelGameManagersWithManagerContext();

//    Assert(oldScene != m_ActiveScene);
//    INVOKE_GLOBAL_CALLBACK(didChangeActiveScene, oldScene, m_ActiveScene);

    return true;
}