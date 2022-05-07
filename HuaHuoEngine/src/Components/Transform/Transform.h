//
// Created by VincentZhang on 4/5/2022.
//

#ifndef HUAHUOENGINE_TRANSFORM_H
#define HUAHUOENGINE_TRANSFORM_H

#include "TypeSystem//Object.h"
#include "TypeSystem/ObjectDefines.h"
#include "Components/BaseComponent.h"
#include "Math/Quaternionf.h"
#include "Math/Vector3f.h"
#include "Math/Simd/RotationOrder.h"
#include "TransformAccess.h"
#include "BaseClasses/ImmediatePtr.h"
#include "TransformType.h"
#include "BaseClasses/GameObject.h"
#include "TransformChangeSystemMask.h"
#include "BaseClasses/MessageData.h"
#include "BaseClasses/MessageIdentifier.h"


DECLARE_MESSAGE_IDENTIFIER(kBeforeTransformParentChanged);
DECLARE_MESSAGE_IDENTIFIER(kTransformParentChanged);
DECLARE_MESSAGE_IDENTIFIER(kTransformChildrenChanged);


class HuaHuoScene;
// It is only supposed to be maintained by UnityScene and Transform.
struct SceneRootNode
{
    ListNode<Transform>     m_ListNode;
    HuaHuoScene*             m_Scene;
#if UNITY_EDITOR
    ListNode<Transform>     m_SortedListNode;
#endif

    SceneRootNode(Transform* t)
            :   m_ListNode(t)
            , m_Scene(NULL)
#if UNITY_EDITOR
    , m_SortedListNode(t)
#endif
    {}

    inline bool IsInScene() const
    {
#if UNITY_EDITOR
        // Assert data consistency
        DebugAssert(m_ListNode.IsInList() == m_SortedListNode.IsInList());
        DebugAssert(m_ListNode.IsInList() == (m_UnityScene != NULL));
#endif
        return m_ListNode.IsInList();
    }
};

class Transform : public BaseComponent {
    REGISTER_CLASS(Transform);
    DECLARE_OBJECT_SERIALIZE();
    friend class HuaHuoScene;
public:
    Transform(ObjectCreationMode mode);

    HuaHuoScene* GetScene();
    SceneRootNode& GetSceneRootNode() { return m_SceneRootNode; }
    const SceneRootNode& GetSceneRootNode() const { return m_SceneRootNode; }
    bool IsSceneRoot() const;

    static void InitializeClass();
    static void CleanupClass();

    bool IsTransformHierarchyInitialized() const { return m_TransformData.hierarchy != NULL; }

    template<class TransferFunction> void CompleteTransformTransfer(TransferFunction& transfer);

    /// Sets the rotation in local space
    void SetLocalRotation(const Quaternionf& rotation);
    /// Sets the Rotation in world space
    void SetRotation(const Quaternionf& rotation);
    /// Sets the local euler angles
    void SetLocalEulerAngles(const Vector3f& eulerAngles, math::RotationOrder order = math::kOrderUnityDefault);

    /// Sets the position in local space
    /// (If the object has no father, localspace is basically the same as world space)
    void SetLocalPosition(const Vector3f& inTranslation);
    /// Sets the position in world space
    void SetPosition(const Vector3f& position);

    /// Sets the father to p(if p is invalid the Transformcomponent will have no father)
    /// Returns false if the father could not be set
    /// This happens only if you are trying to set the father to one of its direct/indirect children.
    enum SetParentOption { kWorldPositionStays = 1 << 0, kLocalPositionStays = 1 << 1, kAllowParentingFromPrefab = 1 << 2, kDisableTransformMessage = 1 << 3 };
    bool SetParent(Transform * parent, SetParentOption options = kWorldPositionStays);

    Quaternionf GetLocalRotation() const;

    /// Sets the scale in local space
    void SetLocalScale(const Vector3f& scale);

    /// Sets the world position and rotation
    void SetPositionAndRotation(const Vector3f& position, const Quaternionf& rotation);

    Vector3f GetLocalPosition() const;

    Vector3f GetLocalScale() const;

    TransformType GetTransformType() const;

    // Returns synced TransformAccess.
    TransformAccessReadOnly GetTransformAccess() const;
    TransformAccess GetTransformAccess();

    void RebuildTransformHierarchy();

    /// Returns a ptr to the father transformcomponent (NULL if no father)
    Transform* GetParent() const   { return m_Father; }

    void QueueChanges();

    Transform* GetParentPtrInternal() const { return m_Father; }

    /// access to the children
    int GetChildrenCount() const                        { return m_Children.size(); }

    // Creates the transform hierarchy if it hasn't been constructed yet
    void EnsureTransformHierarchyExists()
    {
        if (!IsTransformHierarchyInitialized())
            RebuildTransformHierarchy();
    }

    typedef std::vector<ImmediatePtr<Transform> > TransformComList;
    typedef TransformComList::iterator  iterator;

    /// Finds given transform
    iterator Find(const Transform* child);
    Transform& GetChild(int i) const                   { Assert(i < (int)m_Children.size()); return *m_Children[i]; }
    iterator begin()                                   { return m_Children.begin(); }
    iterator end()                                     { return m_Children.end(); }

    void SetParentPtrInternal(Transform* father) { m_Father = father; }

    TransformComList& GetChildrenInternal() { return m_Children; }
    const TransformComList& GetChildrenInternal() const { return m_Children; }

    /// Reset position&rotation
    virtual void Reset() override;
    virtual void ResetReplacement();

    void EnsureCapacityIncrease(UInt32 nodeCountIncrease);
    void SetHierarchyCapacity(size_t capacity);

    UInt32 FindLastChildIndex() const;

    /// Returns a reference to the root transform (top most transform with no parent)
    Transform& GetRoot();

    void ValidateHierarchy(TransformHierarchy& hierarchy);

    virtual void AwakeFromLoad(AwakeFromLoadMode awakeMode) override;

    /// Broadcasts a message to this and all child transforms
    void BroadcastMessageAny(const MessageIdentifier& message, MessageData& data);
    inline void BroadcastMessage(const MessageIdentifier& message) { MessageData data; BroadcastMessageAny(message, data); }

public:
    typedef void TransformChangedCallback (Transform* t);
    typedef void HierarchyChangedCallback (Transform* t);
    typedef void HierarchyChangedCallbackSetParent (Transform* obj, Transform* oldParent, Transform* newParent);

#if HUAHUO_EDITOR
    static void RegisterHierarchyChangedCallback(HierarchyChangedCallback* callback);
    static void RegisterHierarchyChangedSetParentCallback(HierarchyChangedCallbackSetParent* callback);
#endif

protected:
    friend void GameObject::ReplaceTransformComponentInternal(Transform* newTransform/*, AwakeFromLoadQueue* queue*/);
    void ApplySerializedToRuntimeData();
    void ApplyRuntimeToSerializedData();
    void ValidateHierarchyRecursive(TransformHierarchy& hierarchy, int& index, int& nextIndex, int parentIndex, UInt8* hasTransformBeenVisited);
    void OnAddComponent(BaseComponent* com);
    void OnRemoveComponent(BaseComponent* com);

private:
    void SendTransformParentChanged();
    void CommonTransformReset();
    UInt32 CountNodesDeep() const;
    UInt32 InitializeTransformHierarchyRecursive(TransformHierarchy& hierarchy, int& index, int parentIndex);
    static void UpdateTransformAccessors(TransformHierarchy& hierarchy, UInt32 index);

protected:
    TransformAccess                  m_TransformData;
    Quaternionf                      m_LocalRotation;
    Vector3f                         m_LocalPosition;
    Vector3f                         m_LocalScale;

private:
    TransformComList                 m_Children;
    ImmediatePtr<Transform>          m_Father;
    SceneRootNode                    m_SceneRootNode;
};

/// Is transform a child of parent? Or is the transform the same.
bool IsChildOrSameTransform(Transform* transform, Transform* parent);
bool IsChildOrSameTransform(Transform& transform, Transform& parent);
TransformChangeSystemMask GetCloneChangeSystemInterestsMask();
TransformChangeSystemMask GetCloneChangeSystemChangesMask();
UInt32 GetCloneHierarchyChangeSystemInterestsMask();

#endif //HUAHUOENGINE_TRANSFORM_H
