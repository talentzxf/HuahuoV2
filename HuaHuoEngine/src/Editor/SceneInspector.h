//
// Created by VincentZhang on 5/7/2022.
//

#ifndef HUAHUOENGINE_SCENEINSPECTOR_H
#define HUAHUOENGINE_SCENEINSPECTOR_H

#include "Export/Events/ScriptEventManager.h"

class Transform;
class ISceneInspector {

};

class TransformHierarchyEventArgs: public ScriptEventHandlerArgs{
public:
    TransformHierarchyEventArgs(Transform* t);
    TransformHierarchyEventArgs(Transform* t, Transform* oldParent, Transform* newParent);
    Transform* GetTransform() {
        return m_pTransform;
    }

    Transform* GetOldParent(){
        return m_pOldParent;
    }

    Transform* GetNewParent(){
        return m_pNewParent;
    }

private:
    Transform* m_pTransform;
    Transform* m_pOldParent;
    Transform* m_pNewParent;
};

class SceneTracker{
public:
    static void StaticInitialize(void*);
    static void StaticDestroy(void*);

    SceneTracker();
    ~SceneTracker();

private:
    static void TransformHierarchyChangedCallback(Transform* t);
    static void TransformHierarchyChangedSetParentCallback(Transform* obj, Transform* oldParent, Transform* newParent);

    void TransformHierarchyChanged(Transform* t);
    void TransformDidSetParent(Transform* obj, Transform* old, Transform* parent);
};

SceneTracker& GetSceneTracker();
SceneTracker* GetSceneTrackerPtr();

#endif //HUAHUOENGINE_SCENEINSPECTOR_H
