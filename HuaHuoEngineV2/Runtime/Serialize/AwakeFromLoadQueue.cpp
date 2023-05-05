#include <stdlib.h>
#include "AwakeFromLoadQueue.h"

#if UNITY_EDITOR
#include "Editor/Src/Prefabs/PrefabInstance.h"
#include "Editor/Src/Prefabs/PrefabBackwardsCompatibility.h"
#include "Editor/Src/Utility/DanglingComponentManager.h"
#endif

AwakeFromLoadQueue::AwakeFromLoadQueue(MemLabelId label)
{
//    for (int i = 0; i < kMaxQueues; ++i)
//        m_ItemArrays[i].set_memory_label(label);
}

void AwakeFromLoadQueue::Reserve(unsigned size)
{
    for (int i = 0; i < kMaxQueues; i++)
    {
        if (i == kManagersQueue)
            continue;

        m_ItemArrays[i].reserve(size);
    }
}

void AwakeFromLoadQueue::RegisterObjectInstanceIDs()
{
    // SetObjectLockForWrite();
    for (int i = 0; i < kMaxQueues; i++)
        RegisterObjectInstanceIDsInternal(m_ItemArrays[i].begin().base(), m_ItemArrays[i].size());
    // ReleaseObjectLock();
}

void AwakeFromLoadQueue::RegisterObjectInstanceIDsInternal(Item* objects, unsigned size)
{
    for (size_t i = 0; i < size; i++)
    {
        Object* ptr = objects[i].registerObjectPtr;
        Assert(ptr != NULL);
        Object::RegisterInstanceIDNoLock(ptr);
    }
}

void AwakeFromLoadQueue::InitializeClass()
{
//    ExecutionOrderManager &eom = GetExecutionOrderManager();
//    eom.SetDefaultExecutionOrderFor(TypeOf<Animator>(), kAnimatorQueue, ExecutionOrderManager::kExecutionOrderNotSet);
//    eom.SetDefaultExecutionOrderFor(TypeOf<GameObject>(), kGameObjectQueue, ExecutionOrderManager::kExecutionOrderNotSet);
//    eom.SetDefaultExecutionOrderFor(TypeOf<Material>(), kMaterialQueue, ExecutionOrderManager::kExecutionOrderNotSet);
//    eom.SetDefaultExecutionOrderFor(TypeOf<Rigidbody>(), kRigidbodyQueue, ExecutionOrderManager::kExecutionOrderNotSet);
//    eom.SetDefaultExecutionOrderFor(TypeOf<Shader>(), kShaderQueue, ExecutionOrderManager::kExecutionOrderNotSet);
//    eom.SetDefaultExecutionOrderFor(TypeOf<SortingGroup>(), kSortingGroupQueue, ExecutionOrderManager::kExecutionOrderNotSet);
//    eom.SetDefaultExecutionOrderFor(TypeOf<UI::Canvas>(), kCanvasQueue, ExecutionOrderManager::kExecutionOrderNotSet);
//    eom.SetDefaultExecutionOrderFor(WeakTypeOf<CompositeCollider2D>(), kCompositeColliderQueue, ExecutionOrderManager::kExecutionOrderNotSet);
//    eom.SetDefaultExecutionOrderFor(WeakTypeOf<Grid>(), kGridQueue, ExecutionOrderManager::kExecutionOrderNotSet);
//    eom.SetDefaultExecutionOrderFor(WeakTypeOf<ParticleSystem>(), kParticleSystemQueue, ExecutionOrderManager::kExecutionOrderNotSet);
//    eom.SetDefaultExecutionOrderFor(WeakTypeOf<ParticleSystemForceField>(), kParticleSystemForceFieldQueue, ExecutionOrderManager::kExecutionOrderNotSet);
//    eom.SetDefaultExecutionOrderFor(WeakTypeOf<Rigidbody2D>(), kRigidbodyQueue, ExecutionOrderManager::kExecutionOrderNotSet);
//    eom.SetDefaultExecutionOrderFor(WeakTypeOf<TerrainData>(), kTerrainsQueue, ExecutionOrderManager::kExecutionOrderNotSet);
//    eom.SetDefaultExecutionOrderFor(WeakTypeOf<Tilemap>(), kTilemapQueue, ExecutionOrderManager::kExecutionOrderNotSet);
//    eom.SetDefaultExecutionOrderFor(WeakTypeOf<WindZone>(), kParticleSystemForceFieldQueue, ExecutionOrderManager::kExecutionOrderNotSet);
}

int AwakeFromLoadQueue::DetermineQueueIndex(const HuaHuo::Type* type, Object& obj)
{
//    const int customQueue = GetExecutionOrderManager().GetCustomAwakeFromLoadQueueFor(type);
//    if (customQueue != kMaxQueues)
//        return customQueue;
//
//    if (type->IsDerivedFrom<Transform>())
//        return kTransformQueue;
//
//    if (IManagedObjectHost::IsTypeAHost(*type))
//        return kScriptedObjectQueue;
//    else if (type->IsDerivedFrom(WeakTypeOf<Collider2D>()) || type->IsDerivedFrom(WeakTypeOf<Collider>())) // TODO(svr): remove dependency on WeakTypeOf
//        return kColliderQueue;
//    else if (type->IsDerivedFrom<Unity::Component>())
//        return kComponentQueue;
//    else if (type->IsDerivedFrom<GameManager>())
//        return kManagersQueue;
//    else if (type->IsDerivedFrom<Texture>())
//        return kTextureQueue;
//#if UNITY_EDITOR
//    else if (type->IsDerivedFrom<PrefabInstance>())
//        return kPrefabQueue;
//#endif
//    else
        return kAssetQueue;
}

bool AwakeFromLoadQueue::IsInQueue(Object& target)
{
    int queueIndex = DetermineQueueIndex(target.GetType(), target);

    for (size_t i = 0; i < m_ItemArrays[queueIndex].size(); i++)
    {
        if (m_ItemArrays[queueIndex][i].objectPPtr == PPtr<Object>(target.GetInstanceID()))
            return true;
    }

    return false;
}

void AwakeFromLoadQueue::Add(Object& target, const TypeTree* oldType, bool safeBinaryLoaded, AwakeFromLoadMode awakeOverride)
{
    Item item;
    item.registerObjectPtr = &target;
    item.objectPPtr = &target;
    item.type = target.GetType();
    // #if UNITY_EDITOR
    item.oldType = oldType;
    item.safeBinaryLoaded = safeBinaryLoaded;
    item.awakeModeOverride = awakeOverride;
    // #else
    // Assert(awakeOverride == -1);
    // #endif

    int queueIndex = DetermineQueueIndex(item.type, target);

    m_ItemArrays[queueIndex].push_back(item);
}

#if UNITY_EDITOR
void AwakeFromLoadQueue::AddMissingComponents()
{
    Item* items = m_ItemArrays[kGameObjectQueue].begin();
    size_t count = m_ItemArrays[kGameObjectQueue].size();
    for (size_t j = 0; j < count; j++)
    {
        Object* ptr = items[j].objectPPtr;
        if (ptr)
        {
            GameObject* go = (GameObject*)ptr;
            for (size_t index = 0; index < go->GetComponentCount(); ++index)
            {
                Unity::Component* com = go->GetComponentPtrAtIndex(index);
                if (com)
                {
                    ::AddMissingComponents(com, this);
                }
            }
        }
    }
}

#endif

void AwakeFromLoadQueue::PersistentManagerAwakeFromLoad()
{
#if UNITY_EDITOR
    // Make sure that transform structures are valid before creating hierarchy data.
    InvokeCheckConsistency(m_ItemArrays[kGameObjectQueue].begin(), m_ItemArrays[kGameObjectQueue].size());
    InvokeCheckConsistency(m_ItemArrays[kTransformQueue].begin(), m_ItemArrays[kTransformQueue].size());
    InvokeEnsureUniqueTransform(m_ItemArrays[kGameObjectQueue].begin(), m_ItemArrays[kGameObjectQueue].size());

    AddMissingComponents();
#endif

    AwakeFromLoadMode awakeMode = (AwakeFromLoadMode)(kPersistentManagerAwakeFromLoadMode);
    for (int i = 0; i < kMaxQueues; i++)
    {
        bool checkConsistency = true;
#if UNITY_EDITOR
        checkConsistency = i != kGameObjectQueue && i != kTransformQueue;
#endif
        PersistentManagerAwakeFromLoad(i, awakeMode, checkConsistency);
    }
}

void AwakeFromLoadQueue::ClearQueue(int queueIndex)
{
    m_ItemArrays[queueIndex].clear();
}

void AwakeFromLoadQueue::Clear()
{
    for (int i = 0; i < kMaxQueues; i++)
        ClearQueue(i);
}

static bool SortItemByInstanceID(const AwakeFromLoadQueue::Item& lhs, const AwakeFromLoadQueue::Item& rhs)
{
    return lhs.objectPPtr.GetInstanceID() < rhs.objectPPtr.GetInstanceID();
}
//
//static bool SortBehaviourItemByExecutionOrder(const AwakeFromLoadQueue::Item& lhs, const AwakeFromLoadQueue::Item& rhs)
//{
//    const ExecutionOrderManager &m = GetExecutionOrderManager();
//
//    const InstanceID lhsInstanceID = lhs.objectPPtr.GetInstanceID();
//    const InstanceID rhsInstanceID = rhs.objectPPtr.GetInstanceID();
//
//    const int lhsExecutionOrder = m.GetScriptExecutionOrder(
//        lhsInstanceID,
//        lhs.type);
//
//    const int rhsExecutionOrder = m.GetScriptExecutionOrder(
//        rhsInstanceID,
//        rhs.type);
//
//    if (lhsExecutionOrder != rhsExecutionOrder)
//        return lhsExecutionOrder < rhsExecutionOrder;
//    else
//        return false;
//}
//
//static bool SortBehaviourItemByExecutionOrderAndReverseInstanceID(const AwakeFromLoadQueue::Item& lhs, const AwakeFromLoadQueue::Item& rhs)
//{
//    const ExecutionOrderManager &m = GetExecutionOrderManager();
//
//    const InstanceID lhsInstanceID = lhs.objectPPtr.GetInstanceID();
//    const InstanceID rhsInstanceID = rhs.objectPPtr.GetInstanceID();
//
//    const int lhsExecutionOrder = m.GetScriptExecutionOrder(
//        lhsInstanceID,
//        lhs.type);
//
//    const int rhsExecutionOrder = m.GetScriptExecutionOrder(
//        rhsInstanceID,
//        rhs.type);
//
//    if (lhsExecutionOrder != rhsExecutionOrder)
//        return lhsExecutionOrder < rhsExecutionOrder;
//    else
//        return lhsInstanceID > rhsInstanceID;
//}
//
//// Temporarily disable serialization safe check when calling AwakeFromLoad and
//// CheckConsistency, as they are in some cases called during read transfer on
//// the main thread and call into managed code. Like RectTransform::AwakeFromLoad does.
//
//class ScopedDisableSerializationSafeCheck
//{
//    Object* savedHostObject;
//    SerializableManagedRef * savedScriptedObject;
//public:
//    ScopedDisableSerializationSafeCheck()
//    {
//        ThreadAndSerializationSafeCheck::GetObjectBeingTransfered(savedHostObject, savedScriptedObject);
//
//        if (savedHostObject)
//            ThreadAndSerializationSafeCheck::SetObjectBeingTransfered(NULL, NULL);
//    }
//
//    ~ScopedDisableSerializationSafeCheck()
//    {
//        if (savedHostObject)
//            ThreadAndSerializationSafeCheck::SetObjectBeingTransfered(savedHostObject, savedScriptedObject);
//    }
//};

void AwakeFromLoadQueue::PersistentManagerAwakeFromLoad(int queueIndex, AwakeFromLoadMode mode, bool checkConsistency)
{
    Item* array = m_ItemArrays[queueIndex].begin().base();
    size_t size = m_ItemArrays[queueIndex].size();

    std::sort(array, array + size, SortItemByInstanceID);

    if (queueIndex == kScriptedObjectQueue)
    {
//        if (UNITY_EDITOR)
//        {
//            // In the editor the execution order can be changed by the user at any time,
//            // thus we need to sort on load
//            std::stable_sort(array, array + size, SortBehaviourItemByExecutionOrder);
//        }

        ///@TODO: Re-enable this on core/streaming-preloadmanager-refactor branch

        // In the player there are some situations (Found on sein)
        // Where reads happen on the main thread in this case we take this codepath.
        // (LoadAndIntegrateAllPreallocatedObjects -> IntegrateAllThreadedObjects)
        // Since we are reading assets in those cases, sort order does not matter.
        // but of course the instanceID's are not lining up for those assets.
        // So we disable this assert for now. The proper solution is to not use AwakeFromLoadQueue for assets imo.
        /*
        else
        {
            // In the player we write the scene files sorted by execution order
            for (size_t j=1;j<size;j++)
            {
                Assert (!SortBehaviourItemByExecutionOrderAndInstanceID (array[j], array[j-1]));
            }
        }
        */
    }

    // Make sure gameobjects cannot send message during CC, OnValidate or Awake. They are in the very first queue, so most
    // probably the message targets hasn't been processed yet and might be in an invalid state
    ExecutionRestrictions originalRestrictions = kNoRestriction;
    if (queueIndex == kGameObjectQueue)
        originalRestrictions = SetExecutionRestrictions(kDisableSendMessage);

    InvokePersistentManagerAwake(array, size, mode, checkConsistency);

    if (queueIndex == kGameObjectQueue)
        SetExecutionRestrictions(originalRestrictions);
}

//void AwakeFromLoadQueue::EnsureTransformHierarchiesExist()
//{
//#if UNITY_EDITOR
//    // Make sure that transform structures are valid before creating hierarchy data.
//    InvokeCheckConsistency(m_ItemArrays[kGameObjectQueue].begin(), m_ItemArrays[kGameObjectQueue].size());
//    InvokeCheckConsistency(m_ItemArrays[kTransformQueue].begin(), m_ItemArrays[kTransformQueue].size());
//    InvokeEnsureUniqueTransform(m_ItemArrays[kGameObjectQueue].begin(), m_ItemArrays[kGameObjectQueue].size());
//#endif
//
//    Item* items = m_ItemArrays[kTransformQueue].begin();
//    size_t count = m_ItemArrays[kTransformQueue].size();
//
//    for (size_t i = 0; i < count; i++)
//    {
//        Object* ptr = items[i].objectPPtr;
//        if (ptr)
//        {
//            Transform* transform = static_cast<Transform*>(ptr);
//            transform->EnsureTransformHierarchyExists();
//        }
//    }
//}

void AwakeFromLoadQueue::AwakeFromLoad(AwakeFromLoadMode mode, bool checkConsistencyFirst)
{
#if UNITY_EDITOR
    if (checkConsistencyFirst)
    {
        InvokeCheckConsistency(m_ItemArrays[kGameObjectQueue].begin(), m_ItemArrays[kGameObjectQueue].size());
        InvokeCheckConsistency(m_ItemArrays[kTransformQueue].begin(), m_ItemArrays[kTransformQueue].size());
        InvokeEnsureUniqueTransform(m_ItemArrays[kGameObjectQueue].begin(), m_ItemArrays[kGameObjectQueue].size());
        AddMissingComponents();
    }
#else
    AssertMsg(!checkConsistencyFirst, "CheckConsistency is Editor only call!");
#endif

    for (int i = 0; i < kMaxQueues; i++)
    {
        Item* items = m_ItemArrays[i].begin().base();
        size_t count = m_ItemArrays[i].size();

//        if (i == kScriptedObjectQueue)
//        {
//            // For Instantiate we need sort by execution order
//            // The default order in Instantiate is determined by walking the hierarchy and components.
//            // Since instanceIDs are generated negative and decreasing we sort by reverse instanceID for components that do not.
//            std::sort(items, items + count, SortBehaviourItemByExecutionOrderAndReverseInstanceID);
//        }

#if UNITY_EDITOR
        if (checkConsistencyFirst && (i != kGameObjectQueue) && (i != kTransformQueue))
            InvokeCheckConsistency(items, count);
#endif

        InvokeAwakeFromLoad(items, count, mode);
    }
}

#if UNITY_EDITOR

void AwakeFromLoadQueue::InsertAwakeFromLoadQueue(ItemArray& src, ItemArray& dst, AwakeFromLoadMode awakeOverride)
{
    std::sort(src.begin(), src.end(), SortItemByInstanceID);
    std::sort(dst.begin(), dst.end(), SortItemByInstanceID);

    // Inject any non-duplicate elements into the dst array
    int oldDstSize = dst.size();
    int d = 0;
    for (int i = 0; i < src.size(); i++)
    {
        while (d < oldDstSize && dst[d].objectPPtr.GetInstanceID() < src[i].objectPPtr.GetInstanceID())
            d++;

        if (d >= oldDstSize || dst[d].objectPPtr.GetInstanceID() != src[i].objectPPtr.GetInstanceID())
        {
            dst.push_back(src[i]);
        }
        // else -> The object is in the destination array already.
    }
}

void AwakeFromLoadQueue::InsertAwakeFromLoadQueue(AwakeFromLoadQueue& awakeFromLoadQueue, AwakeFromLoadMode awakeOverride)
{
    for (int i = 0; i < kMaxQueues; i++)
        InsertAwakeFromLoadQueue(awakeFromLoadQueue.m_ItemArrays[i], m_ItemArrays[i], awakeOverride);
}

void AwakeFromLoadQueue::SetPersistentForPrefabImporter()
{
    for (int i = 0; i < kMaxQueues; i++)
    {
        Item* items = m_ItemArrays[i].begin();
        size_t count = m_ItemArrays[i].size();

        for (size_t j = 0; j < count; j++)
        {
            Object* ptr = items[j].registerObjectPtr;
            PPtr<Object> objectPPtr = items[j].objectPPtr;
            bool isValid = objectPPtr.IsValid();

            if (ptr && isValid)
            {
                ptr->SetIsPersistent(true);
            }
        }
    }
}

#endif

static AwakeFromLoadMode CalculateAwakeFromLoadMode(const AwakeFromLoadQueue::Item& item, AwakeFromLoadMode awakeMode)
{
#if UNITY_EDITOR
    return (item.awakeModeOverride != kDefaultAwakeFromLoadInvalid) ? item.awakeModeOverride : awakeMode;
#else
    UNUSED(item);
    return awakeMode;
#endif
}

void AwakeFromLoadQueue::InvokePersistentManagerAwake(Item* objects, unsigned size, AwakeFromLoadMode awakeMode, bool checkConsistency)
{
    #if DEBUGMODE
    InstanceID previousInstanceID = InstanceID_None;
    #endif

//    ScopedDisableSerializationSafeCheck disableSerializationSafeCheck;
//
//    // Check consistency of all objects in the queue before we wake up the first one of them.
//    // This avoids problems where one the wake up code for one object may access another object
//    // for which consistency checks have not yet been run (as is the case with transforms).
//    #if UNITY_EDITOR
//    if (checkConsistency)
//    {
//        AutoExecutionRestriction restriction(kDisableImmediateDestruction | kDisableSendMessage | kDisableRendering);
//        for (size_t i = 0; i < size; ++i)
//        {
//            Object* ptr = objects[i].objectPPtr;
//            if (ptr == NULL)
//                continue;
//
//            PROFILER_AUTO(gAwakeFromLoadQueue_CheckConsistency, ptr);
//            ptr->CheckConsistency();
//        }
//        DanglingComponentManager::GetInstance().ResolveDanglingComponents();
//    }
//    #endif
//
//    // Clear m_IsActiveCached for all GameObjects (as done in GameObject::AwakeFromLoad)
//    // before awaking any of them. This avoids order-dependent calls to IsActive() causing
//    // children of inactive parents to be considered active, as seen in case 1051704.
//    if (UNITY_EDITOR || GameObject::ShouldClearActiveCached(awakeMode))
//    {
//        for (size_t i = 0; i < size; i++)
//        {
//            if (!GameObject::ShouldClearActiveCached(CalculateAwakeFromLoadMode(objects[i], awakeMode)))
//                continue;
//
//            Object* ptr = objects[i].objectPPtr;
//            if ((ptr == NULL) || !ptr->Is<GameObject>())
//                continue;
//
//            GameObject* go = (GameObject*)ptr;
//            go->ClearActiveCachedInternal();
//        }
//    }

    for (size_t i = 0; i < size; i++)
    {
        InstanceID instanceId = objects[i].objectPPtr.GetInstanceID();
        // PROFILER_AUTO_INSTANCE_ID(gAwakeFromLoadQueue, instanceId);

        // The AwakeFromLoadQueue should never have any duplicate elements.
        #if DEBUGMODE
        Assert(instanceId != previousInstanceID);
        previousInstanceID = instanceId;
        #endif

        Object* ptr = objects[i].objectPPtr;
        if (ptr == NULL)
            continue;

        ptr->AwakeFromLoad(CalculateAwakeFromLoadMode(objects[i], awakeMode));
        #if UNITY_EDITOR
        // Ensure object was not deleted during Awake call
        if (ptr == Object::IDToPointer(instanceId))
            ptr->ClearPersistentDirty();
        #endif
    }
}

void AwakeFromLoadQueue::PersistentManagerAwakeSingleObject(Object& o, AwakeFromLoadMode awakeMode)
{
//    PROFILER_AUTO(gAwakeFromLoadQueue, &o)
//
//    ScopedDisableSerializationSafeCheck disableSerializationSafeCheck;

#if UNITY_EDITOR
    InstanceID instanceId = o.GetInstanceID();
    {
        PROFILER_AUTO(gAwakeFromLoadQueue_CheckConsistency, &o);
        o.CheckConsistency();
        DanglingComponentManager::GetInstance().ResolveDanglingComponents();
    }
#endif

    o.AwakeFromLoad(awakeMode);

#if UNITY_EDITOR
    // Ensure object was not deleted during Awake call
    if (&o == Object::IDToPointer(instanceId))
        o.ClearPersistentDirty();
#endif
}

void AwakeFromLoadQueue::InvokeAwakeFromLoad(Item* objects, unsigned size, AwakeFromLoadMode mode)
{
    // ScopedDisableSerializationSafeCheck disableSerializationSafeCheck;

    for (size_t i = 0; i < size; i++)
    {
        Object* ptr = objects[i].objectPPtr;
        if (ptr)
        {
            ptr->AwakeFromLoad(CalculateAwakeFromLoadMode(objects[i], mode));
        }
    }
}

#if UNITY_EDITOR
void AwakeFromLoadQueue::InvokeCheckConsistency(Item* objects, unsigned size)
{
    ScopedDisableSerializationSafeCheck disableSerializationSafeCheck;

    for (size_t i = 0; i < size; i++)
    {
        Object* ptr = objects[i].objectPPtr;
        if (ptr)
        {
            PROFILER_AUTO(gAwakeFromLoadQueue_CheckConsistency, ptr);
            ptr->CheckConsistency();
        }
    }
    DanglingComponentManager::GetInstance().ResolveDanglingComponents();
}

void AwakeFromLoadQueue::InvokeEnsureUniqueTransform(Item* objects, size_t size)
{
    ScopedDisableSerializationSafeCheck disableSerializationSafeCheck;

    for (size_t i = 0; i < size; i++)
    {
        GameObject* ptr = dynamic_pptr_cast<GameObject*>(objects[i].objectPPtr);
        if (ptr != nullptr)
        {
            ptr->EnsureUniqueTransform();
        }
    }
}

void AwakeFromLoadQueue::Erase(Object& target, int queueIndex)
{
    Assert(queueIndex >= 0 && queueIndex < kMaxQueues);

    for (auto i = m_ItemArrays[queueIndex].begin(); i != m_ItemArrays[queueIndex].end(); ++i)
    {
        if ((*i).objectPPtr == PPtr<Object>(target.GetInstanceID()))
        {
            m_ItemArrays[queueIndex].erase(i);
            return;
        }
    }
}

#endif

void AwakeFromLoadQueue::ExtractAllObjects(std::vector<PPtr<Object> >& outObjects)
{
    Assert(outObjects.empty());

    int count = 0;
    for (int q = 0; q < kMaxQueues; q++)
        count += m_ItemArrays[q].size();
    outObjects.reserve(count);

    for (int q = 0; q < kMaxQueues; q++)
    {
        int size = m_ItemArrays[q].size();
        Item* objects = m_ItemArrays[q].begin().base();

        for (int i = 0; i < size; i++)
            outObjects.push_back(objects[i].objectPPtr);
    }
}
