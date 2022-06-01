#pragma once

#include "Source/Baselib_InlineImplementation.h"
#include "../Posix/Baselib_Memory_PosixApi.inl.h"

namespace LinuxApi
{
    BASELIB_INLINE_IMPL void Baselib_Memory_SetPageState(void* addressOfFirstPage, uint64_t pageSize, uint64_t pageCount, Baselib_Memory_PageState pageState, Baselib_ErrorState* errorState)
    {
        int protection = PosixApi::detail::Memory_PageStateToPosixPageProtection(static_cast<Baselib_Memory_PageState>(pageState), errorState);
        if (Baselib_ErrorState_ErrorRaised(errorState))
            return;

        // Baselib_ErrorState_Validate_PageRange already made sure that cast to size_t is safe.
        const size_t byteCount = (size_t)(pageSize * pageCount);

        // Tell kernel that it can free the memory in the given range.
        // Note that there is no guarantee on when freeing happens.
        // Subsequent accesses to the memory would repopulate it, but since we're also setting mprotect to PROT_NONE, this would result in a segfault.
        // MADV_FREE is a bit closer to what we want, but it is supported only in linux kernels >4.5
        //
        // In contrast to BSD's MADV_DONTNEED and POSIX_MADV_DONTNEED, on Linux MADV_DONTNEED means that the underlying physical pages may be reused for different allocations.
        if (pageState == Baselib_Memory_PageState_Reserved)
        {
            if (madvise(addressOfFirstPage, byteCount, MADV_DONTNEED))
                PosixApi::detail::Memory_RaiseError(errorState);
        }
        else
        {
            if (madvise(addressOfFirstPage, byteCount, MADV_NORMAL))
                PosixApi::detail::Memory_RaiseError(errorState);
        }

        if (mprotect(addressOfFirstPage, byteCount, protection) != 0)
            PosixApi::detail::Memory_RaiseError(errorState);
    }
}
