//
// Created by VincentZhang on 7/17/2022.
//

#include "CloneObject.h"
#include "Components/Transform/Transform.h"
#include "Serialize/SerializationCaching/BlockMemoryCacheWriter.h"
#include "Serialize/SerializationCaching/MemoryCacherReadBlocks.h"
#include "Components/BaseComponent.h"
#include "Serialize/TransferFunctions/RemapPPtrTransfer.h"
#include "Serialize/AwakeFromLoadQueue.h"
#include <cstring>

inline GameObject* GetGameObjectPtr(Object& o)
{
    GameObject* go = dynamic_pptr_cast<GameObject*>(&o);
    if (go != NULL)
        return go;

    BaseComponent* component = dynamic_pptr_cast<BaseComponent*>(&o);
    if (component != NULL)
        return component->GetGameObjectPtr();

    return NULL;
}


class RemapFunctorTempRemapTable : public GenerateIDFunctor
{
public:
    const TempRemapTable& remap;

    RemapFunctorTempRemapTable(const TempRemapTable& inRemap) : remap(inRemap) {}

    virtual InstanceID GenerateInstanceID(InstanceID oldInstanceID, TransferMetaFlags metaFlags = kNoTransferFlags)
    {
        Assert((metaFlags & kStrongPPtrMask) == 0);

        TempRemapTable::const_iterator found = remap.find(oldInstanceID);
        // No Remap found -> set zero or dont touch instanceID
        if (found == remap.end())
            return oldInstanceID;
            // Remap
        else
            return found->second;
    }
};

static Object& ProduceClone(Object& object)
{
    Object* clone = Object::Produce(object.GetType(), InstanceID_None, kMemBaseObject, kCreateObjectDefaultNoLock);

//    const ManagedObjectHostAttribute* soAttribute = clone->GetType()->FindAttribute<ManagedObjectHostAttribute>();
//    if (soAttribute != NULL)
//    {
//        SerializableManagedRef* dst = IManagedObjectHost::GetManagedReference(soAttribute, clone);
//        SerializableManagedRef* src = IManagedObjectHost::GetManagedReference(soAttribute, &object);
//        dst->SetScriptingDataFrom(clone, *src);
//    }

    return *clone;
}

static void CollectAndProduceSingleObject(Object& singleObject, TempRemapTable& remappedPtrs)
{
    Object& clone = ProduceClone(singleObject);

    remappedPtrs.get_vector().push_back(std::make_pair(singleObject.GetInstanceID(), clone.GetInstanceID()));
}


void CollectAndProduceClonedIsland(Object& o, Transform* newFather, TempRemapTable& remappedPtrs)
{
    // PROFILER_AUTO(gInstantiateProfileProduce, &o)

    remappedPtrs.reserve(64);

//    GameObject* go = GetGameObjectPtr(o);

    // SetObjectLockForWrite();

//    if (go)
//        CollectAndProduceGameObjectHierarchy(go->GetComponent<Transform>(), newFather, remappedPtrs);
//    else
        CollectAndProduceSingleObject(o, remappedPtrs);

    // ReleaseObjectLock();

    remappedPtrs.sort();
}


static Object* CloneObjectImpl(Object* object, Transform* newFather, TempRemapTable& ptrs)
{
    CollectAndProduceClonedIsland(*object, newFather, ptrs);

//    PROFILER_AUTO(gInstantiateProfileCopy, object);

    TempRemapTable::iterator it;

    BlockMemoryCacheWriter cacheWriter(kMemTempAlloc);

    RemapFunctorTempRemapTable functor(ptrs);
    RemapPPtrTransfer remapTransfer(kSerializeForPrefabSystem | kIsCloningObject, true);
    remapTransfer.SetGenerateIDFunctor(&functor);

    for (it = ptrs.begin(); it != ptrs.end(); it++)
    {
        Object& original = *PPtr<Object>(it->first);
        if (original.GetType() == TypeOf<Transform>() || original.GetType() == TypeOf<GameObject>())
            continue;

#if UNITY_EDITOR
        original.WarnInstantiateDisallowed();
#endif

        // Copy Data
        Object& clone = *PPtr<Object>(it->second);

        StreamedBinaryWrite writeStream;
        CachedWriter& writeCache = writeStream.Init(kSerializeForPrefabSystem/*, BuildTargetSelection::NoTarget()*/);
        writeCache.InitWrite(cacheWriter);
        original.VirtualRedirectTransfer(writeStream);
        writeCache.CompleteWriting();

        MemoryCacherReadBlocks cacheReader(cacheWriter.GetCacheBlocks(), cacheWriter.GetFileLength(), cacheWriter.GetCacheSize());

        StreamedBinaryRead readStream;
        CachedReader& readCache = readStream.Init(kSerializeForPrefabSystem | kDontCreateMonoBehaviourScriptWrapper | kIsCloningObject);

        readCache.InitRead(cacheReader, 0, writeCache.GetPosition());
        clone.VirtualRedirectTransfer(readStream);
        readCache.End();

#if UNITY_EDITOR
        clone.CloneAdditionalEditorProperties(original);
#endif

        // Remap references
        clone.VirtualRedirectTransfer(remapTransfer);
    }


    TempRemapTable::iterator found = ptrs.find(object->GetInstanceID());
    Assert(found != ptrs.end());
    object = PPtr<Object>(found->second);

    return object;
}

void AwakeAndActivateClonedObjects(Object** inOutInstantiatedObject, const TempRemapTable& ptrs)
{
    // PROFILER_AUTO(gInstantiateProfileAwake);
    AwakeFromLoadQueue queue(kMemTempAlloc);
    queue.Reserve(ptrs.size());

    // Since Transforms might be replaced with RectTransforms during Awake(),
    // save the GameObject if applicable.
    GameObject* transformGameObject = NULL;
    if ((*inOutInstantiatedObject)->Is<Transform>())
        transformGameObject = &static_cast<Transform*>(*inOutInstantiatedObject)->GetGameObject();

    for (TempRemapTable::const_iterator i = ptrs.begin(); i != ptrs.end(); ++i)
    {
        Object& clone = *PPtr<Object>(i->second);
        clone.SetHideFlags(Object::kHideFlagsNone);
        clone.SetDirty();

#if !UNITY_RELEASE
        // we will clone that object - no need to call Reset as we will construct it fully
        clone.SetResetCalledInternal();
#endif

        queue.Add(*PPtr<Object>(i->second));
    }

    InstanceID instanceID = (*inOutInstantiatedObject)->GetInstanceID();

    queue.AwakeFromLoad((AwakeFromLoadMode)(kDefaultAwakeFromLoad | kInstantiateOrCreateFromCodeAwakeFromLoad));

    if (!PPtr<Object>(instanceID).IsValid())
    {
        *inOutInstantiatedObject = NULL;
        return;
    }

    // Handle potential Transform replacement during Awake().
    if (transformGameObject != NULL)
        *inOutInstantiatedObject = &transformGameObject->GetComponent<Transform>();
}

Object* CloneObject(Object& inObject)
{
    TempRemapTable ptrs;
    Object* object = CloneObjectImpl(&inObject, NULL, ptrs);

    if (object){
        const char* name = object->GetName();
        char clonedName[255];
        strcpy(clonedName, name);
        strcat(clonedName, "(Clone)");
        object->SetName(clonedName);
    }

    AwakeAndActivateClonedObjects(&object, ptrs);
    return object;
}