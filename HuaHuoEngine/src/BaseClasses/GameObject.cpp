//
// Created by VincentZhang on 4/28/2022.
//

#include "GameObject.h"
#include "Components/BaseComponent.h"
#include "Components/Transform/Transform.h"
#include "Components/Transform/TransformHierarchy.h"
#include "Utilities/RegisterRuntimeInitializeAndCleanup.h"
#include "Components/Transform/TransformChangeDispatch.h"

void GameObject::InitializeClass()
{
    GameObjectManager::StaticInitialize();
}

void GameObject::CleanupClass()
{
    GameObjectManager::StaticDestroy();
}

void GameObject::ActivateAwakeRecursivelyInternal(DeactivateOperation deactivateOperation/*, AwakeFromLoadQueue &queue*/)
{
    if (IsActivating())
    {
        ErrorStringObject("GameObject is already being activated or deactivated.", this);
        return;
    }
    bool state;
    bool changed;
    if (m_IsActiveCached != -1)
    {
        bool oldState = m_IsActiveCached;
        m_IsActiveCached = -1;
        state = IsActive();
        changed = oldState != state;
    }
    else
    {
        state = IsActive();
        changed = true;
    }

    m_ActivationState = state ? kActivatingChildren : kDeactivatingChildren;

    Transform *transform = QueryComponent<Transform>();
    if (transform)
    {
        // use a loop by index rather than a iterator, as the children can adjust
        // the child list during the Awake call, and invalidate the iterator
        for (int i = 0; i < transform->GetChildrenCount(); i++)
            transform->GetChild(i).GetGameObject().ActivateAwakeRecursivelyInternal(deactivateOperation/*, queue*/);
    }

    if (changed)
    {
        m_ActivationState = state ? kActivatingComponents : kDeactivatingComponents;
        for (size_t i = 0; i < m_Component.size(); i++)
        {
            BaseComponent& component = *m_Component[i].GetComponentPtr();
            if (state)
            {
                Assert(&*component.m_GameObject == this);
                component.SetGameObjectInternal(this);
                // queue.Add(*m_Component[i].GetComponentPtr());
            }
            else
                component.Deactivate(deactivateOperation);
        }

        if (state)
            UpdateActiveGONode();
        else
            m_ActiveGONode.RemoveFromList();
    }
    m_ActivationState = kNotActivating;
}


void GameObject::ActivateAwakeRecursively(DeactivateOperation deactivateOperation)
{
    // PROFILER_AUTO(gActivateAwakeRecursively, this);

    // AwakeFromLoadQueue queue(kMemTempAlloc);
    ActivateAwakeRecursivelyInternal(deactivateOperation/*, queue*/);
    //queue.AwakeFromLoad(kActivateAwakeFromLoad);
}

void GameObject::Activate()
{
    if (IsActive())
        return;

    // PROFILER_AUTO(gActivateGameObjectProfiler, this);

    if (IsDestroying())
    {
        ErrorStringObject("GameObjects can not be made active when they are being destroyed.", this);
        return;
    }

    SetDirty();

    ActivateInternal();
    ActivateAwakeRecursively();
    // After AwakeFromLoad, 'this' could have been destroyed (if user is Destroying in OnEnable or Awake)
    // So do not access it any further
}

GameObject::GameObject( /*MemLabelId label, */ ObjectCreationMode mode)
        : Super( /*label,*/ mode),
//          m_Component(label),
          m_ActiveGONode(this)
{
//    m_SupportedMessages = 0;
    m_ActivationState = kNotActivating;
    m_Tag = 0;
    m_IsActive = false;
    m_IsActiveCached = -1;

#if UNITY_EDITOR
    m_IsOldVersion = false;
    m_StaticEditorFlags = 0;
    m_MarkedVisible = kSelfVisible;
    m_PrefabInstanceHiddenForInContextEditing = false;
#endif
}

void GameObject::SetName(char const* name)
{
    m_Name.assign(name);
//    if (s_SetGONameCallback)
//        s_SetGONameCallback(this);
    SetDirty();
}

void GameObject::AddComponentInternal(BaseComponent* com, bool awake /*, AwakeFromLoadQueue* queue*/)
{
    Assert(com != NULL);
    m_Component.push_back(ComponentPair::FromComponent(com));
    FinalizeAddComponentInternal(com, awake/*, queue*/);
}

void GameObject::SetHideFlags(HideFlags flags)
{
    Super::SetHideFlags(flags);
    for (size_t i = 0; i < m_Component.size(); i++)
    {
        BaseComponent& com = *m_Component[i].GetComponentPtr();
        com.SetHideFlags(flags);
    }
}

void GameObject::UpdateActiveGONode() {
    m_ActiveGONode.RemoveFromList();
    bool canAddToList = IsActive();

#if UNITY_EDITOR
    //Preview Scene Objects should not be able to be found using
    //the FindGameObject(s)WithTag APIs
    canAddToList &= !IsPreviewSceneObject();
#endif

    if (canAddToList) {
        if (m_Tag != 0)
            GetGameObjectManager().GetTaggedNodes(m_Tag).push_back(m_ActiveGONode);
        else
            GetGameObjectManager().m_ActiveNodes.push_back(m_ActiveGONode);
    }
}


void GameObject::Reset()
{
    Super::Reset();
    m_Layer = kDefaultLayer;
    m_Tag = 0;
#if UNITY_EDITOR
    m_StaticEditorFlags = 0;
    m_TagString = GetTagManager().TagToString(m_Tag);
    m_NavMeshLayer = 0;
#endif
}

void GameObject::SetLayer(int layer)
{
    if (layer >= 0 && layer < 32)
    {
        m_Layer = layer;
        // SendMessage(kLayerChanged);
        SetDirty();
    }
    else
        ErrorString("A game object can only be in one layer. The layer needs to be in the range [0...31]");
}

void GameObject::AwakeFromLoad(AwakeFromLoadMode awakeMode)
{
    Super::AwakeFromLoad(awakeMode);

    if (ShouldClearActiveCached(awakeMode))
        ClearActiveCachedInternal();

    // SetSupportedMessagesDirty();
    UpdateActiveGONode();

//    // There is no need to invoke this callback if the GameObject were loaded from disk
//    // it would simply cause unnecesarry rebuilding of the HierarchyView because the
//    // transform hierarchy is set dirty
//    if (awakeMode != kPersistentManagerAwakeFromLoadMode && s_SetGONameCallback != nullptr)
//        s_SetGONameCallback(this);

#if UNITY_EDITOR
    // When we are modifying the game object active state from the inspector
    // We need to Activate / Deactivate the relevant components
    // This never happens in the player.
    if (awakeMode == kDefaultAwakeFromLoad)
        ActivateAwakeRecursively();
#endif
}


bool GameObject::IsActive() const
{
    if (m_IsActiveCached != -1)
        return m_IsActiveCached;

    // Calculate active state based on the hierarchy
    m_IsActiveCached = m_IsActive && !IsPersistent();
    Transform *trs = QueryComponent<Transform>();
    if (trs)
    {
        Transform *parent = trs->GetParent();
        if (parent && parent->GetGameObjectPtr())
        {
            m_IsActiveCached = m_IsActiveCached && parent->GetGameObjectPtr()->IsActive();
        }
    }

    return m_IsActiveCached;
}

void GameObject::FinalizeAddComponentInternal(BaseComponent* com, bool awake = true/*, AwakeFromLoadQueue* queue*/)
{
    // Make sure it isn't already added to another GO
    Assert(com->GetGameObject().GetInstanceID() == InstanceID_None || com->GetGameObjectPtr() == this);

    com->SetGameObjectInternal(this);
    com->SetHideFlags(GetHideFlags());

    // SetSupportedMessagesDirty();

#if UNITY_EDITOR
    if (queue)
        queue->Add(*com);
    else if (awake)
#endif
    {
        if (IsActive())
            com->AwakeFromLoad(kActivateAwakeFromLoad);
        else
            com->AwakeFromLoad(kDefaultAwakeFromLoad);
    }

    com->SetDirty();
    SetDirty();
}

BaseComponent* GameObject::QueryComponentByType(const HuaHuo::Type* type) const
{
    // Find a component with the requested ID
    Container::const_iterator i;
    Container::const_iterator end = m_Component.end();
    for (i = m_Component.begin(); i != end; ++i)
    {
        if (type->IsBaseOf(i->GetTypeIndex()))
            return i->GetComponentPtr();
    }

    return NULL;
}

BaseComponent* GameObject::QueryComponentByExactType(const HuaHuo::Type* type) const
{
    // Find a component with the requested ID
    Container::const_iterator i;
    Container::const_iterator end = m_Component.end();
    RuntimeTypeIndex index = type->GetRuntimeTypeIndex();
    for (i = m_Component.begin(); i != end; ++i)
    {
        if (i->GetTypeIndex() == index)
            return i->GetComponentPtr();
    }

    return NULL;
}


GameObjectManager* GameObjectManager::s_Instance = NULL;
void GameObjectManager::StaticInitialize()
{
    Assert(GameObjectManager::s_Instance == NULL);
    GameObjectManager::s_Instance = NEW(GameObjectManager/*, kMemBaseObject*/);
}

void GameObjectManager::StaticDestroy()
{
    Assert(GameObjectManager::s_Instance);
    DELETE(GameObjectManager::s_Instance /*, kMemBaseObject*/);
}

GameObjectManager& GetGameObjectManager()
{
    Assert(GameObjectManager::s_Instance);
    return *GameObjectManager::s_Instance;
}

//Must not be called during consistency check as GameObjects get processed first in the queue
static void DestroyComponentImmediate(BaseComponent* component)
{
    Assert(component != NULL);

#if UNITY_EDITOR
    if (IsPrefabInstanceWithValidParent(component))
        DisconnectPrefabInstance(component);
#endif
    // Destroy the object but not before we force (fake) flagging that it was correctly awoken which of course it hasn't yet.
    component->SetAwakeCalledInternal();
    component->SetAwakeDidLoadThreadedCalledInternal();
    DestroySingleObject(component);
}

void GameObject::AddFirstTransformComponentInternal(::Transform* newTransform/*, AwakeFromLoadQueue* queue = nullptr*/)
{
    // Add transform as first component.
    Assert(newTransform != NULL);
    Assert(QueryComponent<Transform>() == NULL);
    m_Component.insert(m_Component.begin(), ComponentPair::FromComponent(newTransform));
    FinalizeAddComponentInternal(newTransform/*, queue*/);
}

void GameObject::ReplaceTransformComponentInternal(Transform* newTransform /*, AwakeFromLoadQueue* queue*/)
{
    Assert(newTransform != NULL);
    Assert(newTransform->GetParentPtrInternal() == NULL);
    Assert(newTransform->GetChildrenCount() == 0);

    Transform* oldTransform = QueryComponentAtIndex<Transform>(0);
    oldTransform->EnsureTransformHierarchyExists();

#if UNITY_EDITOR
    // Transfer root order.
    newTransform->SetRootOrder(oldTransform->GetRootOrder());
#endif

    // Remove the old transform from its parent.
    Transform* parent = oldTransform->GetParent();
    if (parent != NULL)
    {
        // Replace old transform with new transform in parent's list of children
        Transform::iterator i = parent->Find(oldTransform);
        *i = newTransform;
        newTransform->SetParentPtrInternal(parent);
        oldTransform->SetParentPtrInternal(NULL);
    }
    else
    {
//        // Ensure new transform has the same scene as old transform
//        UnityScene* scene = oldTransform->GetScene();
//        if (scene != NULL)
//        {
//            UnityScene::RemoveRootFromScene(*oldTransform, true);
//            UnityScene::AddRootToScene(*scene, *newTransform);
//        }
    }

    // Move the source children so they are children of the target instead.
    newTransform->GetChildrenInternal().swap(oldTransform->GetChildrenInternal());
    Transform::TransformComList& children = newTransform->GetChildrenInternal();
    for (int index = 0; index < children.size(); ++index)
        children[index]->SetParentPtrInternal(newTransform);

    // Update TransformHierarchy with new transform
    TransformAccess access = oldTransform->GetTransformAccess();
    access.hierarchy->mainThreadOnlyTransformPointers[access.index] = newTransform;
    newTransform->m_TransformData = access;
    m_Component[0] = ComponentPair::FromComponent(newTransform);
    oldTransform->m_GameObject = NULL;
    oldTransform->m_TransformData.hierarchy = NULL;

    // Ensure new transform's properties match the TransformHierarchy's values
    newTransform->ApplyRuntimeToSerializedData();
//    if (newTransform->GetType() == TypeOf<UI::RectTransform>())
//    {
//        const Vector3f localPosition = newTransform->GetLocalPosition();
//        Vector2f anchoredPosition(localPosition.x, localPosition.y);
//        static_cast<UI::RectTransform*>(newTransform)->SetAnchoredPositionWithoutNotification(anchoredPosition);
//    }

    FinalizeAddComponentInternal(newTransform/*, queue*/);

    DestroyComponentImmediate(oldTransform);

    // newTransform->RegisterChangeSystemInterests();

    //GetTransformHierarchyChangeDispatch().DispatchSelfOnly(access, TransformHierarchyChangeDispatch::kInterestedInReplacement);

//#if !UNITY_RELEASE
//    newTransform->GetRoot().ValidateHierarchy(*newTransform->GetTransformAccess().hierarchy);
//#endif
}

template<class TransferFunction>
void GameObject::TransferComponents(TransferFunction& transfer)
{
    // When cloning objects for prefabs and instantiate, we don't use serialization to duplicate the hierarchy,
    // we duplicate the hierarchy directly
    if (!SerializePrefabIgnoreProperties(transfer))
        return;

#if UNITY_EDITOR
    if (transfer.IsWriting() && transfer.NeedsInstanceIDRemapping())
    {
        Container filtered_components(kMemTempAlloc);
        for (Container::iterator i = m_Component.begin(); i != m_Component.end(); i++)
        {
            LocalSerializedObjectIdentifier localIdentifier;
            if (transfer.IsWritingGameReleaseData())
            {
                // If the component type is not in a disabled module, don't write the component.
                if (ModuleMetadata::Get().GetModuleIncludeSettingForClass(i->GetComponentPtr()->GetType()) == kForceExclude)
                    continue;
            }

            InstanceIDToLocalSerializedObjectIdentifier(i->GetComponentPtr()->GetInstanceID(), localIdentifier);
            if (localIdentifier.localIdentifierInFile != 0)
                filtered_components.push_back(*i);
        }
        transfer.Transfer(filtered_components, "m_Component", kHideInEditorMask | kStrongPPtrMask | kDisallowSerializedPropertyModification);
        return;
    }
#endif

    transfer.Transfer(m_Component, "m_Component", kHideInEditorMask | kStrongPPtrMask | kDisallowSerializedPropertyModification);

#if !UNITY_EDITOR
    bool shownWarning = false;
    for (Container::iterator i = m_Component.begin(); i != m_Component.end();)
    {
        if (i->GetComponentPtr() == nullptr)
        {
            if (!shownWarning)
            {
                WarningStringMsg("GameObject contains a component type that is not recognized");
                shownWarning = true;
            }

            i = m_Component.erase(i);
        }
        else
        {
            i++;
        }
    }
#endif
}

template<class TransferFunction>
void GameObject::Transfer(TransferFunction& transfer)
{
    Super::Transfer(transfer);
    TransferComponents(transfer);

    TRANSFER(m_Layer);

#if !UNITY_EDITOR
    transfer.Transfer(m_Name, "m_Name");
    TRANSFER(m_Tag);
    transfer.Transfer(m_IsActive, "m_IsActive");
#elif UNITY_EDITOR
    if (transfer.IsVersionSmallerOrEqual(3))
        m_IsOldVersion = true;

    if (transfer.IsVersionSmallerOrEqual(1))
    {
        TRANSFER(m_Tag);
        m_TagString = GetTagManager().TagToString(m_Tag);
        transfer.Transfer(m_IsActive, "m_IsActive");
    }
    else if (transfer.IsVersionSmallerOrEqual(2))
    {
        TRANSFER(m_TagString);
        m_Tag = GetTagManager().StringToTag(m_TagString);
        transfer.Transfer(m_IsActive, "m_IsActive");
    }
    else
    {
        transfer.Transfer(m_Name, "m_Name");

        if (transfer.IsSerializingForGameRelease())
        {
            TRANSFER(m_Tag);
            if (transfer.IsReading())
                m_TagString = GetTagManager().TagToString(m_Tag);

            transfer.Transfer(m_IsActive, "m_IsActive");
        }
        else
        {
            transfer.Transfer(m_TagString, "m_TagString");
            if (transfer.IsReading())
                m_Tag = GetTagManager().StringToTagAddIfUnavailable(m_TagString);

            transfer.Transfer(m_Icon, "m_Icon", kNoTransferFlags);
            transfer.Transfer(m_NavMeshLayer, "m_NavMeshLayer", kHideInEditorMask);

            transfer.Transfer(m_StaticEditorFlags, "m_StaticEditorFlags", kNoTransferFlags | kGenerateBitwiseDifferences);

            // Read deprecated static flag and set it up as m_StaticEditorFlags
            if (transfer.IsReadingBackwardsCompatible())
            {
                bool isStatic = false;
                transfer.Transfer(isStatic, "m_IsStatic", kNoTransferFlags);
                if (isStatic)
                    m_StaticEditorFlags = 0xFFFFFFFF;
            }

            if (transfer.IsReading() && transfer.IsVersionSmallerOrEqual(5))
            {
                // Prior Unity 2018.1, GameObjects that had Lightmap Static flag enabled where automatically baked in the Reflection Probes.
                // Now, Reflection Probe Static flag is treated sperately and used by BakeReflectionProbeManager to bake the objects in Reflection Probes.
                // When upgrading projects, enable Reflection Probe Static flag when Lightmap Static flag is also set, so we don't break old projects.
                if (m_StaticEditorFlags & kLightmapStatic)
                    m_StaticEditorFlags = m_StaticEditorFlags | kReflectionProbeStatic;
            }

            transfer.Transfer(m_IsActive, "m_IsActive", kHideInEditorMask);
        }
    }

    // Make sure that old prefabs are always active.
    if (transfer.IsVersionSmallerOrEqual(3) && transfer.IsLoadingPrefabAsScene())
        m_IsActive = true;
#endif
}

template<class TransferFunction>
void GameObject::ComponentPair::Transfer(TransferFunction& transfer)
{
    transfer.Transfer(component, "component");
    if (transfer.IsReadingPPtr())
    {
        typeIndex = component ? component->GetType()->GetRuntimeTypeIndex() : 0;
    }
}


IMPLEMENT_OBJECT_SERIALIZE(GameObject);
IMPLEMENT_REGISTER_CLASS(GameObject, 1);
INSTANTIATE_TEMPLATE_TRANSFER(GameObject);
