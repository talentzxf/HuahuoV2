//
// Created by VincentZhang on 4/25/2022.
//

#ifndef HUAHUOENGINE_BATCHALLOCATOR_H
#define HUAHUOENGINE_BATCHALLOCATOR_H

#include <cstdlib>
#include "baselib/include/IntegerDefinitions.h"
#include "MemoryMacros.h"

class BatchAllocator {
public:
    enum CommitClearMode
    {
        kClearMemory,
        kLeaveUninitialized,
    };
public:
    BatchAllocator();

    void Commit(CommitClearMode clearMode);

    void Commit()
    {
        Commit(kLeaveUninitialized);
    }

    // Allocates the root object
    template<class T>
    void AllocateRoot(T*& dstPtr, size_t count = 1, size_t alignment = 0);

    // Allocates a field (the ptr passed into this function must be relative to the root)
    template<class T>
    void AllocateField(T*& dstPtr, size_t count, size_t alignment = 0);
    void AllocateField(void*& dstPtr, size_t count, size_t alignment, size_t sizeOf);

    // Allocates an o
    template<class T>
    void Allocate(T*& dstPtr, size_t count, size_t alignment = 0);

    // Allocates a new array and memcppies min(srcCount, newCount) elements,
    // then assigns the new ptr location to passed adress.
    template<class T>
    void Reallocate(T*& dstPtr, size_t newCount, size_t srcCount, size_t alignment = 0);

    // Aligns the next allocation to the platform's cache line size.
    // Use to avoid false sharing between jobs.
    void PadToCacheLine();

    static void DeallocateRoot(void* dstPtr) { FREE(dstPtr); }

private:

    void AllocateInternal(void** dstPtr, SInt32 rootIndex, size_t elementSize, size_t count, size_t alignment);
    void ReallocateInternal(void** dstPtr, size_t elementSize, size_t newCount, size_t oldCount, size_t alignment);

    struct Allocation
    {
        void**  dstPtr;

        SInt32  rootIndex;
        size_t  offset;
        size_t  srcSize;
#if ENABLE_ASSERTIONS
        size_t  size;
#endif
    };

    enum { kMaxAllocationCount = 64 };

    size_t          m_BufferSize;
    size_t          m_AllocationCount;
    size_t          m_MaxAlignment;
    Allocation      m_Allocations[kMaxAllocationCount];
};


#endif //HUAHUOENGINE_BATCHALLOCATOR_H
