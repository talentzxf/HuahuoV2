#pragma once

#include "Source/Baselib_InlineImplementation.h"
#include "../Posix/Baselib_Memory_PosixApi.inl.h"

namespace DarwinApi
{
    BASELIB_INLINE_IMPL void Baselib_Memory_SetPageState(void* addressOfFirstPage, uint64_t pageSize, uint64_t pageCount, Baselib_Memory_PageState pageState, Baselib_ErrorState* errorState)
    {
        int protection = PosixApi::detail::Memory_PageStateToPosixPageProtection(static_cast<Baselib_Memory_PageState>(pageState), errorState);
        if (Baselib_ErrorState_ErrorRaised(errorState))
            return;

        // Baselib_ErrorState_Validate_PageRange already made sure that cast to size_t is safe.
        const size_t byteCount = (size_t)(pageSize * pageCount);

        if (pageState == Baselib_Memory_PageState_Reserved)
        {
            // We prefer MADV_FREE_REUSABLE over MADV_FREE where available (as of writing that is only on OSX).
            // It explicitly allows reuse by any other applications whereas MADV_FREE merely marks as freed.
            // Consequently, any tool showing the system's physical memory use, typically only shows memory marked with MADV_FREE_REUSABLE as available.
            if (madvise(addressOfFirstPage, byteCount, MADV_FREE_REUSABLE) != 0)
            {
                if (madvise(addressOfFirstPage, byteCount, MADV_FREE) != 0)
                    PosixApi::detail::Memory_RaiseError(errorState);
            }
        }
        else
        {
            if (madvise(addressOfFirstPage, byteCount, MADV_NORMAL) != 0)
                PosixApi::detail::Memory_RaiseError(errorState);
        }

        if (mprotect(addressOfFirstPage, byteCount, protection) != 0)
            PosixApi::detail::Memory_RaiseError(errorState);
    }
}
