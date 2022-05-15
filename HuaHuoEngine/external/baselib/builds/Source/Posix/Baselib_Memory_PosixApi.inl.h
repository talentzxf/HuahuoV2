#pragma once

#include "Source/Baselib_InlineImplementation.h"
#include "Include/C/Baselib_Memory.h"
#include "Source/Baselib_Memory_Utils.h"
#include "Source/Posix/ErrorStateBuilder_PosixApi.inl.h"

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/mman.h>

namespace PosixApi
{
namespace detail
{
    static void Memory_RaiseError(Baselib_ErrorState* errorState)
    {
        int errnoErrorCode = errno;
        switch (errnoErrorCode)
        {
            case ENOMEM:
                errorState |= RaiseError(Baselib_ErrorCode_OutOfMemory) | WithErrno(errnoErrorCode);
                return;
            case EACCES:
                errorState |= RaiseError(Baselib_ErrorCode_UnsupportedPageState) | WithErrno(errnoErrorCode);
                return;
        }
        BaselibAssert(false, "Posix api function failed with unexpected error: %d", errnoErrorCode);
        errorState |= RaiseError(Baselib_ErrorCode_UnexpectedError) | WithErrno(errnoErrorCode);
    }

    static int Memory_PageStateToPosixPageProtection(Baselib_Memory_PageState pageState, Baselib_ErrorState* errorState)
    {
        switch (pageState)
        {
            case Baselib_Memory_PageState_Reserved:
            case Baselib_Memory_PageState_NoAccess:
                return PROT_NONE;
            case Baselib_Memory_PageState_ReadOnly:
                return PROT_READ;
            case Baselib_Memory_PageState_ReadWrite:
                return PROT_READ | PROT_WRITE;

            case Baselib_Memory_PageState_ReadOnly_Executable:
                return PROT_EXEC | PROT_READ;
            case Baselib_Memory_PageState_ReadWrite_Executable:
                return PROT_EXEC | PROT_READ | PROT_WRITE;
        }

        errorState |= RaiseError(Baselib_ErrorCode_UnsupportedPageState);
        return 0;
    }
}

    BASELIB_INLINE_IMPL void Baselib_Memory_GetPageSizeInfo(Baselib_Memory_PageSizeInfo* outPagesSizeInfo)
    {
        // There seems to be no pure POSIX way to retrieve large page sizes.
        //
        // Note that some platforms may vary their page size from device to device (even if pointer size is the same)
        // For example the iOS transition guide explicitely states "Never Hard-Code the Virtual Memory Page Size"
        // http://cdn.cocimg.com/cms/uploads/soft/130911/4196-130911095Z1.pdf
        outPagesSizeInfo->defaultPageSize = outPagesSizeInfo->pageSizes[0] = getpagesize();
        outPagesSizeInfo->pageSizesLen = 1;
    }

    BASELIB_INLINE_IMPL Baselib_Memory_PageAllocation Baselib_Memory_AllocatePages(uint64_t pageSize, uint64_t pageCount, uint64_t alignmentInMultipleOfPageSize, Baselib_Memory_PageState pageState, Baselib_ErrorState* errorState)
    {
        const size_t totalReservedBytes = Baselib_MemoryState_Utils_GetReservedByteCountWithAlignmentPadding(pageSize, pageCount, alignmentInMultipleOfPageSize, errorState);
        const int protection = detail::Memory_PageStateToPosixPageProtection(static_cast<Baselib_Memory_PageState>(pageState), errorState);
        if (Baselib_ErrorState_ErrorRaised(errorState))
            return Baselib_Memory_PageAllocation_Invalid;

        const int flags = MAP_ANONYMOUS | MAP_PRIVATE;
        void* const mappedAddress = mmap(nullptr, totalReservedBytes, protection, flags, -1, 0);
        if (mappedAddress == MAP_FAILED)
        {
            detail::Memory_RaiseError(errorState);
            return Baselib_Memory_PageAllocation_Invalid;
        }

        uint8_t* const alignedPointer = (uint8_t*)NextAlignedPointerIfNotAligned(mappedAddress, alignmentInMultipleOfPageSize * pageSize);

        // Unmap memory before and after the aligned block.
        const size_t paddingSizeFront = alignedPointer - (uint8_t*)mappedAddress;
        if (paddingSizeFront > 0)
        {
            if (munmap(mappedAddress, paddingSizeFront) != 0)
                detail::Memory_RaiseError(errorState);
        }
        const size_t paddingSizeBack = totalReservedBytes - paddingSizeFront - pageSize * pageCount;
        if (paddingSizeBack > 0)
        {
            if (munmap(alignedPointer + pageSize * pageCount, paddingSizeBack) != 0)
                detail::Memory_RaiseError(errorState);
        }

        BaselibAssert(IsAligned(alignedPointer, alignmentInMultipleOfPageSize * pageSize));
        return Baselib_Memory_PageAllocation {alignedPointer, pageSize, pageCount};
    }

    BASELIB_INLINE_IMPL void Baselib_Memory_ReleasePages(Baselib_Memory_PageAllocation pageAllocation, Baselib_ErrorState* errorState)
    {
        if (munmap(pageAllocation.ptr, pageAllocation.pageSize * pageAllocation.pageCount) != 0)
            detail::Memory_RaiseError(errorState);
    }
}
