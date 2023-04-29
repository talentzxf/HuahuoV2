//
// Created by VincentZhang on 2023-04-27.
//

#include "TypeTreeCache.h"
#include "TypeTree.h"
#include "TypeSystem/Object.h"
#include "Serialize/TransferFunctions/GenerateTypeTreeTransfer.h"
#include "TypeTreeQueries.h"
#include "Utilities/RegisterRuntimeInitializeAndCleanup.h"


// from TransferScriptingObjectImpl
//template<class TransferFunction>
//void TransferScriptingObject(TransferFunction& transfer, ScriptingObjectPtr instance, ScriptingClassPtr klass, const MonoScriptCache * scriptCache);

namespace TypeTreeCache
{
    struct CachedTypeTreeData
    {
        CachedTypeTreeData(int valid = 0) : invalid(!valid) {}

        bool operator!=(const CachedTypeTreeData & rhs) const
        {
            return invalid == 0;
        }

        static void Release(CachedTypeTreeData & _this)
        {
            DebugAssert(_this.data != NULL);
            _this.data->Release();
        }

        bool invalid;
        TransferInstructionFlags flags;
        TypeTreeShareableData * data;
    };

    struct HashGenerator
    {
        UInt64 operator()(const UInt64 &desc) const { return desc;  }
    };

//    typedef GfxDoubleCache<UInt64,
//    CachedTypeTreeData,
//    HashGenerator,
//    std::equal_to<UInt64>,
//    GfxDoubleCacheConcurrencyPolicy::LocklessGet,
//    GfxDoubleCacheDefaultEmptyDeletedGenerator<UInt64>,
//    kMemTypeTreeId>
//            TypeTreeCacheCollection;
//    static TypeTreeCacheCollection s_Cache(kMemTypeTree, false);

    static CachedTypeTreeData s_InvalidCacheItem(0);
    bool GetTypeTree(const Object *object, TransferInstructionFlags flags, TypeTree& outTypeTree)
    {
        if (object == NULL)
        {
            outTypeTree = TypeTree(kMemTypeTree);
            return false;
        }

        UInt64 key = TypeTreeQueries::GenerateTypeTreeSignature(flags, *object);
        const CachedTypeTreeData& cachedTT = s_Cache.Find(key, s_InvalidCacheItem);
        if (!cachedTT.invalid)
        {
            outTypeTree = TypeTree(cachedTT.data, kMemTypeTree);
            return true;
        }

        // Lets build the type tree then...
        // We unfortunately do not have const correct scripting functions, so need to cast the const away here
        bool cacheResult = true;
        outTypeTree = TypeTree(kMemTypeTree);
        Object* nonConstObj = const_cast<Object*>(object);
        GenerateTypeTreeTransfer transfer(outTypeTree, flags, nonConstObj, object->GetType()->GetSize());

//        if (IManagedObjectHost::IsObjectsTypeAHost(object))
//        {
//            const SerializableManagedRef & ref = *IManagedObjectHost::GetManagedReference(nonConstObj);
//            const ScriptingObjectPtr instance = ref.GetInstance(nonConstObj);
//            int size = instance != SCRIPTING_NULL ? ::scripting_class_instance_size(::scripting_object_get_class(instance)) : 0;
//            transfer.SetScriptingObject(instance, size);
//            cacheResult = ref.GeneratedTypeTreeIsCachable(nonConstObj);
//        }

        nonConstObj->VirtualRedirectTransfer(transfer);

        if (cacheResult)
        {
            // Store it in the cache
            CachedTypeTreeData ctt(1);
            ctt.flags = flags;
            ctt.data = outTypeTree.GetData();
            ctt.data->Retain();
            s_Cache.Set(key, ctt);
            return true;
        }
        else
            return false;
    }

//    bool GetTypeTree(const ScriptingObjectPtr managedObj, TransferInstructionFlags flags, TypeTree& outTypeTree)
//    {
//        if (managedObj == SCRIPTING_NULL)
//        {
//            outTypeTree = TypeTree(kMemTypeTree);
//            return false;
//        }
//
//        ScriptingClassPtr klass = ::scripting_object_get_class(managedObj);
//
//        TypeTree::Signature key = TypeTreeQueries::GenerateTypeTreeSignature(flags, klass);
//        const CachedTypeTreeData& cachedTT = s_Cache.Find(key, s_InvalidCacheItem);
//        if (!cachedTT.invalid)
//        {
//            outTypeTree = TypeTree(cachedTT.data, kMemTypeTree);
//            return true;
//        }
//
//        // Lets build the type tree then...
//        outTypeTree = TypeTree(kMemTypeTree);
//        GenerateTypeTreeTransfer transfer(outTypeTree, flags, NULL, 0);
//        transfer.BeginTransfer(kTransferNameIdentifierBase, ::scripting_class_get_name(klass), NULL, kNoTransferFlags);
//        ::TransferScriptingObject(transfer, managedObj, klass, NULL);
//        transfer.EndTransfer();
//
//        CachedTypeTreeData ctt(1);
//        ctt.flags = flags;
//        ctt.data = outTypeTree.GetData();
//        ctt.data->Retain();
//        s_Cache.Set(key, ctt);
//        return true;
//    }
//
//    bool GetTypeTree(const ScriptingClassPtr klass, TransferInstructionFlags flags, TypeTree& outTypeTree)
//    {
//        if (klass == SCRIPTING_NULL)
//        {
//            outTypeTree = TypeTree(kMemTypeTree);
//            return false;
//        }
//
//        TypeTree::Signature key = TypeTreeQueries::GenerateTypeTreeSignature(flags, klass);
//        const CachedTypeTreeData& cachedTT = s_Cache.Find(key, s_InvalidCacheItem);
//        if (!cachedTT.invalid)
//        {
//            outTypeTree = TypeTree(cachedTT.data, kMemTypeTree);
//            return true;
//        }
//
//        // Instanciate a default instance of the managed klass and scan that
//        // We don't want to call the constructor as that may run user code and
//        // may have all kinds of side-effects.  All we need is an instance for
//        // running the TypeTree generation.
//        ScriptingExceptionPtr exception = SCRIPTING_NULL;
//        ScriptingObjectPtr managedObj = ::scripting_object_new(klass);
//        if (managedObj == SCRIPTING_NULL)
//        {
//            core::string ns = ::scripting_class_get_namespace(klass);
//            core::string klassName = ::scripting_class_get_name(klass);
//            if (!ns.empty())
//                klassName = ns + "." + klassName;
//            WarningString(Format("The class '%s' could not be instantiated!", klassName.c_str()));
//        }
//
//        // Lets build the type tree then...
//        outTypeTree = TypeTree(kMemTypeTree);
//        GenerateTypeTreeTransfer transfer(outTypeTree, flags, NULL, 0);
//
//        int size = (klass != SCRIPTING_NULL && managedObj != SCRIPTING_NULL) ? ::scripting_class_instance_size(klass) : 0;
//        transfer.SetScriptingObject(managedObj, size);
//
//        transfer.BeginTransfer(kTransferNameIdentifierBase, scripting_class_get_name(klass), NULL, kNoTransferFlags);
//        ::TransferScriptingObject(transfer, managedObj, klass, NULL);
//        transfer.EndTransfer();
//
//        CachedTypeTreeData ctt(1);
//        ctt.flags = flags;
//        ctt.data = outTypeTree.GetData();
//        ctt.data->Retain();
//        s_Cache.Set(key, ctt);
//
//        return true;
//    }
//
//    bool GetTypeTree(const core::string & className, const core::string & ns, const core::string asmx, TransferInstructionFlags flags, TypeTree& outTypeTree)
//    {
//        ScriptingClassPtr klass = APIUpdating::Queries::ResolveTypeFromName(asmx.c_str(), ns.c_str(), className.c_str());
//
//        return GetTypeTree(klass, flags, outTypeTree);
//    }
//
//    bool RegisterTypeTree(const ScriptingClassPtr klass, TransferInstructionFlags flags, TypeTree& typeTree)
//    {
//        if (klass == SCRIPTING_NULL)
//            return false;
//
//        TypeTree::Signature key = TypeTreeQueries::GenerateTypeTreeSignature(flags, klass);
//        const CachedTypeTreeData& cachedTT = s_Cache.Find(key, s_InvalidCacheItem);
//        if (!cachedTT.invalid)
//        {
//            // We already have it
//            return false;
//        }
//
//        CachedTypeTreeData ctt(1);
//        ctt.flags = flags;
//        ctt.data = typeTree.GetData();
//        ctt.data->Retain();
//        s_Cache.Set(key, ctt);
//
//        return true;
//    }

#if ENABLE_UNIT_TESTS_WITH_FAKES
    size_t CacheSize()
    {
        return s_Cache.Size();
    }

#endif

//    /// Cleanup related stuff--------------------------------------------------------
//    void Reset()
//    {
//        s_Cache.Cleanup(CachedTypeTreeData::Release);
//    }
//
//    static void OnRuntimeInitialized(void*)
//    {
//        s_Cache.Init();
//        GlobalCallbacks::Get().domainUnloadComplete.Register(Reset);
//    }
//
//    static void OnRuntimeUninitialized(void*)
//    {
//        GlobalCallbacks::Get().domainUnloadComplete.Unregister(Reset);
//
//        s_Cache.Cleanup(CachedTypeTreeData::Release);
//        s_Cache.Dispose();
//    }
//
//    // Not using: RuntimeStatic<> as I dont want to pay for the extra ptr indirection.
//    static RegisterRuntimeInitializeAndCleanup s_TypeTreeCacheInitializer(OnRuntimeInitialized, OnRuntimeUninitialized, kStaticInitializeLogConsolePath + 1); // Just before kStaticInitializeLogConsolePath
} // end of TypeTreeCache