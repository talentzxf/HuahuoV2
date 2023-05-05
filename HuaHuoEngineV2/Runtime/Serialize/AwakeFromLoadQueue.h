#pragma once

#include "TypeSystem/Object.h"
#include "BaseClasses/PPtr.h"

// kRigidbodyQueue : Rigidbody/Rigidbody2D must run before any Collider/Collider2D.  This is because all colliders need to attach to a fully initialized body.

// kAnimatorQueue : Animators must come after all other components because they bind properties (e.g. SkinnedMeshRenderer needs to be fully initialized to bind blendshapes)

// kScriptedObjectQueue : MonoBehaviours always come last since they can call anything on any component or asset.

// kColliderQueue : All Collider2D must run before any Effector2D. This is because effectors use colliders as definitions of the spatial area they operate within and check
// with the colliders not only the spatial area but other configuration of the collider.

// kCompositeColliderQueue : All CompositeCollider2D must run after any Rigidbody2D but before any Collider2D. This is because we want to ensure that the composite is fully
// initialized before any colliders using it start, at which point they check they are part of the composite before they then early-out any initialization.
// The CompositeCollider2D is also dependent upon a Rigidbody2D to which it connects to and generates all its geometry in the Rigidbody2D space.

// kParticleSystemQueue : All Particle Systems must run after Wind zones(kWindZoneQueue) and Force Fields(kParticleSystemForceField) so that they have been registered with their managers, allowing the External Forces module to use them during prewarming.

enum
{
    kManagersQueue = 0
    ,   kGameObjectQueue
    ,   kTransformQueue
    ,   kTextureQueue
    ,   kShaderQueue
    ,   kMaterialQueue
    ,   kTerrainsQueue
    ,   kAssetQueue
    ,   kCanvasQueue
    ,   kGridQueue
    ,   kTilemapQueue// After Grid but before Colliders.
    ,   kRigidbodyQueue
    ,   kCompositeColliderQueue
    ,   kColliderQueue
    ,   kParticleSystemForceFieldQueue
    ,   kParticleSystemQueue
    ,   kComponentQueue
    ,   kAnimatorQueue
    ,   kSortingGroupQueue
    ,   kPrefabQueue
    ,   kScriptedObjectQueue
    ,   kMaxQueues
};

class AwakeFromLoadQueue
{
public:

    struct Item
    {
        Object*            registerObjectPtr;

        PPtr<Object>       objectPPtr;
        const HuaHuo::Type* type;

// #if UNITY_EDITOR
        const TypeTree*    oldType;
        bool               safeBinaryLoaded;
        AwakeFromLoadMode  awakeModeOverride;
// #endif
    };

    AwakeFromLoadQueue(MemLabelId label);

    typedef std::vector<Item> ItemArray;

    bool IsInQueue(Object& target);

    void Reserve(unsigned size);
    void Add(Object& target, const TypeTree* oldType = NULL, bool safeBinaryLoaded = false, AwakeFromLoadMode awakeOverride = kDefaultAwakeFromLoadInvalid);
    void Erase(Object& target, int queueIndex);

    static void InitializeClass();

    static void PersistentManagerAwakeSingleObject(Object& o, AwakeFromLoadMode awakeMode);

    void PersistentManagerAwakeFromLoad();
    void PersistentManagerAwakeFromLoad(int queueIndex, AwakeFromLoadMode mode, bool checkConsistency);

    void EnsureTransformHierarchiesExist();

    void ClearQueue(int queueIndex);
    void Clear();

    /// Call AwakeFromLoad() on all objects in the queue.
    /// @param checkConsistencyFirst If true, CheckConsistency() will be called on all objects
    ///     in each subqueue before calling AwakeFromLoad() on the objects in the subqueue.
    ///     If false, no consistency checking is done.
    void AwakeFromLoad(AwakeFromLoadMode mode)
    {
        AwakeFromLoad(mode, false);
    }

    void RegisterObjectInstanceIDs();

    #if UNITY_EDITOR
    void CheckConsistencyAndAwakeFromLoad(AwakeFromLoadMode mode)
    {
        AwakeFromLoad(mode, true);
    }

    void InsertAwakeFromLoadQueue(AwakeFromLoadQueue& awakeFromLoadQueue, AwakeFromLoadMode awakeOverride);
    void SetPersistentForPrefabImporter();
    #endif

    void ExtractAllObjects(std::vector<PPtr<Object> >& outObjects);

    ItemArray& GetItemArray(int queueIndex) { return m_ItemArrays[queueIndex]; }
    const ItemArray& GetItemArray(int queueIndex) const { return m_ItemArrays[queueIndex]; }

    template<typename T>
    T* GetManagerFromQueue() const;

private:

    int DetermineQueueIndex(const HuaHuo::Type* type, Object& obj);

    static void InvokeAwakeFromLoad(Item* objects, unsigned size, AwakeFromLoadMode mode);
    #if UNITY_EDITOR
    static void InvokeCheckConsistency(Item* objects, unsigned size);
    static void InvokeEnsureUniqueTransform(Item* objects, size_t size);
    #endif
    static void InvokePersistentManagerAwake(Item* objects, unsigned size, AwakeFromLoadMode awakeMode, bool checkConsistency);

    static void RegisterObjectInstanceIDsInternal(Item* objects, unsigned size);

    void AwakeFromLoad(AwakeFromLoadMode mode, bool checkConsistencyFirst);
    void InsertAwakeFromLoadQueue(std::vector<Item>& src, std::vector<Item>& dst, AwakeFromLoadMode awakeOverride);

    #if UNITY_EDITOR
    void AddMissingComponents();
    #endif

    ItemArray m_ItemArrays[kMaxQueues];
};

template<typename T>
T* AwakeFromLoadQueue::GetManagerFromQueue() const
{
    const AwakeFromLoadQueue::ItemArray& managerItems = GetItemArray(kManagersQueue);
    for (size_t i = 0; i < managerItems.size(); i++)
    {
        T* loadedManager = dynamic_instanceID_cast<T*>(managerItems[i].objectPPtr.GetInstanceID());
        if (loadedManager)
            return loadedManager;
    }
    return NULL;
}
