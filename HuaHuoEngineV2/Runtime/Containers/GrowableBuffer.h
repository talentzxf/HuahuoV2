#pragma once

// Buffer of arbitrary data that you can write into, and read from.
// Similar to vector/dynamic_array, but for arbitrary variable-sized data.
// Major feature: takes care of proper alignment when writing/reading.

#include <new> // for placement new
#include <cstdlib>
#include "Baselib.h"
#include "Logging/LogAssert.h"

class GrowableBuffer
{
public:
    enum class GrowMode
    {
        // Grow by fixed size (given in constructor)
        Fixed = 0,
        // Double the size when reallocating
        DoubleOnGrow = 1
    };

    GrowableBuffer(MemLabelRef label = kMemUtility, size_t initialSize = 256, size_t growIncrement = 8*1024, GrowMode growMode = GrowMode::Fixed);
    GrowableBuffer(const GrowableBuffer& other);
    ~GrowableBuffer();

    // Adds to the buffer (resizes if necessary).
    template<class T> void     WriteValueType(const T& val, size_t alignment = alignof(T))
    {
        void* pdata = GetWriteDataPointer(sizeof(T), alignment);
        new(pdata) T(val);
    }

    template<class T> void     WriteArrayType(const T* vals, int count, size_t alignment = alignof(T))
    {
        T* pdata = (T*)GetWriteDataPointer(count * sizeof(T), alignment);
        for (int i = 0; i < count; i++)
            new(&pdata[i]) T(vals[i]);
    }

    // Reads from the buffer at given position, and changes position to point
    // after what is read. Caller should stop reading after
    // reaching size().
    template<class T> T&       ReadValueType(size_t& inOutPosition, size_t alignment = alignof(T)) const
    {
        void* pdata = GetReadDataPointer(sizeof(T), alignment, inOutPosition);
        T& src = *reinterpret_cast<T*>(pdata);
        return src;
    }

    template<class T> T*       ReadArrayType(size_t& inOutPosition, int count, size_t alignment = alignof(T)) const
    {
        void* pdata = GetReadDataPointer(count * sizeof(T), alignment, inOutPosition);
        T* src = reinterpret_cast<T*>(pdata);
        return src;
    }

    size_t size() const { return m_Size; }
    size_t capacity() const { return m_Capacity; }
//    MemLabelRef GetMemoryLabel() const { return m_Label; }

    void clear() { m_Size = 0; }

private:
    FORCE_INLINE static size_t Align(size_t val, size_t alignment) { return (val + alignment - 1) & ~(alignment - 1); }

    void*   GetWriteDataPointer(size_t size, size_t alignment);
    void*   GetReadDataPointer(size_t size, size_t alignment, size_t& inOutPosition) const;
    void    EnlargeBuffer(size_t dataPos, size_t dataEnd);

    GrowableBuffer& operator=(const GrowableBuffer&); // No assignment

private:
    MemLabelId m_Label;
    char* m_Buffer;
    size_t m_Capacity;
    size_t m_Size;
    size_t m_GrowStepSize;
    GrowMode m_GrowMode;
};


inline void* GrowableBuffer::GetWriteDataPointer(size_t size, size_t alignment)
{
    size = Align(size, alignment);
    size_t dataPos = Align(m_Size, alignment);
    size_t dataEnd = dataPos + size;
    if (dataEnd > m_Capacity)
    {
        EnlargeBuffer(dataPos, dataEnd);
    }
    m_Size = dataEnd;
    DebugAssert(m_Size <= m_Capacity);
    return &m_Buffer[dataPos];
}

inline void* GrowableBuffer::GetReadDataPointer(size_t size, size_t alignment, size_t& inOutPosition) const
{
    size = Align(size, alignment);
    size_t dataPos = Align(inOutPosition, alignment);
    size_t dataEnd = dataPos + size;
    DebugAssert(dataEnd <= m_Size);
    inOutPosition = dataEnd;
    return &m_Buffer[dataPos];
}
