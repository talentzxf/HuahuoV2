//
// Created by VincentZhang on 5/7/2022.
//

#include "SceneInspector.h"
#include "Components/Transform/Transform.h"
#include "Export/Events/ScriptEventManager.h"

#if WEB_ENV
#include <emscripten.h>
#endif

SceneTracker * gSceneTracker = NULL;

static RegisterRuntimeInitializeAndCleanup gRegisterCallbacks_SceneTracker(SceneTracker::StaticInitialize, SceneTracker::StaticDestroy, 1);

TransformHierarchyEventArgs::TransformHierarchyEventArgs(Transform* t)
:m_pTransform(t){

}

void SceneTracker::StaticInitialize(void*)
{
    Assert(gSceneTracker == NULL);

//    Object::RegisterDirtyCallback(SetObjectDirty);
//    Object::RegisterDestroyedCallback(DestroyObjectCallback);
//    GameObject::RegisterDestroyedCallback(DestroyGameObjectCallback);
//    GameObject::RegisterSetGONameCallback(SetGameObjectNameCallback);
    Transform::RegisterHierarchyChangedCallback(TransformHierarchyChangedCallback);
    Transform::RegisterHierarchyChangedSetParentCallback(TransformHierarchyChangedSetParentCallback);

    // gSceneTracker = UNITY_NEW_AS_ROOT(SceneTracker, kMemEditorUtility, "Editor", "SceneTracker") ();
    gSceneTracker = NEW(SceneTracker);
}

void SceneTracker::StaticDestroy(void*) {

}

SceneTracker::SceneTracker(/*MemLabelId label*/)
{
//    m_DontSendSelectionChanged = 0;
//    m_OldActive = m_Active = m_OldActiveGameObject = m_ActiveGameObject = InstanceID_None;
//    Assert(!gSceneTracker);
//    m_LockedInspector = 0;
//    m_DontSendSelectionChanged = false;
//    m_DirtyCallbacks = UNITY_NEW(CallbackContainer, label) (label);
//    m_TransformHierarchyHasChanged = false;
//    m_ActiveContext = InstanceID_None;
//    m_MemLabel = label;
//
//    static_cast<CallbackContainer*>(m_DirtyCallbacks)->reserve(124);
}

SceneTracker::~SceneTracker()
{
//    Assert(m_DontSendSelectionChanged == 0);
//    Assert(m_SceneInspectors.empty());
//
//    CallbackContainer* callbacks = static_cast<CallbackContainer*>(m_DirtyCallbacks);
//    UNITY_DELETE(callbacks, m_MemLabel);
//    gSceneTracker = NULL;
}

void SceneTracker::TransformHierarchyChanged(Transform* t)
{
//    CLEAR_ALLOC_OWNER;
//    DebugAssert(CurrentThread::EqualsID(Thread::mainThreadId));
//
//    bool shouldBePartOfVisibleHierarchy = !t->IsPersistent() && !t->TestHideFlag(Object::kHideInHierarchy);
//    if (shouldBePartOfVisibleHierarchy)
//        DirtyTransformHierarchy();

// VZ: Callback to javascript to refresh the hierarchy.
    TransformHierarchyEventArgs args(t);
    GetScriptEventManager()->TriggerEvent(EventType::OnHierarchyChange, &args);
}

void SceneTracker::TransformHierarchyChangedCallback(Transform *t) {
    // DebugAssert(CurrentThread::EqualsID(Thread::mainThreadId));
    gSceneTracker->TransformHierarchyChanged(t);
}

void SceneTracker::TransformHierarchyChangedSetParentCallback(Transform *obj, Transform *oldParent,
                                                              Transform *newParent) {
}

SceneTracker& GetSceneTracker()
{
    Assert(gSceneTracker != NULL);
    return *gSceneTracker;
}

SceneTracker* GetSceneTrackerPtr()
{
    return gSceneTracker;
}
