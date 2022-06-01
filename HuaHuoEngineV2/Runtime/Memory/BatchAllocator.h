//
// Created by VincentZhang on 4/25/2022.
//

#ifndef HUAHUOENGINE_BATCHALLOCATOR_H
#define HUAHUOENGINE_BATCHALLOCATOR_H

#include <cstdlib>
#include "BaseClasses/BaseTypes.h"
#include "MemoryMacros.h"
#include "Utilities/remove_const.h"
#include "Logging/LogAssert.h"

class BatchAllocator {
public:
    enum CommitClearMode
    {
        kClearMemory,
        kLeaveUninitialized,
    };
public:
    BatchAllocator();

    void Commit(MemLabelRef label, CommitClearMode clearMode);

    void Commit(MemLabelRef label)
    {
        Commit(label, kLeaveUninitialized);
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

    static void DeallocateRoot(MemLabelRef label, void* dstPtr) { HUAHUO_FREE(label, dstPtr); }

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

template<class T>
void BatchAllocator::AllocateRoot(T*& dstPtr, size_t count, size_t alignment)
{
    // NOTE: For now we assume that first allocation is root allocation
    // If we have use cases for multiple root allocations, then we need to genaralize this code a bit more...
    typedef typename core::remove_const<T>::type MutableT;
    MutableT *&dstPtrMutable = const_cast<MutableT *&>(dstPtr);
    Assert((void*)&dstPtr == (void*)&dstPtrMutable);

    Assert(m_AllocationCount == 0);
    alignment = (alignment != 0) ? alignment : alignof(MutableT);

    AllocateInternal(reinterpret_cast<void**>(&dstPtrMutable), -1, sizeof(MutableT), count, alignment);

    dstPtrMutable = NULL;
}


template<class T>
void BatchAllocator::Allocate(T*& dstPtr, size_t count, size_t alignment)
{
    // NOTE: For now we assume that first allocation is root allocation
    // If we have use cases for multiple root allocations, then we need to genaralize this code a bit more...
    typedef typename core::remove_const<T>::type MutableT;
    MutableT *&dstPtrMutable = const_cast<MutableT *&>(dstPtr);
    Assert((void*)&dstPtr == (void*)&dstPtrMutable);

    Assert(m_AllocationCount >= 1);
    alignment = (alignment != 0) ? alignment : alignof(MutableT);

    AllocateInternal(reinterpret_cast<void**>(&dstPtrMutable), -1, sizeof(MutableT), count, alignment);
}

template<class T>
void BatchAllocator::Reallocate(T*& dstPtr, size_t newCount, size_t srcCount, size_t alignment)
{
    // NOTE: For now we assume that first allocation is root allocation
    // If we have use cases for multiple root allocations, then we need to genaralize this code a bit more...
    typedef typename core::remove_const<T>::type MutableT;
    MutableT *&dstPtrMutable = const_cast<MutableT *&>(dstPtr);
    Assert((void*)&dstPtr == (void*)&dstPtrMutable);

    alignment = (alignment != 0) ? alignment : alignof(MutableT);

    ReallocateInternal(reinterpret_cast<void**>(&dstPtrMutable), sizeof(MutableT), newCount, srcCount, alignment);
}

template<class T>
void BatchAllocator::AllocateField(T*& dstPtr, size_t count, size_t alignment)
{
    // NOTE: For now we assume that first allocation is root allocation
    // If we have use cases for multiple root allocations, then we need to genaralize this code a bit more...
    Assert(m_AllocationCount >= 1);

    typedef typename core::remove_const<T>::type MutableT;
    MutableT *&dstPtrMutable = const_cast<MutableT *&>(dstPtr);
    Assert((void*)&dstPtr == (void*)&dstPtrMutable);

    alignment = (alignment != 0) ? alignment : alignof(MutableT);

    AllocateInternal(reinterpret_cast<void**>(&dstPtrMutable), 0, sizeof(MutableT), count, alignment);
}

inline void BatchAllocator::AllocateField(void*& dstPtr, size_t count, size_t alignment, size_t sizeOf)
{
    // NOTE: For now we assume that first allocation is root allocation
    // If we have use cases for multiple root allocations, then we need to genaralize this code a bit more...
    Assert(m_AllocationCount >= 1);

    alignment = (alignment != 0) ? alignment : sizeOf;

    AllocateInternal(&dstPtr, 0, sizeOf, count, alignment);
}

#endif //HUAHUOENGINE_BATCHALLOCATOR_H
