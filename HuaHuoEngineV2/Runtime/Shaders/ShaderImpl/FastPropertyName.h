#pragma once

#include "FastPropertyNameTypes.h"
#include "Logging/LogAssert.h"

#define FAST_PROPERTY_NAME_DEBUG 0

// Used for fast string comparisons in shaderlab.
// A string is converted to a index which is unique per name.

namespace ShaderLab
{
    struct FastPropertyName
    {
        int   index;

        const char* GetName() const;
        void        SetName(const char *name) { Init(name); }

        void InitBy28BitHash(UInt32 hash);


        friend bool operator==(const FastPropertyName& lhs, const FastPropertyName& rhs)
        {
            return lhs.index == rhs.index;
        }

        friend bool operator!=(const FastPropertyName& lhs, const FastPropertyName& rhs)
        {
            return lhs.index != rhs.index;
        }

        friend bool operator<(const FastPropertyName& lhs, const FastPropertyName& rhs)
        {
            return lhs.index < rhs.index;
        }

        FastPropertyName() : index(kShaderPropInvalidIndex)
        {
        #if FAST_PROPERTY_NAME_DEBUG
            name = NULL;
        #endif
        }

        FastPropertyName(const char* fastPropertyName) : index(kShaderPropInvalidIndex)
        {
            Init(fastPropertyName);
        }

        explicit FastPropertyName(const  std::string& fastPropertyName) : index(kShaderPropInvalidIndex)
        {
            Init(fastPropertyName.c_str());
        }

        explicit FastPropertyName(int id) : index(id)
        {
        #if FAST_PROPERTY_NAME_DEBUG
            name = NULL;
        #endif
        }

        void    Clear()         { index = kShaderPropInvalidIndex; }
        bool    IsValid() const { return index != kShaderPropInvalidIndex; }


        bool    IsBuiltin() const       { return (IsValid() && (index & kShaderPropBuiltinMask) != 0); }
        bool    IsBuiltinVector() const { return (IsValid() && (index & kShaderPropBuiltinMask) == kShaderPropBuiltinVectorMask); }
        bool    IsBuiltinMatrix() const { return (IsValid() && (index & kShaderPropBuiltinMask) == kShaderPropBuiltinMatrixMask); }
        bool    IsBuiltinTexEnv() const { return (IsValid() && (index & kShaderPropBuiltinMask) == kShaderPropBuiltinTexEnvMask); }

        int     BuiltinIndex() const    { return (index & kShaderPropBuiltinIndexMask); }


    private:
    #if FAST_PROPERTY_NAME_DEBUG
        // Having names in Debug makes it easier to figure out what goes wrong with properties
        const char *name;
    #endif
        EXPORT_COREMODULE void Init(const char* name);
    };

    typedef FastPropertyName Property;

    struct FastPropertyNameHashFunctor
    {
        size_t operator()(FastPropertyName name) const
        {
            return name.index;
        }
    };

    inline FastPropertyName FastPropertyNameFromBuiltinTexEnv(UInt32 prop)
    {
        FastPropertyName fp;
        fp.index = prop | kShaderPropBuiltinTexEnvMask;
        DebugAssert(fp.IsBuiltin() && fp.IsBuiltinTexEnv());
        return fp;
    }

    inline FastPropertyName FastPropertyNameFromBuiltinVector(UInt32 prop)
    {
        FastPropertyName fp;
        fp.index = prop | kShaderPropBuiltinVectorMask;
        DebugAssert(fp.IsBuiltin() && fp.IsBuiltinVector());
        return fp;
    }

    inline FastPropertyName FastPropertyNameFromBuiltinMatrix(UInt32 prop)
    {
        FastPropertyName fp;
        fp.index = prop | kShaderPropBuiltinMatrixMask;
        DebugAssert(fp.IsBuiltin() && fp.IsBuiltinMatrix());
        return fp;
    }

#define SHADERPROP(a) \
    ShaderLab::FastPropertyName kSLProp ## a("_" #a)
#define SHADERPROPRAW(a) \
    ShaderLab::FastPropertyName kSLProp ## a(#a)

    struct CommonPropertyNames
    {
        static void StaticInitialize(void*);
        static void StaticCleanup(void*);
    };

    int GetFastPropertyIndexByName(const char* name);
    UInt32 GenerateFastPropertyName28BitHash(const char* name);
} // namespace
