#pragma once

#include "Include/C/Baselib_Memory.h"

static inline void Baselib_Memory_Validate_Alignment(size_t alignment)
{
    BaselibAssert(alignment >= sizeof(void*) && alignment <= Baselib_Memory_MaxAlignment && baselib::Algorithm::IsPowerOfTwo(alignment), "Invalid alignment: %zu", alignment);
    if (alignment >= sizeof(void*) && alignment <= Baselib_Memory_MaxAlignment && baselib::Algorithm::IsPowerOfTwo(alignment))
        return;
    Baselib_Process_Abort(Baselib_ErrorCode_UnsupportedAlignment);
}

static inline void Baselib_ErrorState_Validate_PageRange(uint64_t pageSize, uint64_t pageCount, uint64_t alignmentInMultipleOfPageSize, const Baselib_Memory_PageSizeInfo& pageSizeInfo, Baselib_ErrorState* errorState)
{
    if (pageCount == 0)
    {
        errorState |= RaiseError(Baselib_ErrorCode_InvalidPageCount);
        return;
    }

    bool supportedPageSize = false;
    for (uint64_t i = 0; i < pageSizeInfo.pageSizesLen; ++i)
    {
        if (pageSizeInfo.pageSizes[i] == pageSize)
        {
            supportedPageSize = true;
            break;
        }
    }
    if (!supportedPageSize)
        errorState |= RaiseError(Baselib_ErrorCode_InvalidPageSize);

    if (alignmentInMultipleOfPageSize == 0 || !baselib::Algorithm::IsPowerOfTwo(alignmentInMultipleOfPageSize))
        errorState |= RaiseError(Baselib_ErrorCode_UnsupportedAlignment);


    if (baselib::Algorithm::DoesMultiplicationOverflow(pageSize, pageCount))
        errorState |= RaiseError(Baselib_ErrorCode_OutOfMemory);
    if (baselib::Algorithm::DoesMultiplicationOverflow(alignmentInMultipleOfPageSize, pageCount))
        errorState |= RaiseError(Baselib_ErrorCode_UnsupportedAlignment);
    // All implementations use byte counts with size_t internally. This may overflow, so protect against it.
    if (!std::is_same<size_t, uint64_t>::value && pageSize * pageCount > std::numeric_limits<size_t>::max())
        errorState |= RaiseError(Baselib_ErrorCode_OutOfMemory);
}

static inline void Baselib_ErrorState_Validate_PageRange(uint64_t pageSize, uint64_t pageCount, const Baselib_Memory_PageSizeInfo& pageSizeInfo, Baselib_ErrorState* errorState)
{
    Baselib_ErrorState_Validate_PageRange(pageSize, pageCount, 1, pageSizeInfo, errorState);
}

static inline void Baselib_ErrorState_Validate_PageRange(uint64_t pageSize, uint64_t pageCount, Baselib_ErrorState* errorState)
{
    Baselib_Memory_PageSizeInfo sizeInfo;
    Baselib_Memory_GetPageSizeInfo(&sizeInfo);
    Baselib_ErrorState_Validate_PageRange(pageSize, pageCount, sizeInfo, errorState);
}

static inline void Baselib_ErrorState_Validate_PageRange(const Baselib_Memory_PageAllocation& pageAllocation, Baselib_ErrorState* errorState)
{
    return Baselib_ErrorState_Validate_PageRange(pageAllocation.pageSize, pageAllocation.pageCount, errorState);
}

BASELIB_C_INTERFACE
{
    void Baselib_Memory_GetPageSizeInfo(Baselib_Memory_PageSizeInfo* outPagesSizeInfo)
    {
        if (!outPagesSizeInfo)
            return;
        return platform::Baselib_Memory_GetPageSizeInfo(outPagesSizeInfo);
    }

    void* Baselib_Memory_Allocate(size_t size)
    {
        return platform::Baselib_Memory_Allocate(size);
    }

    void* Baselib_Memory_Reallocate(void* ptr, size_t newSize)
    {
        return platform::Baselib_Memory_Reallocate(ptr, newSize);
    }

    void  Baselib_Memory_Free(void* ptr)
    {
        return platform::Baselib_Memory_Free(ptr);
    }

    void* Baselib_Memory_AlignedAllocate(size_t size, size_t alignment)
    {
        Baselib_Memory_Validate_Alignment(alignment);
        if (alignment < Baselib_Memory_MinGuaranteedAlignment)
            alignment = Baselib_Memory_MinGuaranteedAlignment;
        return platform::Baselib_Memory_AlignedAllocate(size, alignment);
    }

    void* Baselib_Memory_AlignedReallocate(void* ptr, size_t newSize, size_t alignment)
    {
        Baselib_Memory_Validate_Alignment(alignment);
        return platform::Baselib_Memory_AlignedReallocate(ptr, newSize, alignment);
    }

    void  Baselib_Memory_AlignedFree(void* ptr)
    {
        return platform::Baselib_Memory_AlignedFree(ptr);
    }

    Baselib_Memory_PageAllocation Baselib_Memory_AllocatePages(uint64_t pageSize, uint64_t pageCount, uint64_t alignmentInMultipleOfPageSize, Baselib_Memory_PageState pageState, Baselib_ErrorState* errorState)
    {
        Baselib_Memory_PageSizeInfo sizeInfo;
        Baselib_Memory_GetPageSizeInfo(&sizeInfo);
        Baselib_ErrorState_Validate_PageRange(pageSize, pageCount, alignmentInMultipleOfPageSize, sizeInfo, errorState);
        if (Baselib_ErrorState_ErrorRaised(errorState))
            return Baselib_Memory_PageAllocation_Invalid;

        return platform::Baselib_Memory_AllocatePages(pageSize, pageCount, alignmentInMultipleOfPageSize, pageState, errorState);
    }

    void Baselib_Memory_ReleasePages(Baselib_Memory_PageAllocation pageAllocation, Baselib_ErrorState* errorState)
    {
        if (pageAllocation.ptr == nullptr || pageAllocation.pageCount == 0)
            return;

        Baselib_ErrorState_Validate_PageRange(pageAllocation, errorState);
        if (Baselib_ErrorState_ErrorRaised(errorState))
            return;

        return platform::Baselib_Memory_ReleasePages(pageAllocation, errorState);
    }

    void Baselib_Memory_SetPageState(void* addressOfFirstPage, uint64_t pageSize, uint64_t pageCount, Baselib_Memory_PageState pageState, Baselib_ErrorState* errorState)
    {
        if (addressOfFirstPage == nullptr || pageCount == 0)
            return;
        Baselib_ErrorState_Validate_PageRange(pageSize, pageCount, errorState);
        if (Baselib_ErrorState_ErrorRaised(errorState))
            return;

        return platform::Baselib_Memory_SetPageState(addressOfFirstPage, pageSize, pageCount, pageState, errorState);
    }
}
