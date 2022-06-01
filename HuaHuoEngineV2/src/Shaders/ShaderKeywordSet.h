#pragma once

#include "Utilities/BitUtility.h"
#include "Serialize/SerializeUtility.h"
#include "Containers/fixed_bitset.h"
#include "Configuration/IntegerDefinitions.h"

typedef int ShaderKeyword;
// BIND_MANAGED_TYPE_NAME(ShaderKeyword, UnityEngine_Rendering_ShaderKeyword);

// Keep in sync with ShaderKeyword.bindings.cs ShaderKeywordType
enum ShaderKeywordType
{
    kShaderKeywordNone = 0,
    kShaderKeywordAuto = (1 << 0), // Automatically detect the ShaderKeywordFlags, if the keyword already exist, keep it's flags, otherwise it's a kShaderKeywordUserDefined keyword
    kShaderKeywordBuiltinDefault = (1 << 1), // Shader keywords that are created by runtime. These are the keywords created in keywords::Initialize.
    kShaderKeywordBuiltinExtra = (1 << 2) | kShaderKeywordBuiltinDefault, // Shader keywords that are created by runtime which may not be created if the runtime feature is not used.
    kShaderKeywordBuiltinAutoStripped = (1 << 3) | kShaderKeywordBuiltinDefault, // Only built-in keyword can be automatically stripped
    kShaderKeywordUserDefined = (1 << 4), // Shader keywords created by user code.
};
// BIND_MANAGED_TYPE_NAME(ShaderKeywordType, UnityEngine_Rendering_ShaderKeywordType);

// Keep in sync with C# ShaderKeyword.k_MaxShaderKeywords
const int kMaxShaderKeywords = 448;

struct ShaderKeywordSet
{
    typedef fixed_bitset<kMaxShaderKeywords, UIntPtr> BitContainerT;
public:
    explicit ShaderKeywordSet() { Reset(); }

    void Set(ShaderKeyword key, bool value) { m_Bits.set(key, value); }
    void Enable(ShaderKeyword key) { m_Bits.set(key); }
    void Disable(ShaderKeyword key) { m_Bits.reset(key); }
    bool IsEnabled(ShaderKeyword key) const { return m_Bits.test(key); }
    bool IsEmpty() const { return m_Bits.none(); }
    void DisableBuiltinMask(UInt32 mask) { m_Bits.word(0) &= ~BitContainerT::word_type(mask); }
    void EnableAll() { m_Bits.set_all(); }
    void Reset() { m_Bits.reset_all(); }

    // Shader keywords are stored internally in the native word size (32 or 64 bits)
    const void* GetRawKeywordData() const { return m_Bits.data(); }
    // Size in bytes
    size_t GetRawKeywordDataSize() const { return m_Bits.size(); }

    bool operator==(const ShaderKeywordSet& o) const { return m_Bits == o.m_Bits; }
    bool operator!=(const ShaderKeywordSet& o) const { return m_Bits != o.m_Bits; }
    bool operator<(const ShaderKeywordSet& o) const { return m_Bits < o.m_Bits; }

    const ShaderKeywordSet& operator|=(const ShaderKeywordSet& o) { m_Bits |= o.m_Bits; return *this; }
    const ShaderKeywordSet& operator&=(const ShaderKeywordSet& o) { m_Bits &= o.m_Bits; return *this; }
    const ShaderKeywordSet& operator^=(const ShaderKeywordSet& o) { m_Bits ^= o.m_Bits; return *this; }
    ShaderKeywordSet operator|(const ShaderKeywordSet& o) const { ShaderKeywordSet c = *this; c |= o; return c; }
    ShaderKeywordSet operator&(const ShaderKeywordSet& o) const { ShaderKeywordSet c = *this; c &= o; return c; }
    ShaderKeywordSet operator^(const ShaderKeywordSet& o) const { ShaderKeywordSet c = *this; c ^= o; return c; }
    ShaderKeywordSet operator~() const { ShaderKeywordSet c = *this; c.m_Bits = ~c.m_Bits; return c; }

    inline void Mask(const ShaderKeywordSet& o) { m_Bits &= o.m_Bits; }
    inline void Add(const ShaderKeywordSet& o) { m_Bits |= o.m_Bits; }
    inline void Remove(const ShaderKeywordSet& o) { m_Bits &= ~o.m_Bits; }

    int CountEnabled() const { return BitsInArray<UIntPtr, BitContainerT::kWordCount>((const UIntPtr*)m_Bits.data()); }

private:
    BitContainerT   m_Bits;
};
// BIND_MANAGED_TYPE_NAME(ShaderKeywordSet, UnityEngine_Rendering_ShaderKeywordSet);
