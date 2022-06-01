//
// Created by VincentZhang on 4/25/2022.
//

#include "BatchAllocator.h"
#include "Logging/LogAssert.h"
#include <cstring>

#ifndef PLATFORM_CACHE_LINE_SIZE
#define PLATFORM_CACHE_LINE_SIZE 64
#endif

BatchAllocator::BatchAllocator()
{
    m_BufferSize = 0;
    m_AllocationCount = 0;
    m_MaxAlignment = 4;
}

void BatchAllocator::AllocateInternal(void** dstPtr, SInt32 rootIndex, size_t elementSize, size_t count, size_t alignment)
{
    Assert(elementSize >= alignment || elementSize == 1);
    Assert(elementSize % alignment == 0 || elementSize == 1);

    AssertMsg(m_AllocationCount < kMaxAllocationCount, "kMaxAllocationCount exceeded. This will crash. Please increase kMaxAllocationCount.");

    size_t size = elementSize * count;

    if (rootIndex != -1)
    {
#if ENABLE_ASSERTIONS
        Assert(reinterpret_cast<size_t>(dstPtr) < m_Allocations[rootIndex].offset + m_Allocations[rootIndex].size);
#endif
        Assert(reinterpret_cast<size_t>(dstPtr) >= m_Allocations[rootIndex].offset);
    }
    else
    {
        Assert(reinterpret_cast<size_t>(dstPtr) >= m_BufferSize);
    }

    m_Allocations[m_AllocationCount].dstPtr = dstPtr;
    m_Allocations[m_AllocationCount].rootIndex = rootIndex;
    m_Allocations[m_AllocationCount].srcSize = 0;
#if ENABLE_ASSERTIONS
    m_Allocations[m_AllocationCount].size = size;
#endif

    m_BufferSize = AlignSize(m_BufferSize, alignment);
    m_Allocations[m_AllocationCount].offset = m_BufferSize;
    m_BufferSize += size;

    m_MaxAlignment = MaxAlignment(m_MaxAlignment, alignment);

    m_AllocationCount++;
}

void BatchAllocator::PadToCacheLine()
{
    m_BufferSize = AlignSize(m_BufferSize, PLATFORM_CACHE_LINE_SIZE);
    m_MaxAlignment = MaxAlignment(m_MaxAlignment, PLATFORM_CACHE_LINE_SIZE);
}

void BatchAllocator::ReallocateInternal(void** dstPtr, size_t elementSize, size_t newCount, size_t oldCount, size_t alignment)
{
    AllocateInternal(dstPtr, -1, elementSize, newCount, alignment);
    m_Allocations[m_AllocationCount - 1].srcSize = std::min(oldCount, newCount) * elementSize;
}

void BatchAllocator::Commit(MemLabelRef label, BatchAllocator::CommitClearMode clearMode)
{
    UInt8* buffer = reinterpret_cast<UInt8*>(HUAHUO_MALLOC_ALIGNED(label, m_BufferSize, m_MaxAlignment));

    if (kClearMemory == clearMode)
    {
        memset(buffer, 0, m_BufferSize);
    }

    for (size_t i = 0; i != m_AllocationCount; i++)
    {
        void* ptr = buffer + m_Allocations[i].offset;
        size_t dstOffset = 0;
        SInt32 rootIndex = m_Allocations[i].rootIndex;
        if (rootIndex != -1)
            dstOffset = (size_t)buffer + m_Allocations[rootIndex].offset;

        UInt8* dstPtr = reinterpret_cast<UInt8*>(m_Allocations[i].dstPtr) + dstOffset;
        if (m_Allocations[i].srcSize != 0)
            memcpy(ptr, *(reinterpret_cast<void**>(dstPtr)), m_Allocations[i].srcSize);

        *(reinterpret_cast<void**>(dstPtr)) = ptr;
    }
}
