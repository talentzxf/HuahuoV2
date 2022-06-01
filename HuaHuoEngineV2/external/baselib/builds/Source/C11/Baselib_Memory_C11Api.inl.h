#pragma once

#include "Source/Baselib_InlineImplementation.h"
#include "Include/C/Baselib_Memory.h"

#include "Source/Baselib_Memory_Utils.h"

#include <stdlib.h>
#include <string.h>


// Windows does not have and may never get support for aligned_alloc:
//
// Source: https://blogs.msdn.microsoft.com/vcblog/2017/08/11/c17-features-and-stl-fixes-in-vs-2017-15-3/
//
// aligned_alloc() will probably never be implemented, as baselib specified it in a way that’s incompatible with our implementation
// (namely, that free() must be able to handle highly aligned allocations).

namespace C11Api
{
    BASELIB_INLINE_IMPL void* Baselib_Memory_AlignedAllocate(size_t size, size_t alignment)
    {
        void* allocatedMemory = ::aligned_alloc(alignment, size);
        if (OPTIMIZER_LIKELY(allocatedMemory != nullptr))
            return allocatedMemory;
        if (size == 0)
            return Baselib_Memory_AlignedAllocate(1, alignment);

        // Given that we checked for all conditions, a nullptr *should* mean out of memory.
        // Platform specific implementations might be more accurate here.
        Baselib_Process_Abort(Baselib_ErrorCode_OutOfMemory);
    }

    BASELIB_INLINE_IMPL void* Baselib_Memory_AlignedReallocate(void* ptr, size_t newSize, size_t alignment)
    {
        if (ptr == nullptr)
            return Baselib_Memory_AlignedAllocate(newSize, alignment);

        ptr = ::realloc(ptr, newSize);
        if (ptr == nullptr)
        {
            if (newSize == 0)
                return Baselib_Memory_AlignedAllocate(newSize, alignment);

            // Given that we checked for all conditions, a nullptr *should* mean out of memory.
            // Platform specific implementations might be more accurate here.
            Baselib_Process_Abort(Baselib_ErrorCode_OutOfMemory);
        }

        // There is no standardized way to reallocate with alignment.
        // Also, realloc does not give any guarantees on alignment.
        if (IsAligned(ptr, alignment))
            return ptr;

        void* newPtr = Baselib_Memory_AlignedAllocate(newSize, alignment);
        memcpy(newPtr, ptr, newSize);
        Baselib_Memory_AlignedFree(ptr);
        return newPtr;
    }

    BASELIB_INLINE_IMPL void Baselib_Memory_AlignedFree(void* ptr)
    {
        free(ptr);
    }
}
