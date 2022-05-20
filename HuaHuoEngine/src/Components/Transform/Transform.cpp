//
// Created by VincentZhang on 4/5/2022.
//

#include "Transform.h"
#include "Utilities/TypeConversion.h"
#include "TransformHierarchy.h"
#include "TransformChangeDispatch.h"
#include "Math/Simd/vec-transform.h"
#include "Utilities/ValidateArgs.h"
#include "TransformSync.h"
#include "BaseClasses/MessageHandlerRegistration.h"
#include "Math/Vector2f.h"
#include "Components/BaseComponent.h"
#include "SceneManager/HuaHuoScene.h"
#include "SceneManager/SceneManager.h"

using namespace TransformInternal;
static TransformChangeSystemHandle gHasChangedDeprecatedSystem;


DEFINE_MESSAGE_IDENTIFIER(kBeforeTransformParentChanged,
                          ("OnBeforeTransformParentChanged", MessageIdentifier::kSendToScripts));
DEFINE_MESSAGE_IDENTIFIER(kTransformParentChanged, ("OnTransformParentChanged", MessageIdentifier::kSendToScripts));
DEFINE_MESSAGE_IDENTIFIER(kTransformChildrenChanged, ("OnTransformChildrenChanged", MessageIdentifier::kSendToScripts));

#if HUAHUO_EDITOR

#include "BaseClasses/Callbacks/CallbackArray.h"

static Transform::HierarchyChangedCallback *gHierarchyChangedCallback = NULL;
static CallbackArray3<Transform *, Transform *, Transform *> gHierarchyChangedSetParentCallback;
#endif

Transform::Transform(ObjectCreationMode mode)
        : Super(mode), m_TransformData(TransformAccess::Null()), m_SceneRootNode(this) {

}

template<class TransferFunction>
void Transform::Transfer(TransferFunction &transfer) {
    Super::Transfer(transfer);
//
    if (transfer.IsWriting() && IsTransformHierarchyInitialized())
        ApplyRuntimeToSerializedData();

    TRANSFER(m_LocalRotation);
    TRANSFER(m_LocalPosition);
    TRANSFER(m_LocalScale);

    // Complete the transform transfer.
    CompleteTransformTransfer(transfer);

    if (transfer.IsReading() && IsTransformHierarchyInitialized())
        ApplySerializedToRuntimeData();
}

Quaternionf Transform::GetLocalRotation() const {
    printf("Get Local Rotation!\n");
#if UNITY_EDITOR
    if (!IsTransformHierarchyInitialized())
    {
        ErrorStringObject("Illegal transform access. Are you accessing a transform localRotation from OnValidate?\n", this);
        return Quaternionf::identity();
    }
#endif

    RETURN_QUATERNION(math::rotation(GetLocalTRS(GetTransformAccess())));
}

void Transform::SetRotation(const Quaternionf &inRotation) {
    if (SetGlobalR(GetTransformAccess(), QuaternionfTofloat4(inRotation))) {
        QueueChanges();
    }
}

void Transform::SetLocalPosition(const Vector3f &inTranslation) {
    ABORT_INVALID_VECTOR3(inTranslation, localPosition, transform);

    if (SetLocalT(GetTransformAccess(), Vector3fTofloat3(inTranslation))) {
        QueueChanges();
    }
}

void Transform::SetPosition(const Vector3f &p) {
    ABORT_INVALID_VECTOR3(p, position, transform);

    if (SetGlobalT(GetTransformAccess(), Vector3fTofloat3(p))) {
        QueueChanges();
    }
}

void Transform::CommonTransformReset() {
#if UNITY_EDITOR
    m_LocalEulerAnglesHint = Vector3f::zero;
#endif
}

void Transform::Reset() {
    Super::Reset();
    m_LocalRotation = Quaternionf::identity();
    m_LocalPosition = Vector3f::zero;
    m_LocalScale = Vector3f::one;

    CommonTransformReset();
}

void Transform::ResetReplacement() {
    Super::Reset();
    CommonTransformReset();
}

void Transform::ApplyRuntimeToSerializedData() {
    using namespace math;

    TransformAccess transformAccess = GetTransformAccess();

    const trsX &trs = GetLocalTRS(transformAccess);
    vstore3f(m_LocalPosition.GetPtr(), translation(trs));
    vstore4f(m_LocalRotation.GetPtr(), rotation(trs));
    vstore3f(m_LocalScale.GetPtr(), scale(trs));

#if UNITY_EDITOR
    vstore3f(m_LocalEulerAnglesHint.GetPtr(), GetEulerHint(transformAccess));
#endif
}

Transform::iterator Transform::Find(const Transform *child) {
    iterator it, itEnd = end();
    for (it = begin(); it != itEnd; ++it) {
        if (*it == child)
            return it;
    }
    return itEnd;
}


void Transform::QueueChanges() {
//    Assert(IsTransformHierarchyInitialized());
//    GetTransformChangeDispatch().QueueTransformChangeIfHasChanged(m_TransformData);
}

// Separate out the basic transfer properties from specialized processing during transfers.
// This is done so that types derived from Transform can perform their own special handling of the basic
// transform properties.  An example of this is RectTransform when it uses its driven properties.
template<class TransferFunction>
void Transform::CompleteTransformTransfer(TransferFunction &transfer) {
//    // When cloning objects for prefabs and instantiate, we don't use serialization to duplicate the hierarchy,
//    // we duplicate the hierarchy directly
//    if (SerializePrefabIgnoreProperties(transfer))
//    {
//#if UNITY_EDITOR
//        if (transfer.IsWriting() && transfer.NeedsInstanceIDRemapping())
//        {
//            WriteAllValidChildren(transfer, m_Children);
//        }
//        else
//#endif
//        {
//            transfer.Transfer(m_Children, "m_Children", kHideInEditorMask | kStrongPPtrMask);
//        }
//
//        transfer.Transfer(m_Father, "m_Father", kHideInEditorMask);
//    }
//
//#if UNITY_EDITOR
//
//    if (transfer.IsWriting())
//    {
//        m_RootOrder = GetSiblingIndex();
//    }
//    else if (transfer.IsReading())
//    {
//        // Set to kSemiNumericCompare so if it is not read in it will get inserted by name to the rootMapOrder
//        m_RootOrder = kSemiNumericCompare;
//    }
//
//    TRANSFER_EDITOR_ONLY_HIDDEN(m_RootOrder);
//    TRANSFER_EDITOR_ONLY_HIDDEN(m_LocalEulerAnglesHint);
//
//#endif
}

TransformAccessReadOnly Transform::GetTransformAccess() const {
    SyncTransformAccess(m_TransformData);
    return m_TransformData;
}

TransformAccess Transform::GetTransformAccess() {
    SyncTransformAccess(m_TransformData);
    return m_TransformData;
}


void Transform::SetLocalRotation(const Quaternionf &inRotation) {
    // ABORT_INVALID_QUATERNION(inRotation, localRotation, transform);

    if (SetLocalR(GetTransformAccess(), QuaternionfTofloat4(inRotation))) {
        // QueueChanges();
    }
}

Vector3f Transform::GetLocalPosition() const {
#if UNITY_EDITOR
    if (!IsTransformHierarchyInitialized())
    {
        ErrorStringObject("Illegal transform access. Are you accessing a transform localPosition from OnValidate?\n", this);
        return Vector3f::zero;
    }
#endif

    RETURN_VECTOR3(math::translation(GetLocalTRS(GetTransformAccess())));
}

Vector3f Transform::GetPosition() const
{
#if UNITY_EDITOR
    if (!IsTransformHierarchyInitialized())
    {
        ErrorStringObject("Illegal transform access. Are you accessing a transform position from OnValidate?\n", this);
        return Vector3f::zero;
    }
#endif

    RETURN_VECTOR3(CalculateGlobalPosition(GetTransformAccess()));
}

UInt32 Transform::InitializeTransformHierarchyRecursive(TransformHierarchy &hierarchy, int &index, int parentIndex) {
    UInt32 newIndex = index;
    index = hierarchy.nextIndices[newIndex];
    UInt32 oldIndex = m_TransformData.index;
    TransformHierarchy *oldHierarchy = m_TransformData.hierarchy;

    // Setup hierarchy and parent indices
    m_TransformData.index = newIndex;
    m_TransformData.hierarchy = &hierarchy;
    hierarchy.parentIndices[newIndex] = parentIndex;
    hierarchy.mainThreadOnlyTransformPointers[newIndex] = this;

    bool readFromSerializedData = oldHierarchy == NULL;
    if (readFromSerializedData) {
#if ENABLE_DEBUG_ASSERTIONS
        // Due to ASSERT_TRANSFORM_ACCESS failure.
        hierarchy.deepChildCount[newIndex] = 1;
#endif

        ApplySerializedToRuntimeData();
        hierarchy.systemChanged[newIndex] = gHasChangedDeprecatedSystem.Mask();
        hierarchy.systemInterested[newIndex] = gHasChangedDeprecatedSystem.Mask();

        hierarchy.hierarchySystemInterested[newIndex] = 0;

        // RegisterChangeSystemInterests();
    } else {
        hierarchy.localTransforms[newIndex] = oldHierarchy->localTransforms[oldIndex];
        hierarchy.localTransformTypes[newIndex] = oldHierarchy->localTransformTypes[oldIndex];

        hierarchy.systemChanged[newIndex] = oldHierarchy->systemChanged[oldIndex];
        hierarchy.systemInterested[newIndex] = oldHierarchy->systemInterested[oldIndex];

        hierarchy.hierarchySystemInterested[newIndex] = oldHierarchy->hierarchySystemInterested[oldIndex];

#if UNITY_EDITOR
        hierarchy.eulerHints[newIndex] = oldHierarchy->eulerHints[oldIndex];
#endif
    }

    hierarchy.combinedSystemChanged |= hierarchy.systemChanged[newIndex];
    hierarchy.combinedSystemInterest |= hierarchy.systemInterested[newIndex];

    UInt32 count = 1;
    size_t childCount = m_Children.size();
    for (size_t i = 0; i != childCount; i++)
        count += m_Children[i]->InitializeTransformHierarchyRecursive(hierarchy, index, newIndex);

    hierarchy.deepChildCount[newIndex] = count;
    return count;
}


void Transform::AwakeFromLoad(AwakeFromLoadMode awakeMode) {
    Super::AwakeFromLoad(awakeMode);

    EnsureTransformHierarchyExists();
    SyncTransformAccess(m_TransformData);

    // Only call SendTransformChanged if it was really changed eg.
    // by a propepertyeditor or prefab propagation but not if it was loaded from disk
    if (awakeMode == kDefaultAwakeFromLoad) {
        ApplySerializedToRuntimeData();

        // OnTransformChangedTRS(GetTransformAccess());

        QueueChanges();
    }

    // If it's already in a scene, don't move it to other scene.
    // Otherwise, whenever you modify the transform, the game object will be moved to other scene (most likely the active scene).
    if (!m_SceneRootNode.IsInScene())
    {
        // HuaHuoScene* scene = GetSceneManager().GetSceneIntegratingOnMainThread();
        HuaHuoScene* scene = NULL;
        if (scene == NULL)
            scene = GetSceneManager().GetActiveScene();

        if (IsSceneRoot() && scene)
        {
            HuaHuoScene::AddRootToScene(*scene, *this);

            // Do not call OnGameObjectChangedScene for corrupt transform in AwakeFromLoad (dangling transform without a pointer to its GameObject)
            if (GetGameObjectPtr())
                HuaHuoScene::OnGameObjectChangedScene(GetGameObject(), scene, NULL);
        }
    }

#if HUAHUO_EDITOR
    if (gHierarchyChangedCallback)
        gHierarchyChangedCallback(this);
#endif // #if UNITY_EDITOR
}

#if HUAHUO_EDITOR

void Transform::RegisterHierarchyChangedCallback(HierarchyChangedCallback *callback) {
    // If you hit this Assert then you may need to consider changing gHierarchyChangedCallback to a CallbackArray.
    AssertMsg(gHierarchyChangedCallback == nullptr, "gHierarchyChangedCallback already has a callback registered");
    gHierarchyChangedCallback = callback;
}

void Transform::RegisterHierarchyChangedSetParentCallback(HierarchyChangedCallbackSetParent *callback) {
    gHierarchyChangedSetParentCallback.Register(callback);
}

#endif

void Transform::RebuildTransformHierarchy() {
    //@TODO: There is no real need for this to support oldHierarchy != NULL case.
    //       However during load there is some cases where we need it.
    //       Lets refactor and fix those differently.
    //       Removes complexity and potential corner cases with change masks.

    Transform *root = this;
    while (root->GetParent() != NULL)
        root = root->GetParent();

    TransformHierarchy *oldHierarchy = root->m_TransformData.hierarchy;

    SInt32 nodeCount = root->CountNodesDeep();
    TransformHierarchy *hierarchy = CreateTransformHierarchy(nodeCount);

    AllocateTransformThread(*hierarchy, 0, nodeCount - 1);

    int index = 0;
    root->InitializeTransformHierarchyRecursive(*hierarchy, index, -1);
    Assert(index == -1);
    Assert(GetDeepChildCount(*hierarchy, 0) == nodeCount);

//    QueueChanges();
//
//#if !UNITY_RELEASE
//    root->ValidateHierarchy(*hierarchy);
//#endif

    DestroyTransformHierarchy(oldHierarchy);

    GetTransformHierarchyChangeDispatch().DispatchSelfAndAllChildren(root->m_TransformData,
                                                                     TransformHierarchyChangeDispatch::kInterestedInTransformAccess);
}

void Transform::ApplySerializedToRuntimeData() {
    TransformAccess transformAccess = GetTransformAccess();

    TransformInternal::InitLocalTRS(transformAccess, math::vload3f(m_LocalPosition.GetPtr()),
                                    math::vload4f(m_LocalRotation.GetPtr()), math::vload3f(m_LocalScale.GetPtr()));

#if UNITY_EDITOR
    SetEulerHint(transformAccess, math::vload3f(m_LocalEulerAnglesHint.GetPtr()));
#endif
}

UInt32 Transform::CountNodesDeep() const {
    int count = 1;
    for (int i = 0; i < m_Children.size(); i++)
        count += m_Children[i]->CountNodesDeep();
    return count;
}

bool IsChildOrSameTransform(Transform *transform, Transform *inParent) {
    Transform *child = transform;
    while (child) {
        if (child == inParent)
            return true;
        child = child->GetParent();
    }
    return false;
}

bool IsChildOrSameTransform(Transform &transform, Transform &inParent) {
    return IsChildOrSameTransform(&transform, &inParent);
}

void Transform::EnsureCapacityIncrease(UInt32 nodeCountIncrease) {
    TransformHierarchy &hierarchy = *GetTransformAccess().hierarchy;
    UInt32 requiredCapacity = GetDeepChildCount(hierarchy, 0) + nodeCountIncrease;
    if (requiredCapacity > hierarchy.transformCapacity)
        SetHierarchyCapacity(requiredCapacity * 2);
}

void Transform::UpdateTransformAccessors(TransformHierarchy &hierarchy, UInt32 index) {
    UInt32 count = GetDeepChildCount(hierarchy, index);
    Transform **transforms = hierarchy.mainThreadOnlyTransformPointers;
    SInt32 *parentIndices = hierarchy.parentIndices;

    Transform &rootTransform = *transforms[index];
    rootTransform.m_TransformData.hierarchy = &hierarchy;
    rootTransform.m_TransformData.index = index;
    if (index == 0)
        parentIndices[index] = -1;
    else
        parentIndices[index] = rootTransform.m_Father->m_TransformData.index;

    SInt32 cur = hierarchy.nextIndices[index];
    for (UInt32 i = 1; i < count; i++) {
        Transform &transform = *transforms[cur];
        transform.m_TransformData.hierarchy = &hierarchy;
        transform.m_TransformData.index = cur;
        parentIndices[cur] = transform.m_Father->m_TransformData.index;
        cur = hierarchy.nextIndices[cur];
    }
}

Transform &Transform::GetRoot() {
    // Fast path: Get the root transform directly from the TransformHierarchy information if possible
    if (m_TransformData.hierarchy != NULL)
        return *(m_TransformData.hierarchy->mainThreadOnlyTransformPointers)[0];

    // Otherwise walk the ancestors to find the root transform
    Transform *cur = this;
    Transform *curParent = NULL;
    while ((curParent = cur->GetParent()) != NULL)
        cur = curParent;

    return *cur;
}

void Transform::SetHierarchyCapacity(size_t capacity) {
    TransformAccess access = GetTransformAccess();
    TransformHierarchy &srcHierarchy = *access.hierarchy;

    Assert(capacity >= GetDeepChildCount(srcHierarchy, 0));

    if (srcHierarchy.transformCapacity == capacity)
        return;

    // PROFILER_AUTO(gSetCapacityProfile, this);

    TransformHierarchy *newHierarchy = CreateTransformHierarchy(capacity/*, GetRoot().GetMemoryLabel()*/);
    CopyTransformSubhierarchy(srcHierarchy, 0, *newHierarchy, GetCloneChangeSystemInterestsMask(),
                              TransformChangeSystemMask(0), GetCloneHierarchyChangeSystemInterestsMask(), false);
    UpdateTransformAccessors(*newHierarchy, 0);
    DestroyTransformHierarchy(&srcHierarchy);

    // GetTransformChangeDispatch().QueueTransformChangeIfHasChanged(*newHierarchy);
    GetTransformHierarchyChangeDispatch().DispatchSelfAndAllChildren(GetRoot().GetTransformAccess(),
                                                                     TransformHierarchyChangeDispatch::kInterestedInTransformAccess);
}

UInt32 Transform::FindLastChildIndex() const {
    const Transform *cur = this;

    while (cur->m_Children.size() > 0)
        cur = cur->m_Children[cur->m_Children.size() - 1];

    return cur->m_TransformData.index;
}

void Transform::ValidateHierarchy(TransformHierarchy &hierarchy) {
    Assert(GetParent() == NULL);
    Assert(hierarchy.deepChildCount[0] <= hierarchy.transformCapacity);
    int index = 0;
    int nextIndex = 0;

    UInt8 *hasTransformBeenVisited = 0;
    // ALLOC_TEMP_AUTO(hasTransformBeenVisited, hierarchy.transformCapacity);
    hasTransformBeenVisited = ALLOC_ARRAY(UInt8, hierarchy.transformCapacity);
    memset(hasTransformBeenVisited, 0, hierarchy.transformCapacity);

    ValidateHierarchyRecursive(hierarchy, index, nextIndex, -1, hasTransformBeenVisited);
    Assert(index == hierarchy.deepChildCount[0]);
    Assert(nextIndex == -1);

    // Validate free list
    UInt32 freeListCount = 0;

    SInt32 freeList = hierarchy.firstFreeIndex;
    Assert(freeList == -1 || hierarchy.prevIndices[freeList] == -1);

    while (freeList != -1) {
        Assert(hasTransformBeenVisited[freeList] == 0);
        hasTransformBeenVisited[freeList] = 1;

        DebugAssert(hierarchy.deepChildCount[freeList] == 0);

        SInt32 prev = freeList;
        freeList = hierarchy.nextIndices[freeList];

        Assert(freeList == -1 || hierarchy.prevIndices[freeList] == prev);

        freeListCount++;
    }

    Assert(hierarchy.transformCapacity - hierarchy.deepChildCount[0] == freeListCount);

    if ((hierarchy.combinedSystemChanged & ~gHasChangedDeprecatedSystem.Mask()) != 0) {
        Assert(hierarchy.changeDispatchIndex >= 0);
    }
}


void Transform::ValidateHierarchyRecursive(TransformHierarchy &hierarchy, int &hierarchyOrderIndex, int &nextIndex,
                                           int parentIndex, UInt8 *hasTransformBeenVisited) {
    SInt32 index = m_TransformData.index;

    Assert(m_TransformData.index == nextIndex);

    Assert(hasTransformBeenVisited[index] == 0);
    hasTransformBeenVisited[index] = 1;

    Assert(m_TransformData.hierarchy == &hierarchy);
    Assert(hierarchy.parentIndices[index] == parentIndex);
    Assert(hierarchy.mainThreadOnlyTransformPointers[index] == this);
    Assert(CalculateTransformType(GetTransformAccess()) == hierarchy.localTransformTypes[index]);
    Assert((hierarchy.systemInterested[index] & gHasChangedDeprecatedSystem.Mask()) != 0);
    // Assert((hierarchy.hierarchySystemInterested[index] & ~GetTransformHierarchyChangeDispatch().GetAllRegisteredSystemsMask()) == 0);
    Assert((hierarchy.systemInterested[index] | hierarchy.systemChanged[index]) == hierarchy.systemInterested[index]);
    Assert((hierarchy.combinedSystemChanged | hierarchy.systemChanged[index] | gHasChangedDeprecatedSystem.Mask()) ==
           (hierarchy.combinedSystemChanged | gHasChangedDeprecatedSystem.Mask()));

    // ValidateChangeSystemInterests();

    SInt32 expectedHierarchyOrderIndex = hierarchyOrderIndex + GetDeepChildCount(hierarchy, index);
    hierarchyOrderIndex++;

    nextIndex = hierarchy.nextIndices[index];
    if (nextIndex != -1) {
        SInt32 prevIndex = hierarchy.prevIndices[nextIndex];
        Assert(prevIndex == index);
    }

    for (size_t i = 0; i != m_Children.size(); i++)
        m_Children[i]->ValidateHierarchyRecursive(hierarchy, hierarchyOrderIndex, nextIndex, m_TransformData.index,
                                                  hasTransformBeenVisited);

    Assert(hierarchyOrderIndex == expectedHierarchyOrderIndex);
}

void Transform::SendTransformParentChanged() {
    GetGameObject().TransformParentHasChanged();

    // Send message that parenting has changed to any of the children of this.
    // (The transforms that now have a new direct or indirect parent)
    BroadcastMessage(kTransformParentChanged);
}

void Transform::BroadcastMessageAny(const MessageIdentifier &messageID, MessageData &data) {
    GameObject *go = GetGameObjectPtr();
    if (go)
        go->SendMessageAny(messageID, data);

    for (int i = 0; i < m_Children.size(); i++)
        m_Children[i]->BroadcastMessageAny(messageID, data);
}

void Transform::OnAddComponent(BaseComponent *com) {
    if (IsTransformHierarchyInitialized()) {
//        GetTransformChangeDispatch().AddPermanentInterests(GetTransformAccess(), com->GetType());
//        GetTransformHierarchyChangeDispatch().AddPermanentInterests(GetTransformAccess(), com->GetType());
    }
}

void Transform::OnRemoveComponent(BaseComponent *com) {
    if (IsTransformHierarchyInitialized()) {
//        GetTransformChangeDispatch().RemovePermanentInterests(GetTransformAccess(), com->GetType());
//        GetTransformHierarchyChangeDispatch().RemovePermanentInterests(GetTransformAccess(), com->GetType());
    }
}

void Transform::InitializeClass() {
    REGISTER_MESSAGE_PTR(kDidAddComponent, OnAddComponent, BaseComponent);
    REGISTER_MESSAGE_PTR(kDidRemoveComponent, OnRemoveComponent, BaseComponent);

#if UNITY_EDITOR
    gDirtyIndexSystem = GetTransformChangeDispatch().RegisterPermanentInterestSystem("gDirtyIndexSystem", TypeOf<Transform>(), TransformChangeDispatch::kInterestedInLocalTRS);
    gDirtyCallbackSystem = GetTransformChangeDispatch().RegisterPermanentInterestSystem("gDirtyCallbackSystem", TypeOf<Transform>(), TransformChangeDispatch::kInterestedInLocalTRS);
#endif
//    gHasChangedDeprecatedSystem = GetTransformChangeDispatch().RegisterPermanentInterestSystem("gHasChangedDeprecatedSystem", TypeOf<Transform>(), TransformChangeDispatch::kInterestedInGlobalTRS);
//    GetTransformChangeDispatch().SetDeprecatedTransformChange(gHasChangedDeprecatedSystem);
}

void Transform::CleanupClass() {
#if UNITY_EDITOR
    GetTransformChangeDispatch().UnregisterSystem(gDirtyIndexSystem);
    GetTransformChangeDispatch().UnregisterSystem(gDirtyCallbackSystem);
#endif
    // GetTransformChangeDispatch().UnregisterSystem(gHasChangedDeprecatedSystem);
}

//***@TODO: kDisableTransformMessage shouldn't this require that tranfsormhierarchy = null and that it is not going to be rebuilt???

bool Transform::SetParent(Transform *newFather, SetParentOption options) {
    // PROFILER_AUTO(gSetParentProfile);

    using namespace math;

    if (IsTransformHierarchyInitialized())
        SyncTransformAccess(m_TransformData);
    if ((newFather != NULL) && newFather->IsTransformHierarchyInitialized())
        SyncTransformAccess(newFather->m_TransformData);

    if (newFather == GetParent())
        return true;

    if (GetGameObject().IsDestroying()) {
        ErrorString(Format("Cannot set the parent of the GameObject '%s' while it is being destroyed.",
                           GetGameObject().GetName()));
        return false;
    }

    if (newFather && newFather->GetGameObject().IsDestroying()) {
        ErrorString(Format("Cannot set the parent of the GameObject '%s' while its new parent '%s' is being destroyed",
                           GetGameObject().GetName(), newFather->GetGameObject().GetName()));
        return false;
    }

    if ((GetParent() && GetParent()->GetGameObject().IsActivating()) ||
        (newFather && newFather->GetGameObject().IsActivating())) {
        GameObject &invalidParent = GetParent() ? GetParent()->GetGameObject() : newFather->GetGameObject();
        ErrorString(
                Format("Cannot set the parent of the GameObject '%s' while activating or deactivating the parent GameObject '%s'.",
                       GetGameObject().GetName(), invalidParent.GetName()));
        return false;
    }

    if ((options & kAllowParentingFromPrefab) == 0) {
        if (IsPrefabAsset() || (newFather && newFather->IsPrefabAsset())) {
            ErrorString(
                    Format("Setting the parent of a transform which resides in a Prefab Asset is disabled to prevent data corruption (GameObject: '%s').",
                           GetGameObject().GetName()));
            return false;
        }
    }

#if UNITY_EDITOR
    bool isPrefabInstance = GetPrefabInstance().GetInstanceID() != InstanceID_None;
    if (isPrefabInstance)
    {
        Transform* prefabInstanceCorrespondingSource = dynamic_pptr_cast<Transform*>(GetCorrespondingSourceObject());
        if (prefabInstanceCorrespondingSource)
        {
            bool isPrefabRoot = &prefabInstanceCorrespondingSource->GetRoot() == prefabInstanceCorrespondingSource;
            if (!isPrefabRoot)
            {
                ErrorStringObject(Format("Setting the parent of a transform which resides in a Prefab instance is not possible (GameObject: '%s').", GetGameObject().GetName()), this);
                return false;
            }
        }
    }
#endif

    // Make sure that the new father is not a child of this transform.
    if (IsChildOrSameTransform(newFather, this))
        return false;

    if ((options & kDisableTransformMessage) == 0) {
        // Send a message before applying any changes that the transform parenting will be changed
        BroadcastMessage(kBeforeTransformParentChanged);
    }

    // Save the old position in worldspace - only used for kWorldPositionStays
    float3 globalPosition;
    float4 globalRotation;
    float3x3 globalScale;
    Vector2f globalRectDims; // Only if RectTransform
    Vector3f globalRectPos; // Only if RectTransform
//    UI::RectTransform* rectTransform = NULL; // Only if RectTransform

    if (options & kWorldPositionStays) {
        TransformAccessReadOnly transformAccess = GetTransformAccess();
        CalculateGlobalPositionAndRotation(transformAccess, globalPosition, globalRotation);
        globalScale = CalculateGlobalSM(transformAccess, globalRotation);

//        if (GetType() == TypeOf<UI::RectTransform>())
//        {
//            rectTransform = static_cast<UI::RectTransform*>(this);
//            rectTransform->GetWorldSpace(globalRectPos, globalRectDims);
//        }
    }

    // UnityScene* previousScene = GetScene();
    Transform *previousFather = m_Father;
    if (previousFather) {
        iterator it = previousFather->Find(this);
        Assert(it != previousFather->end());
        previousFather->m_Children.erase(it);
        previousFather->SetDirty();
    }

    // Needs to be computed before newFather->m_Children is touched.
    UInt32 transformThreadInsertIndex = 0;

    if (newFather) {
        if ((options & kDisableTransformMessage) == 0) {
            // Potential resizing of newFather's hierarchy data has to happen before computing the insertion index.
            if ((previousFather == NULL) ||
                (previousFather->m_TransformData.hierarchy != newFather->m_TransformData.hierarchy)) {
                UInt32 nodeCount = GetDeepChildCount(m_TransformData);
                newFather->EnsureCapacityIncrease(nodeCount);
            }

            // Now it is safe to compute the insertion index.
            transformThreadInsertIndex = newFather->FindLastChildIndex();
        }

        // When setting new parent we intentionally add to last (so gui elements are rendered topmost: we render last items last)
        newFather->m_Children.push_back(this);
        newFather->SetDirty();
    }

    m_Father = newFather;
//    if (previousFather == NULL)
//    {
//        // From root to non-root: remove it from the scene's root list
//        UnityScene::RemoveRootFromScene(*this, true);
//    }
//    else if (newFather == NULL)
//    {
//        // From non-root to root: add it to the scene's root list (Note: it might not have been part of a scene since HideAndDontSave transforms in
//        // the editor are not part of any scene.
//        // This needs to happen before any callbacks are performed, because they might inadvertently change the scene and get called with invalid
//        // scene data in edge-cases, see case 1275898.
//        // Also note that the scene this transform ultimately ends up with could also be changing in the below "SendTransformChanged (kParentingChanged)";
//        if (IsSceneRoot() && previousScene)
//            UnityScene::AddRootToScene(*previousScene, *this);
//    }

    if ((options & kDisableTransformMessage) == 0) {
        TransformHierarchyChangeDispatch::InterestType childHierarchyChanges = TransformHierarchyChangeDispatch::kInterestedInParentHierarchy;

        if (!previousFather && newFather) {
            // Move hierarchy from root.
            UInt32 nodeCount = GetDeepChildCount(m_TransformData);
            TransformHierarchy *oldHierarchy = m_TransformData.hierarchy;
            TransformHierarchy *newHierarchy = newFather->m_TransformData.hierarchy;
            UInt32 newFirst = 0, newLast = 0;
            AddTransformSubhierarchy(*oldHierarchy, 0, *newHierarchy, newFirst, newLast,
                                     GetCloneChangeSystemInterestsMask(), TransformChangeSystemMask(0),
                                     GetCloneHierarchyChangeSystemInterestsMask(), false);
            InsertTransformThreadAfter(*newHierarchy, transformThreadInsertIndex, newFirst, newLast);
            UpdateDeepChildCountUpwards(*newHierarchy, newFather->m_TransformData.index, nodeCount);
            UpdateTransformAccessors(*newHierarchy, newFirst);
            DestroyTransformHierarchy(oldHierarchy);
            childHierarchyChanges |= TransformHierarchyChangeDispatch::kInterestedInTransformAccess;
        } else if (previousFather && !newFather) {
            // Move subhierarchy to root.
            UInt32 nodeCount = GetDeepChildCount(m_TransformData);
            TransformHierarchy *oldHierarchy = m_TransformData.hierarchy;
            UInt32 oldFirst = m_TransformData.index;
            UInt32 oldLast = FindLastChildIndex();
            TransformHierarchy *newHierarchy = CreateTransformHierarchy(nodeCount/*, GetMemoryLabel()*/);
            CopyTransformSubhierarchy(*oldHierarchy, oldFirst, *newHierarchy, GetCloneChangeSystemInterestsMask(),
                                      TransformChangeSystemMask(0), GetCloneHierarchyChangeSystemInterestsMask(),
                                      false);
            DetachTransformThread(*oldHierarchy, oldFirst, oldLast);
            FreeTransformThread(*oldHierarchy, oldFirst, oldLast);
            UpdateDeepChildCountUpwards(*oldHierarchy, previousFather->m_TransformData.index, -(SInt32) nodeCount);
            UpdateTransformAccessors(*newHierarchy, 0);
            childHierarchyChanges |= TransformHierarchyChangeDispatch::kInterestedInTransformAccess;
        } else if (previousFather->m_TransformData.hierarchy == newFather->m_TransformData.hierarchy) {
            // Move subhierarchy within hierarchy.
            UInt32 nodeCount = GetDeepChildCount(m_TransformData);
            TransformHierarchy *hierarchy = m_TransformData.hierarchy;
            UInt32 first = m_TransformData.index;
            UInt32 last = FindLastChildIndex();
            DetachTransformThread(*hierarchy, first, last);
            UpdateDeepChildCountUpwards(*hierarchy, previousFather->m_TransformData.index, -(SInt32) nodeCount);
            InsertTransformThreadAfter(*hierarchy, transformThreadInsertIndex, first, last);
            UpdateDeepChildCountUpwards(*hierarchy, newFather->m_TransformData.index, nodeCount);
            UpdateTransformAccessors(*hierarchy, first);
        } else {
            // Move subhierarchy between hierarchies.
            UInt32 nodeCount = GetDeepChildCount(m_TransformData);
            TransformHierarchy *oldHierarchy = m_TransformData.hierarchy;
            UInt32 oldFirst = m_TransformData.index;
            UInt32 oldLast = FindLastChildIndex();
            TransformHierarchy *newHierarchy = newFather->m_TransformData.hierarchy;
            UInt32 newFirst = 0, newLast = 0;
            AddTransformSubhierarchy(*oldHierarchy, m_TransformData.index, *newHierarchy, newFirst, newLast,
                                     GetCloneChangeSystemInterestsMask(), TransformChangeSystemMask(0),
                                     GetCloneHierarchyChangeSystemInterestsMask(), false);
            DetachTransformThread(*oldHierarchy, oldFirst, oldLast);
            UpdateDeepChildCountUpwards(*oldHierarchy, previousFather->m_TransformData.index, -(SInt32) nodeCount);
            FreeTransformThread(*oldHierarchy, oldFirst, oldLast);
            InsertTransformThreadAfter(*newHierarchy, transformThreadInsertIndex, newFirst, newLast);
            UpdateDeepChildCountUpwards(*newHierarchy, newFather->m_TransformData.index, nodeCount);
            UpdateTransformAccessors(*newHierarchy, newFirst);
            childHierarchyChanges |= TransformHierarchyChangeDispatch::kInterestedInTransformAccess;
        }

        // Force mark TRS dirty
        // OnTransformChangedMask(GetTransformAccess(), TransformChangeSystemMask(0), GetTransformChangeDispatch().GetChangeMaskForInterest(TransformChangeDispatch::kInterestedInGlobalTRS | TransformChangeDispatch::kInterestedInParentHierarchy), TransformChangeSystemMask(0));
        QueueChanges();

#if !UNITY_RELEASE
        GetRoot().ValidateHierarchy(*GetRoot().m_TransformData.hierarchy);
        if (previousFather)
            previousFather->GetRoot().ValidateHierarchy(*previousFather->GetRoot().m_TransformData.hierarchy);
#endif

        if (options & kWorldPositionStays) {
            TransformAccess transformAccess = GetTransformAccess();
            // Restore old position so they stay at the same position in worldspace
            SetGlobalMatrixLossy(transformAccess, globalPosition, globalRotation, globalScale);

//            if (rectTransform)
//                rectTransform->SetWorldSpace(globalRectPos, globalRectDims);
        }

        QueueChanges();

        // Dispatch transform hierarchy changes.
        GetTransformHierarchyChangeDispatch().DispatchSelfAndAllChildren(GetTransformAccess(), childHierarchyChanges);
//        if (previousFather != NULL)
//            GetTransformHierarchyChangeDispatch().DispatchSelfAndParents(previousFather->GetTransformAccess(), TransformHierarchyChangeDispatch::kInterestedInChildHierarchy);
//        if (newFather != NULL)
//            GetTransformHierarchyChangeDispatch().DispatchSelfAndParents(newFather->GetTransformAccess(), TransformHierarchyChangeDispatch::kInterestedInChildHierarchy);

        // Send message to this and all children that parent has changed.
        SendTransformParentChanged();

        // Send msg to new and old direct parent.
        // That it's direct children have changed.
        if (previousFather != NULL)
            previousFather->SendMessage(kTransformChildrenChanged);
        if (newFather != NULL)
            newFather->SendMessage(kTransformChildrenChanged);
    } else {
        Assert(m_TransformData.hierarchy == NULL);
        Assert(previousFather == NULL || previousFather->m_TransformData.hierarchy == NULL);
        Assert(newFather == NULL || newFather->m_TransformData.hierarchy == NULL);
    }

//    UnityScene* currentScene = GetScene();
//    if (currentScene != previousScene)
//        UnityScene::OnGameObjectChangedScene(GetGameObject(), currentScene, previousScene);

#if HUAHUO_EDITOR
    gHierarchyChangedSetParentCallback.Invoke(this, previousFather, newFather);
#endif

    SetDirty();

    return true;
}

bool Transform::IsSceneRoot() const
{
    if (GetParent() != NULL)
        return false;

    if (IsPersistent())
        return false;

#if UNITY_EDITOR
    // Root GameObjects in Preview Scenes are always regarded as root and kept in the Preview Scene regardless of HideFlags.
    // For normal scenes we remove gameobject from the scene if they are one of the kNoBelongToScene below.
    if (m_SceneRootNode.m_ListNode.IsInList())
    {
        if (m_SceneRootNode.m_UnityScene && m_SceneRootNode.m_UnityScene->IsPreviewScene())
            return true;
    }

    // Here is very tricky:
    // There are 2 special cases:
    // 1. kDontSaveInEditor + !kHideInHierarchy
    //      It should be in the hierarchy window, but is not supposed to be saved into
    //      the scene file. It needs to be recorded in the scene because that's the only way it can be show
    //      in the hierarchy window.
    // 2. !kDontSaveInEditor + kHideInHierarchy
    //      It is not in the hierarchy window, but is supposed to be saved into the scene file.
    //      We have to keep track of it.
    const Object::HideFlags kNoBelongToScene = kDontSaveInEditor | kDontSaveInBuild | kHideInHierarchy;
    if (TestHideFlag(kNoBelongToScene))
        return false;
#endif

    return true;
}

HuaHuoScene* Transform::GetScene()
{
    // PROFILER_AUTO(gGetScene);

    if (m_SceneRootNode.m_ListNode.IsInList())
        return m_SceneRootNode.m_Scene;

    Transform& root = GetRoot();
    return root.GetSceneRootNode().m_Scene;
}

TransformChangeSystemMask GetCloneChangeSystemInterestsMask() {
    return 0;
    // return GetTransformChangeDispatch().GetPermanentInterestMask();
}

Matrix4x4f Transform::GetWorldToLocalMatrixNoScale() const
{
    RETURN_AFFINE4X4(CalculateInverseGlobalMatrixNoScale(GetTransformAccess()));
}

Vector3f Transform::InverseTransformDirection(const Vector3f& inDirection) const
{
    RETURN_VECTOR3(::InverseTransformDirection(GetTransformAccess(), math::vload3f(inDirection.GetPtr())));
}

Vector3f Transform::TransformDirection(const Vector3f& inDirection) const
{
    RETURN_VECTOR3(::TransformDirection(GetTransformAccess(), math::vload3f(inDirection.GetPtr())));
}

TransformChangeSystemMask GetCloneChangeSystemChangesMask() {
    return gHasChangedDeprecatedSystem.Mask();
}

UInt32 GetCloneHierarchyChangeSystemInterestsMask() {
    return 0;
    // return GetTransformHierarchyChangeDispatch().GetPermanentInterestMask();
}

Quaternionf Transform::GetRotation() const
{
#if UNITY_EDITOR
    if (!IsTransformHierarchyInitialized())
    {
        ErrorStringObject("Illegal transform access. Are you accessing a transform rotation from OnValidate?\n", this);
        return Quaternionf::identity();
    }
#endif

    RETURN_QUATERNION(CalculateGlobalRotation(GetTransformAccess()));
}


IMPLEMENT_OBJECT_SERIALIZE(Transform);

INSTANTIATE_TEMPLATE_TRANSFER(Transform);

IMPLEMENT_REGISTER_CLASS(Transform, 3);

INSTANTIATE_TEMPLATE_TRANSFER_FUNCTION(Transform, CompleteTransformTransfer);