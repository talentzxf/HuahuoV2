//
// Created by VincentZhang on 5/7/2022.
//

#include "HuaHuoScene.h"
#include "Components/Transform/Transform.h"

HuaHuoScene::HuaHuoScene(/*UnitySceneHandle handle, MemLabelRef memLabel, core::string_ref scenePath, core::string_ref assetPath, const UnityGUID& sceneGUID, int sceneBuildIndex, bool previewScene*/)
//        :   ThreadSharedObject<UnityScene>(memLabel)
//        ,   m_Handle(handle)
//        ,   m_AssetPath(assetPath)
//        ,   m_SceneBuildIndex(sceneBuildIndex)
//        ,   m_SceneCullingMask(kDefaultSceneCullingMask)
//        ,   m_PhysicsSceneHandle3D(kDefaultPhysicsSceneHandle)
//        ,   m_PhysicsSceneHandle2D(kDefaultPhysicsSceneHandle2D)
#if UNITY_EDITOR
,   m_Dirtiness(0)
    ,   m_IsPreviewScene(previewScene)
    ,   m_IsClosing(false)
    ,   m_IsSaving(false)
    ,   m_IsSubScene(false)
#endif
{
//    SetPathAndGUID(scenePath, sceneGUID);
//    SetLoadingState(kNotLoaded);
}

HuaHuoScene::~HuaHuoScene()
{
#if UNITY_EDITOR
    if (GetSceneTrackerPtr() && !m_IsPreviewScene)
        GetSceneTracker().DirtyTransformHierarchy();

    // On a very rare case, there could be DontDestroyOnLoad objects left here after the scene is unloaded,
    // if the user drag the DontDestroyOnLoad objects to this scene in HierarchyWindow in play mode.
    //
    // Literally, the DontDestroyOnLoad objects should not belong to this scene.
    // We keep them in this scene just to avoid breaking the dragging feature in play mode for now.
    // After we splitting the HierarchyWindow into 2 areas in play mode, we'll disable this kind of dragging.
    // At that time, these lines should be removed.
    UnityScene& dontDestroyOnLoadScene = GetSceneManager().GetDontDestroyOnLoadScene();
    while (!m_Roots.empty())
    {
        Transform& t = *m_Roots.begin()->GetData();
        RemoveRootFromScene(t, true);
        AddRootToScene(dontDestroyOnLoadScene, t);
        OnGameObjectChangedScene(t.GetGameObject(), &dontDestroyOnLoadScene, this);
    }
#endif

    // DestroyPhysicsSceneHandles();
}

bool HuaHuoScene::IsEmpty() const
{
    return m_Roots.empty();
}

void HuaHuoScene::AddRootToScene(HuaHuoScene& scene, Transform& t)
{
    SceneRootNode& rootNode = t.m_SceneRootNode;
    if (rootNode.IsInScene())
        return;

    // Here, we just make sure the parent is NULL.
    // We don't use a more strict assert: t.IsSceneRoot ().
    // Because, we also have a special UnityScene: EditorSceneManager::m_DontDestroyOnLoadScene.
    // In the extreme case, the user could set a GameObject with HideFlags.HideAndDontSave to be DontDestroyOnLoad.
    // HideAndDontSave GamaObjects should not be recorded in a normal scene,
    // but they have to be recorded in the DontDestroyOnLoadScene.
    DebugAssert(t.GetParent() == NULL);

#if UNITY_EDITOR
    if (t.GetRootOrder() >= 0)
    {
        RootTransformList::iterator insertIt = scene.m_Roots.begin();
        int count = 0;

        for (; insertIt != scene.m_Roots.end(); ++insertIt, ++count)
        {
            Transform* sibling = insertIt->GetData();
            if (sibling->GetRootOrder() > t.GetRootOrder())
                break;
        }

        scene.m_Roots.insert(insertIt, rootNode.m_ListNode);
    }
    else if (t.GetRootOrder() == Transform::kSemiNumericCompare)
    {
        RootTransformList::iterator insertIt = scene.m_Roots.begin();

        for (; insertIt != scene.m_Roots.end(); ++insertIt)
        {
            Transform* sibling = insertIt->GetData();
            int compare = SemiNumericCompare(t.GetName(), sibling->GetName());

            if (compare < 0)
                break;
        }

        scene.m_Roots.insert(insertIt, rootNode.m_ListNode);
    }
    else
    {
        Assert(t.GetRootOrder() == Transform::kPushBack);
        scene.m_Roots.push_back(rootNode.m_ListNode);
    }

    scene.m_UnsortedRoots.push_back(rootNode.m_SortedListNode);
    rootNode.m_UnityScene = &scene;

    if (GetSceneTrackerPtr() && !t.TestHideFlag(Object::kHideInHierarchy))
        GetSceneTracker().DirtyTransformHierarchy();
#else
    scene.m_Roots.push_back(rootNode.m_ListNode);
    rootNode.m_Scene = &scene;
#endif
}

void HuaHuoScene::OnGameObjectChangedScene(GameObject& gameObject, HuaHuoScene* currentScene, HuaHuoScene* previousScene)
{

}