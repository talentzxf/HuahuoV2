#pragma once
#include "Logging/LogAssert.h"
#include <cstring>

// Fixed-size bitset. Similar to dynamic_bitset, but does not do dynamic allocations and stuff.
template<int N, typename WordType>
class fixed_bitset
{
public:
    enum
    {
        kSize = N,
        kBitsPerWord = sizeof(WordType) * 8,
        kWordCount = (N + kBitsPerWord - 1) / kBitsPerWord
    };

    typedef size_t  size_type;
    typedef WordType word_type;

    fixed_bitset()
    {
        for (int i = 0; i < kWordCount; ++i)
            m_Words[i] = 0;
    }

    // note: default copy constructor and assignment operator are ok

    void set(size_type index, bool value)
    {
        DebugAssert(index < N);
        m_Words[word_index(index)] &= ~bit_value(index);
        m_Words[word_index(index)] |= (value ? bit_value(index) : 0);
    }

    void set(size_type index)
    {
        DebugAssert(index < N);
        m_Words[word_index(index)] |= bit_value(index);
    }

    void reset(size_type index)
    {
        DebugAssert(index < N);
        m_Words[word_index(index)] &= ~bit_value(index);
    }

    bool test(size_type index) const
    {
        DebugAssert(index < N);
        return (m_Words[word_index(index)] & bit_value(index)) != 0;
    }

    bool any() const
    {
        for (int i = 0; i < kWordCount; ++i)
        {
            if (m_Words[i] != 0)
                return true;
        }
        return false;
    }

    bool none() const
    {
        return !any();
    }

    void set_all()
    {
        memset(m_Words, 0xff, sizeof(m_Words));
    }

    void reset_all()
    {
        memset(m_Words, 0, sizeof(m_Words));
    }

    bool operator==(const fixed_bitset& o) const
    {
        for (int i = 0; i < kWordCount; ++i)
        {
            if (m_Words[i] != o.m_Words[i])
                return false;
        }
        return true;
    }

    bool operator!=(const fixed_bitset& o) const
    {
        return !(*this == o);
    }

    // Sortable with highest bit indices as the most significant
    bool operator<(const fixed_bitset& o) const
    {
        for (int i = kWordCount - 1; i >= 0; --i)
        {
            if (m_Words[i] != o.m_Words[i])
                return m_Words[i] < o.m_Words[i];
        }
        return false;
    }

    fixed_bitset operator|(const fixed_bitset& o) const
    {
        fixed_bitset c = *this;
        c |= o;
        return c;
    }

    fixed_bitset operator&(const fixed_bitset& o) const
    {
        fixed_bitset c = *this;
        c &= o;
        return c;
    }

    fixed_bitset operator^(const fixed_bitset& o) const
    {
        fixed_bitset c = *this;
        c ^= o;
        return c;
    }

    fixed_bitset operator~() const
    {
        fixed_bitset c;
        for (int i = 0; i < kWordCount; ++i)
            c.m_Words[i] = ~m_Words[i];
        return c;
    }

    fixed_bitset& operator|=(const fixed_bitset& o)
    {
        for (int i = 0; i < kWordCount; ++i)
            m_Words[i] |= o.m_Words[i];
        return *this;
    }

    fixed_bitset& operator&=(const fixed_bitset& o)
    {
        for (int i = 0; i < kWordCount; ++i)
            m_Words[i] &= o.m_Words[i];
        return *this;
    }

    fixed_bitset& operator^=(const fixed_bitset& o)
    {
        for (int i = 0; i < kWordCount; ++i)
            m_Words[i] ^= o.m_Words[i];
        return *this;
    }

    // Accessors for underlying word-sized data
    const WordType& word(size_type index) const
    {
        DebugAssert(index < kWordCount);
        return m_Words[index];
    }

    WordType& word(size_type index)
    {
        DebugAssert(index < kWordCount);
        return m_Words[index];
    }

    // Accessors for typeless data
    const void* data() const
    {
        return m_Words;
    }

    void* data()
    {
        return m_Words;
    }

    size_t size() const
    {
        return sizeof(m_Words);
    }

private:
    // Use simplified math when the fixed_bitset only has one word
    static size_t word_index(size_t index) { return (kWordCount > 1) ? (index / kBitsPerWord) : 0; }
    static WordType bit_value(size_t index) { return WordType(1) << ((kWordCount > 1) ? (index & (kBitsPerWord - 1)) : index); }

    WordType m_Words[kWordCount];
};
