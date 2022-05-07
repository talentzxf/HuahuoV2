//
// Created by VincentZhang on 5/7/2022.
//

#ifndef HUAHUOENGINE_HUAHUOSCENE_H
#define HUAHUOENGINE_HUAHUOSCENE_H
#include "Utilities/LinkedList.h"

class GameObject;
class Transform;
typedef List<ListNode<Transform> > RootTransformList;
class HuaHuoScene {
public:
    HuaHuoScene();
    ~HuaHuoScene();

    int GetRootCount() const;

    static void AddRootToScene(HuaHuoScene& scene, Transform& t);
    static void RemoveRootFromScene(Transform& t, bool notifySceneTracker);
    static void OnGameObjectChangedScene(GameObject& gameObject, HuaHuoScene* currentScene, HuaHuoScene* previousScene);
    bool IsEmpty() const;

    inline RootTransformList::iterator RootBegin(bool alphaSorted = false)
    {
#if UNITY_EDITOR
        if (alphaSorted)
        {
            if (!m_UnsortedRoots.empty())
                MergeUnsortedRoots();
            return m_SortedRoots.begin();
        }
        else
#endif
        return m_Roots.begin();
    }

    inline RootTransformList::iterator RootEnd(bool alphaSorted = false)
    {
#if UNITY_EDITOR
        if (alphaSorted)
        {
            if (!m_UnsortedRoots.empty())
                MergeUnsortedRoots();
            return m_SortedRoots.end();
        }
        else
#endif
        return m_Roots.end();
    }

    inline RootTransformList::const_iterator RootBegin(bool alphaSorted = false) const
    {
        return RootTransformList::const_iterator(&*const_cast<HuaHuoScene*>(this)->RootBegin(alphaSorted));
    }

    inline RootTransformList::const_iterator RootEnd(bool alphaSorted = false) const
    {
        return RootTransformList::const_iterator(&*const_cast<HuaHuoScene*>(this)->RootEnd(alphaSorted));
    }

private:
    RootTransformList       m_Roots;
};


#endif //HUAHUOENGINE_HUAHUOSCENE_H
