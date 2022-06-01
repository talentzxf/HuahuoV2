#pragma once

// dynamic_block_array
//
// Allocates dynamic_arrays to hold the data in small blocks.
// Growing pushbacks allocates one new block at a time.
// Resize adds blocks to match size - shrinking will preserve allocated blocks
//

#include <vector>
#include <cstdlib>
#include "Internal/PlatformEnvironment.h"
#include "Internal/CoreMacros.h"
#include "Logging/LogAssert.h"
#include "Memory/MemoryMacros.h"
#include "StaticAssert.h"


template<typename T, size_t blockSize>
struct dynamic_block_array
{
private:
    typedef     std::vector<T>                    internal_container;
    typedef     std::vector<internal_container*>  container;

    template<typename TItValue, typename TItContainer>
    struct internal_iterator
    {
        typedef std::random_access_iterator_tag iterator_category;
        typedef TItValue          value_type;
        typedef size_t            size_type;
        typedef ptrdiff_t         difference_type;
        typedef value_type*       pointer;
        typedef value_type&       reference;
        typedef const value_type& const_reference;

        internal_iterator() : m_Data(NULL), m_Index(0) {}

        internal_iterator(const internal_iterator& other)
        {
            m_Data = other.m_Data;
            m_Index = other.m_Index;
        }

        reference operator*() const
        {
            return Get();
        }

        pointer operator->() const
        {
            return &Get();
        }

        internal_iterator& operator++()
        {
            DebugAssertMsg(m_Data != NULL, "Can't iterate over NULL pointer.");
            ++m_Index;
            return *this;
        }

        internal_iterator operator++(int)
        {
            internal_iterator copy(*this);
            ++(*this);
            return copy;
        }

        internal_iterator& operator--()
        {
            DebugAssertMsg(m_Data != NULL, "Can't iterate over NULL pointer.");
            --m_Index;
            return *this;
        }

        internal_iterator operator--(int)
        {
            internal_iterator copy(*this);
            --(*this);
            return copy;
        }

        internal_iterator operator+(difference_type offset)
        {
            internal_iterator copy(*this);
            copy.m_Index += offset;
            return copy;
        }

        difference_type operator-(const internal_iterator& rhs) const
        {
            return m_Index - rhs.m_Index;
        }

        internal_iterator& operator+=(difference_type n)
        {
            m_Index += n;
            return *this;
        }

        operator internal_iterator<const TItValue, const TItContainer>() const
        {
            return internal_iterator<const TItValue, const TItContainer>(m_Data, m_Index);
        }

        bool operator==(const internal_iterator& other) const { return m_Data == other.m_Data && m_Index == other.m_Index; }
        bool operator!=(const internal_iterator& other) const { return m_Data != other.m_Data || m_Index != other.m_Index; }

    private:
        internal_iterator(TItContainer * data, size_t initPos) : m_Data(data), m_Index(initPos) {}
        reference Get() const { DebugAssert(m_Index < m_Data->size() * blockSize); return (*(*m_Data)[m_Index / blockSize])[m_Index % blockSize]; }
        TItContainer * m_Data;
        size_t m_Index;
        friend struct dynamic_block_array<T, blockSize>;
    };

public:
    typedef T value_type;
    typedef internal_iterator<T, container> iterator;
    typedef internal_iterator<const T, const container> const_iterator;
    typedef ptrdiff_t difference_type;

    dynamic_block_array()
        : m_size(0)
    {
        m_label = SetCurrentMemoryOwner(kMemDynamicArray);
    }

    dynamic_block_array(MemLabelId label)
        : m_size(0)
    {
        m_label = SetCurrentMemoryOwner(label);
        m_data.set_memory_label(m_label);
    }

    dynamic_block_array(const dynamic_block_array& rhs)
        : m_size(0)
    {
        m_label = SetCurrentMemoryOwner(rhs.m_label);
        m_data.set_memory_label(m_label);
        *this = rhs;
    }

    ~dynamic_block_array()
    {
        clear_dealloc();
    }

    void clear_dealloc()
    {
        for (size_t i = 0; i < m_data.size(); i++)
            DELETE(m_data[i]);//, m_label);
        m_data.clear_dealloc();
        m_size = 0;
    }

    void resize_initialized(size_t size)
    {
        ResizerWithNoDefault resizer;
        resize_with_resizer_internal(size, resizer);
    }

    void resize_initialized(size_t size, const T& t)
    {
        ResizerWithDefault resizer(t);
        resize_with_resizer_internal(size, resizer);
    }

    void resize_uninitialized(size_t size)
    {
//#if SUPPORTS_HAS_TRIVIAL_DESTRUCTOR
//        CompileTimeAssert(HAS_TRIVIAL_DESTRUCTOR(T), "only trivial types are allowed for resize_uninitialized");
//#endif
        ResizerUninitialized resizer;
        resize_with_resizer_internal(size, resizer);
    }

    dynamic_block_array& operator=(const dynamic_block_array& other)
    {
        if (this == &other)
            return *this;

        m_size = other.m_size;
        grow(m_size);

        for (int i = 0; i < m_data.size(); i++)
            *m_data[i] = *other.m_data[i];

        return *this;
    }

    void copy_range(iterator begin, iterator end, std::vector<T>& dest)
    {
        DebugAssert(begin.m_Index < end.m_Index);
        DebugAssert(end.m_Index <= m_size);
        DebugAssertMsg(dest.empty(), "The provided array were not empty!");

        dest.reserve(end.m_Index - begin.m_Index);

        size_t endIndex = end.m_Index;

        // Clamp index to full chunks
        size_t fullChunks = endIndex - (endIndex % blockSize);
        size_t readOffset = begin.m_Index;
        while (readOffset < fullChunks)
        {
            size_t currentBlock = readOffset / blockSize;
            // Take into account that first block might have offset - will be 0 for all other blocks reads
            size_t currentBlockOffset = readOffset % blockSize;
            dest.insert(dest.end(), m_data[currentBlock]->begin() + currentBlockOffset, m_data[currentBlock]->end());
            readOffset += blockSize - currentBlockOffset;
        }

        // Write last non-full block(if any)
        if (fullChunks != endIndex)
        {
            size_t lastBlockIndex = endIndex / blockSize;
            size_t amountToRead = endIndex % blockSize;
            dest.insert(dest.end(), m_data[lastBlockIndex]->begin(), m_data[lastBlockIndex]->begin() + amountToRead);
            readOffset += amountToRead;
        }
    }

    // Copies the contents into dest
    // Label should be the same as what was used to allocate dest
    // Or an appropriate label for the area its used from if not available.
    // get size written by doing resultIterator - begin
    void copy_range(iterator begin, iterator end, T* dest)
    {
        DebugAssert(begin.m_Index < end.m_Index);
        DebugAssert(end.m_Index <= m_size);
        DebugAssert(begin.m_Index >= 0);
        // CompileTimeAssert(HAS_TRIVIAL_DESTRUCTOR(T), "only trivial types are allowed for copy_range");

        size_t endIndex = end.m_Index;

        // Clamp index to full chunks
        size_t fullChunks = endIndex - (endIndex % blockSize);
        while (begin.m_Index < fullChunks)
        {
            size_t chunkReadSize = blockSize - (begin.m_Index % blockSize);
            memcpy(dest, &(*begin), chunkReadSize * sizeof(T));
            // We use begin iterator instead of direct index here
            // as it already calculates chunk positions when dereferencing
            begin.m_Index += chunkReadSize;
            dest += chunkReadSize;
        }

        // write last non-full block(if any)
        if (fullChunks != endIndex)
        {
            memcpy(dest, &(*begin), (endIndex - begin.m_Index) * sizeof(T));
            begin.m_Index += (endIndex - begin.m_Index);
        }
    }

    void copy_to(std::vector<T>& dest) const
    {
        dest.resize_initialized(0);
        size_t copied = 0;
        int block = 0;
        dest.reserve(m_size);
        while (copied < m_size)
        {
            size_t subcount = std::min(m_size - copied, blockSize);
            dest.insert(dest.end(), m_data[block]->begin(), m_data[block]->begin() + subcount);
            copied += subcount;
            block++;
        }
    }

    T& push_back()
    {
#if SUPPORTS_HAS_TRIVIAL_DESTRUCTOR
        CompileTimeAssert(HAS_TRIVIAL_DESTRUCTOR(T), "only trivial types are allowed for push_back (void)");
#endif
        return emplace_back_uninitialized();
    }

    T& emplace_back_uninitialized()
    {
#if SUPPORTS_HAS_TRIVIAL_DESTRUCTOR
        CompileTimeAssert(HAS_TRIVIAL_DESTRUCTOR(T), "only trivial types are allowed for push_back (void) and emplace_back_uninitialized (void)");
#endif
        m_size++;
        grow(m_size);

        return get_block_by_element_position(m_size)->emplace_back_uninitialized();
    }

    T& emplace_back()
    {
        m_size++;
        grow(m_size);

        return get_block_by_element_position(m_size)->emplace_back();
    }

    template<class TArg1>
    T& emplace_back(const TArg1& arg1)
    {
        m_size++;
        grow(m_size);
        return get_block_by_element_position(m_size)->emplace_back(arg1);
    }

    template<class TArg1, class TArg2>
    T& emplace_back(const TArg1& arg1, const TArg2& arg2)
    {
        m_size++;
        grow(m_size);
        return get_block_by_element_position(m_size)->emplace_back(arg1, arg2);
    }

    T& push_back(const T& t)
    {
        return emplace_back(t);
    }

    void pop_back()
    {
        DebugAssert(!empty());
        get_block_by_element_position(m_size)->pop_back();
        m_size--;
    }

    size_t capacity() const { return m_data.size() * blockSize; }
    size_t size() const { return m_size; }
    bool empty() const { return m_size == 0; }

    void grow(size_t newSize)
    {
        while (newSize > capacity())
        {
            m_data.push_back(HUAHUO_NEW(internal_container, m_label)());
            m_data.back()->reserve(blockSize);
        }
    }

    void shrink_to_fit()
    {
        size_t containerDiff = capacity() - m_size;
        if (containerDiff > 0)
        {
            containerDiff = containerDiff / blockSize;
            while (containerDiff > 0)
            {
                DELETE(m_data.back());
                m_data.pop_back();
                containerDiff--;
            }
        }
    }

    T& back() { Assert(m_size != 0); return (*this)[m_size - 1]; }

    T const& operator[](size_t index) const { DebugAssert(index < m_size); return (*m_data[index / blockSize])[index % blockSize]; }
    T& operator[](size_t index) { DebugAssert(index < m_size); return (*m_data[index / blockSize])[index % blockSize]; }

    iterator begin()
    {
        return iterator(&m_data, 0);
    }

    const_iterator begin() const
    {
        return const_iterator(&m_data, 0);
    }

    iterator end()
    {
        return iterator(&m_data, m_size);
    }

    const_iterator end() const
    {
        return const_iterator(&m_data, m_size);
    }

    iterator erase_swap_back(iterator it)
    {
        DebugAssert(it.m_Index >= 0 && it.m_Index < m_size);
        // This function is not safe for destructors.
        // and complicated to implement when deconstructors must be called correctly.
#if SUPPORTS_HAS_TRIVIAL_DESTRUCTOR
        CompileTimeAssert(HAS_TRIVIAL_DESTRUCTOR(T), "only trivial types are allowed erase_swap_back");
#endif

        memcpy(&(*it), &back(), sizeof(T));
        pop_back();

        return it;
    }

private:

    struct ResizerWithNoDefault
    {
        static void resize(internal_container * data, size_t size)
        {
            data->resize_initialized(size);
        }
    };

    struct ResizerWithDefault
    {
        ResizerWithDefault(const T& def) : m_Default(def) {}

        void resize(internal_container * data, size_t size) const
        {
            data->resize_initialized(size, m_Default);
        }

        const T& m_Default;
    };

    struct ResizerUninitialized
    {
        static void resize(internal_container * data, size_t size)
        {
            data->resize_uninitialized(size);
        }
    };

    template<typename Resizer>
    void resize_with_resizer_internal(size_t size, const Resizer& resizer)
    {
        if (m_size < size)
        {
            grow(size);
            size_t startBlock = m_size / blockSize;
            size_t endBlock = (size - 1) / blockSize;
            for (size_t block = startBlock; block < endBlock; ++block)
            {
                resizer.resize(m_data[block], blockSize);
            }
            size_t blockFill = size - (endBlock * blockSize);
            DebugAssert(blockFill >= 1 && blockFill <= blockSize);
            resizer.resize(m_data[endBlock], blockFill);
        }
        else if (m_size > size)
        {
            size_t startBlock = size / blockSize;
            size_t endBlock = (m_size - 1) / blockSize;
            size_t blockClear = size - (startBlock * blockSize);
            DebugAssert(blockClear >= 0 && blockClear < blockSize);
            resizer.resize(m_data[startBlock], blockClear);
            for (size_t block = startBlock + 1; block <= endBlock; ++block)
                resizer.resize(m_data[block], 0);
        }
        m_size = size;
    }

    FORCE_INLINE internal_container * get_block_by_element_position(size_t index)
    {
        return m_data[(index - 1) / blockSize];
    }

    size_t      m_size;
    MemLabelId  m_label;
    container   m_data;
};
