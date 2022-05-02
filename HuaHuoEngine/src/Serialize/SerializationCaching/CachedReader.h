#pragma once

#include "Memory/MemoryMacros.h"
#include "baselib/include/IntegerDefinitions.h"
#include "Serialize/SerializationMetaFlags.h"
#include <string>
#include <memory.h>
class CacheReaderBase;

struct ResourceImage
{
    UInt8*      m_Data;
    size_t      m_Size;

public:

    ResourceImage(const std::string& path);
    ~ResourceImage();

    UInt8* Fetch(size_t offset, size_t size)
    {
        Assert(m_Data != NULL);
        Assert(size + offset <= m_Size);
        return m_Data + offset;
    }
};

struct ResourceImageGroup
{
    ResourceImage* resourceImages[kNbResourceImages];

    ResourceImageGroup() { memset(this, 0, sizeof(ResourceImageGroup)); }

    void AddResourceImage(int index, const std::string& path)
    {
        resourceImages[index] = NEW(ResourceImage)(path);
    }

    void Cleanup()
    {
        for (int i = 0; i < kNbResourceImages; i++)
            DELETE(resourceImages[i]);
    }
};

class CachedReader
{
private:

    UInt8*             m_CachePosition;
    UInt8*             m_CacheStart;
    UInt8*             m_CacheEnd;
    CacheReaderBase*   m_Cacher;
    SInt32             m_Block;
    size_t             m_CacheSize;
    size_t             m_MinimumPosition;
    size_t             m_MaximumPosition;
    bool               m_OutOfBoundsRead;

    ResourceImageGroup m_ResourceImageGroup;

    void UpdateReadCache(void* data, size_t size);

    CachedReader(const  CachedReader& c); // undefined
    CachedReader& operator=(const  CachedReader& c);  // undefined

    void OutOfBoundsError(size_t position, size_t size);
    void LockCacheBlockBounded();

public:

    CachedReader();
    ~CachedReader();

    void InitRead(CacheReaderBase& cacher, size_t position, size_t size);
    void InitResourceImages(ResourceImageGroup& resourceImage);

    size_t GetEndPosition() { return m_MaximumPosition; }

    size_t End();

    template<class T>
    void Skip()
    {
        m_CachePosition += sizeof(T);
    }

    void Skip(size_t size);

    UInt8* FetchResourceImageData(ActiveResourceImage index, size_t offset, size_t size);

#if UNITY_NO_UNALIGNED_MEMORY_ACCESS
    // WebGL needs 8 byte alignment for doubles. use memcpy instead.
    void Read(double& data, size_t position)
    {
        m_CachePosition = m_CacheStart + position - m_Block * m_CacheSize;
        if (m_CachePosition >= m_CacheStart && m_CachePosition + sizeof(data) <= m_CacheEnd)
        {
            memcpy(&data, m_CachePosition, sizeof(double));
            m_CachePosition += sizeof(data);
        }
        else
            UpdateReadCache(&data, sizeof(data));
    }

    void Read(double& data)
    {
        if (m_CachePosition + sizeof(double) <= m_CacheEnd)
        {
            memcpy(&data, m_CachePosition, sizeof(double));
            m_CachePosition += sizeof(double);
        }
        else
            UpdateReadCache(&data, sizeof(data));
    }

#endif

    template<class T>
    void Read(T& data, size_t position)
    {
        m_CachePosition = m_CacheStart + position - m_Block * m_CacheSize;
        if (m_CachePosition >= m_CacheStart && m_CachePosition + sizeof(data) <= m_CacheEnd)
        {
            data = *reinterpret_cast<T*>(m_CachePosition);
            m_CachePosition += sizeof(T);
        }
        else
            UpdateReadCache(&data, sizeof(data));
    }

    template<class T>
    void Read(T& data)
    {
        if (m_CachePosition + sizeof(T) <= m_CacheEnd)
        {
            data = *reinterpret_cast<T*>(m_CachePosition);
            m_CachePosition += sizeof(T);
        }
        else
            UpdateReadCache(&data, sizeof(data));
    }

    void Align4Read();

    size_t GetPosition()   const                           { return m_CachePosition - m_CacheStart + m_Block * m_CacheSize; }
    void SetPosition(size_t position);
    void SetAbsoluteMemoryPosition(UInt8* position) { m_CachePosition = position; }
    UInt8* GetAbsoluteMemoryPosition() { return m_CachePosition; }

    void Read(void* data, size_t size);

    CacheReaderBase*    GetCacher() const { return m_Cacher; }
};
