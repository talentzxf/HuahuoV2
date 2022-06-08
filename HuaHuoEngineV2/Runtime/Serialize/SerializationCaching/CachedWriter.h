//
// Created by VincentZhang on 4/26/2022.
//

#ifndef HUAHUOENGINE_CACHEDWRITER_H
#define HUAHUOENGINE_CACHEDWRITER_H

#include <cstdlib>
#include "BaseClasses/BaseTypes.h"
#include <limits>
#include <cstdio>

class CacheWriterBase;

class CachedWriter {
public:
    CachedWriter() {
    }

    void InitWrite(CacheWriterBase &cacher);

    template<class T>
    void Write(const T &data);

    void Write(const void *data, size_t size);

    size_t GetPosition() const {
        return m_ActiveWriter.GetPosition();
    }

    bool CompleteWriting();

    void Align4Write();

    CacheWriterBase& GetCacheBase() { return *m_ActiveWriter.cacheBase; }

private:
    struct ActiveWriter {
        UInt8 *cachePosition;
        UInt8 *cacheStart;
        UInt8 *cacheEnd;
        size_t block;
        CacheWriterBase *cacheBase;

        ActiveWriter() {
            cachePosition = NULL;
            cacheStart = NULL;
            cacheEnd = NULL;
            block = std::numeric_limits<size_t>::max();
            cacheBase = NULL;
        }

        size_t GetPosition() const;
    };

    ActiveWriter m_ActiveWriter;

    void UpdateWriteCache(const void *data, size_t size);

    void SetPosition(size_t position);

    void PreallocateForWrite(size_t sizeOfWrite);

    static void InitActiveWriter(CachedWriter::ActiveWriter &activeWriter, CacheWriterBase &cacher);
};

template<class T>
inline void CachedWriter::Write(const T &data) {
#if CHECK_SERIALIZE_ALIGNMENT
    if (m_CheckSerializeAlignment)
    {
        SInt32 position = reinterpret_cast<SInt32>(m_ActiveWriter.cachePosition);
        SInt32 size = sizeof(T);
        SInt32 align = position % size;
        if (align != 0)
        {
            ErrorString("Alignment error ");
        }
    }
#endif

    if (m_ActiveWriter.cachePosition + sizeof(T) < m_ActiveWriter.cacheEnd) {
        *reinterpret_cast<T *>(m_ActiveWriter.cachePosition) = data;
        m_ActiveWriter.cachePosition += sizeof(T);
    } else
        UpdateWriteCache(&data, sizeof(data));
}

#endif //HUAHUOENGINE_CACHEDWRITER_H
