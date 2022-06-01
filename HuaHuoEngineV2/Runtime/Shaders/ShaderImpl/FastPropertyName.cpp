#include "FastPropertyName.h"
#include "GfxDevice/BuiltinShaderParams.h"
#include "Logging/LogAssert.h"
#include "Utilities/RegisterRuntimeInitializeAndCleanup.h"
#include "Utilities/CRC32.h"
#include "Utilities/HashStringFunctions.h"
// #include "Runtime/Threads/ReadWriteSpinLock.h"
//#include "Core/Containers/hash_map.h"
#include "Memory/MemoryMacros.h"
#include <map>
#include "Utilities/Word.h"

extern MemLabelRootId* gShaderLabContainer;

static const char*     kInvalidIndexStringName = "<noninit>";

namespace ShaderLab
{
    struct CStringCompare : public std::binary_function<char*, char*, bool>
    {
        bool operator()(const char* lhs, const char* rhs) const
        {
            return strcmp(lhs, rhs) < 0;
        }
    };
    struct ConstCharPtrHashFunctor
    {
        size_t operator()(const char* str) const
        {
            return ComputeShortStringHash32(str);
        }
    };

    struct ConstCharPtrEqualTo
    {
        bool operator()(const char* a, const char* b) const
        {
            return a == b || (a != NULL && b != NULL && strcmp(a, b) == 0);
        }
    };
    typedef std::unordered_map<const char*, int, ConstCharPtrHashFunctor, ConstCharPtrEqualTo> FastPropertyMap;
    typedef std::vector<const char*> FastPropertyIndexArray;
    typedef std::unordered_map<UInt32, int> FastPropertyHashToIndexMap;

    const int               kMaxStaticFastProperties = 500;
    static int              gStaticFastPropertyCount = 0;
    struct InitFastProperty
    {
        FastPropertyName* fp;
        const char*       name;
    };
    static InitFastProperty gStaticInitializedFastProperties[kMaxStaticFastProperties];

    // This is a ptr because of static initialization order...
    static FastPropertyMap*             gFastPropertyMap          = NULL;
    static FastPropertyIndexArray*      gFastPropertyIndexArray   = NULL;
    static FastPropertyHashToIndexMap*  gFastPropertyHashToIndexMap = NULL;
    static int                          gFastPropertyFreeIndex    = 0;

//    static ReadWriteSpinLock gFastPropertyMapLock;


    void CommonPropertyNames::StaticInitialize(void*)
    {
        Assert(gFastPropertyMap == NULL);

        InitializeBuiltinShaderParamNames(); // This must happen before creating gFastProperyMap or else it crashes on Windows.
        gFastPropertyMap = HUAHUO_NEW(FastPropertyMap, kMemShader)(); //HUAHUO_NEW(FastPropertyMap, kMemShader)(kMemShader);
        gFastPropertyIndexArray = HUAHUO_NEW(FastPropertyIndexArray, kMemShader)();
        gFastPropertyHashToIndexMap = HUAHUO_NEW(FastPropertyHashToIndexMap, kMemShader)();

        // Ensure that empty string maps to 0,
        // so that script defaults for the struct maps to empty string.
        FastPropertyName emptyProp;
        emptyProp.SetName("");
        Assert(emptyProp.index == 0);

        for (int i = 0; i < gStaticFastPropertyCount; i++)
        {
            InitFastProperty& ifp = gStaticInitializedFastProperties[i];
            ifp.fp->SetName(ifp.name);
        }
    }

    void CommonPropertyNames::StaticCleanup(void*)
    {
        for (FastPropertyMap::iterator it = gFastPropertyMap->begin(); it != gFastPropertyMap->end(); ++it)
        {
            HUAHUO_FREE(kMemShader, const_cast<char*>(it->first));
        }

        HUAHUO_DELETE(gFastPropertyMap, kMemShader);
        HUAHUO_DELETE(gFastPropertyIndexArray, kMemShader);
        HUAHUO_DELETE(gFastPropertyHashToIndexMap, kMemShader);
        gFastPropertyFreeIndex = 0;

        CleanupBuiltinShaderParamNames();
    }

    static RegisterRuntimeInitializeAndCleanup s_CommonPropertyNamesInitialize(CommonPropertyNames::StaticInitialize, CommonPropertyNames::StaticCleanup);

    const char* FastPropertyName::GetName() const
    {
        Assert(gFastPropertyMap != NULL);

        const char* propName = kInvalidIndexStringName;

        if (IsValid())
        {
            switch (index & kShaderPropBuiltinMask)
            {
                case kShaderPropBuiltinVectorMask:
                    propName = GetBuiltinVectorParamName(BuiltinShaderVectorParam(index & kShaderPropBuiltinIndexMask));
                    break;
                case kShaderPropBuiltinMatrixMask:
                    propName = GetBuiltinMatrixParamName(BuiltinShaderMatrixParam(index & kShaderPropBuiltinIndexMask));
                    break;
                case kShaderPropBuiltinTexEnvMask:
                    propName = GetBuiltinTexEnvParamName(BuiltinShaderTexEnvParam(index & kShaderPropBuiltinIndexMask));
                    break;
                default:
                {
                    // ReadWriteSpinLock::AutoReadLock lock(gFastPropertyMapLock);
                    if (index >= 0 && index < (int)gFastPropertyIndexArray->size())
                        propName = (*gFastPropertyIndexArray)[index];
                }
            }
        }

        return propName;
    }

    UInt32 GenerateFastPropertyName28BitHash(const char* name)
    {
        UInt32 hash = ComputeCRC32(name);
        hash &= 0xFFFFFFF;
        return hash;
    }

    void FastPropertyName::InitBy28BitHash(UInt32 hash)
    {
        Assert(gFastPropertyMap != NULL);
        Assert(gFastPropertyHashToIndexMap != NULL);

        // ReadWriteSpinLock::AutoReadLock lock(gFastPropertyMapLock);

        auto lookupResult = gFastPropertyHashToIndexMap->find(hash);

        index = (lookupResult == gFastPropertyHashToIndexMap->end()) ? kShaderPropInvalidIndex : lookupResult->second;

        if (index == kShaderPropInvalidIndex)
        {
            for (FastPropertyMap::const_iterator i = gFastPropertyMap->begin(); i != gFastPropertyMap->end(); i++)
            {
                if (GenerateFastPropertyName28BitHash(i->first) == hash)
                {
                    index = i->second;
                    // gFastPropertyHashToIndexMap->insert(hash, index);
                    gFastPropertyHashToIndexMap->insert(std::pair<UInt32, int>(hash, index));
                    break;
                }
            }
        }
    }

    void FastPropertyName::Init(const char* inName)
    {
    #if FAST_PROPERTY_NAME_DEBUG
        name = NULL;
    #endif

        // Before initialization - static global constructors
        if (gFastPropertyMap == NULL)
        {
            AssertMsg(gStaticFastPropertyCount < kMaxStaticFastProperties, "gStaticInitializedFastProperties ran out of space during static initialization. Increase kMaxStaticFastProperties or remove some static FastPropertyNames.");
            InitFastProperty& ifp = gStaticInitializedFastProperties[gStaticFastPropertyCount++];
            ifp.fp = this;
            ifp.name = inName;
            return;
        }

        if (!strcmp(inName, kInvalidIndexStringName))
        {
            index = kShaderPropInvalidIndex;
        #if FAST_PROPERTY_NAME_DEBUG
            name = kInvalidIndexStringName;
        #endif
            return;
        }

        // Lookup if the name already exists (most common path, lookups can be done in parallel from multiple threads)
        {
            // ReadWriteSpinLock::AutoReadLock lock(gFastPropertyMapLock);

            FastPropertyMap::const_iterator i = gFastPropertyMap->find(inName);
            if (i != gFastPropertyMap->end())
            {
                index = i->second;
            #if FAST_PROPERTY_NAME_DEBUG
                name = i->first;
            #endif
                return;
            }
        }

        int builtinIndex = -1;
        if (IsVectorBuiltinParam(inName, &builtinIndex))
            index = builtinIndex | kShaderPropBuiltinVectorMask;
        else if (IsMatrixBuiltinParam(inName, &builtinIndex))
            index = builtinIndex | kShaderPropBuiltinMatrixMask;
        else if (IsTexEnvBuiltinParam(inName, &builtinIndex))
            index = builtinIndex | kShaderPropBuiltinTexEnvMask;

        SET_ALLOC_OWNER(gShaderLabContainer->rootLabel);

        char* nameCopy = StrDup(kMemShader, inName);

        // Adding new property, must be write protected. (Only one writer & reader at one time)
        {
            // ReadWriteSpinLock::AutoWriteLock lock(gFastPropertyMapLock);
            bool builtin = IsBuiltin();

            if (!builtin)
                index = gFastPropertyFreeIndex++;

            if (gFastPropertyMap->insert(std::pair<const char*, int>(nameCopy, index)).second)
            {
                if (!builtin)
                {
                    gFastPropertyIndexArray->push_back(nameCopy);
                    AssertFormatMsg(gFastPropertyIndexArray->size() - 1 == index,
                        "FastPropertyName '%s' inconsistency, index is %i but array size-1 is %i",
                        nameCopy, index, (int)(gFastPropertyIndexArray->size() - 1));
                }
            }
            else
            {
                // It is not guaranteed that the insertion will be performed
                // just because we verified it earlier within the ReadLock.
                // Other writers could have added the keyword before this thread.
                HUAHUO_FREE(kMemShader, nameCopy);
                if (!builtin)
                    gFastPropertyFreeIndex--;
            }
        }

    #if FAST_PROPERTY_NAME_DEBUG
        name = nameCopy;
    #endif
    }

    int GetFastPropertyIndexByName(const char* name)
    {
        FastPropertyName newItem(name);
        return newItem.index;
    }
} // namespace
