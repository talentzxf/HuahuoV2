#pragma once

#include "Include/C/Baselib_Memory.h"
#include "Include/C/Baselib_Process.h"
#include "Source/Baselib_InlineImplementation.h"
#include "Source/Baselib_Memory_Utils.h"

#include <stdlib.h>
#include <string.h>

namespace C99Api
{
    BASELIB_INLINE_IMPL void* Baselib_Memory_Allocate(size_t size)
    {
        void* allocatedMemory = ::malloc(size);
        if (OPTIMIZER_LIKELY(allocatedMemory != nullptr))
            return allocatedMemory;
        if (size == 0)
            return Baselib_Memory_Allocate(1);

        // Given that we checked for all conditions, a nullptr *should* mean out of memory.
        // Platform specific implementations might be more accurate here.
        ::Baselib_Process_Abort(Baselib_ErrorCode_OutOfMemory);
    }

    BASELIB_INLINE_IMPL void* Baselib_Memory_Reallocate(void* ptr, size_t newSize)
    {
        void* reallocatedMemory = ::realloc(ptr, newSize);
        if (OPTIMIZER_LIKELY(reallocatedMemory != nullptr))
            return reallocatedMemory;
        if (newSize == 0)
            return Baselib_Memory_Allocate(newSize);

        // Given that we checked for all conditions, a nullptr *should* mean out of memory.
        // Platform specific implementations might be more accurate here.
        ::Baselib_Process_Abort(Baselib_ErrorCode_OutOfMemory);
    }

    BASELIB_INLINE_IMPL void Baselib_Memory_Free(void* ptr)
    {
        ::free(ptr);
    }

//
// C99 doesn't have any means of doing aligned allocations,
// and even though posix may have posix_memalign (or memalign)
// those functions are not guaranteed to work with realloc
//
// due to this limitation we have our own alignment built on
// top of regular malloc.
//
// C11/C++17/glibc has aligned_alloc which does work with regular
// realloc. Still doesn't give any guarantees as in terms
// of alignment on realloc but it doesn't crash and doesn't
// waste as much memory as our own C99 implementation.
//
namespace detail
{
    static inline size_t Baselib_Memory_Max(size_t a, size_t b)
    {
        return a >= b ? a : b;
    }

    static inline size_t Baselib_Memory_PaddingForHeaderAndAlignment(size_t alignment)
    {
        // We use max here on the assumption user alignment as well as
        // malloc system alignment is at least size of a pointer
        return Baselib_Memory_Max(sizeof(void*), alignment);
    }

    static inline void* Baselib_Memory_AlignPointer(void* blockPtr, size_t alignment)
    {
        return (void*)(((uintptr_t)blockPtr + Baselib_Memory_PaddingForHeaderAndAlignment(alignment)) & ~(alignment - 1));
    }

    static inline void* Baselib_Memory_ReadBlockPointerInHeader(void* alignedPtr)
    {
        return static_cast<void**>(alignedPtr)[-1];
    }

    static inline void Baselib_Memory_StoreBlockPointerInHeader(void* alignedPtr, void* blockPtr)
    {
        BaselibAssert(alignedPtr > blockPtr);
        static_cast<void**>(alignedPtr)[-1] = blockPtr;
    }
}

    BASELIB_INLINE_IMPL void* Baselib_Memory_AlignedAllocate(size_t size, size_t alignment)
    {
        const size_t padding = detail::Baselib_Memory_PaddingForHeaderAndAlignment(alignment);
        if (baselib::Algorithm::DoesAdditionOverflow(size, padding))
            ::Baselib_Process_Abort(Baselib_ErrorCode_OutOfMemory);

        void* blockPtr = Baselib_Memory_Allocate(size + padding);
        if (blockPtr == nullptr)
            return nullptr;

        void* ptr = detail::Baselib_Memory_AlignPointer(blockPtr, alignment);
        detail::Baselib_Memory_StoreBlockPointerInHeader(ptr, blockPtr);
        return ptr;
    }

    BASELIB_INLINE_IMPL void* Baselib_Memory_AlignedReallocate(void* oldPtr, size_t newSize, size_t alignment)
    {
        if (oldPtr == nullptr)
            return Baselib_Memory_AlignedAllocate(newSize, alignment);

        void* oldBlockPtr = detail::Baselib_Memory_ReadBlockPointerInHeader(oldPtr);
        const uintptr_t oldPointerOffset = (uintptr_t)oldPtr - (uintptr_t)oldBlockPtr;
        const size_t minPadding = detail::Baselib_Memory_PaddingForHeaderAndAlignment(alignment);
        const size_t padding = detail::Baselib_Memory_Max(minPadding, oldPointerOffset);
        if (baselib::Algorithm::DoesAdditionOverflow(newSize, padding))
            ::Baselib_Process_Abort(Baselib_ErrorCode_OutOfMemory);

        void* newBlockPtr = Baselib_Memory_Reallocate(oldBlockPtr, newSize + padding);
        if (newBlockPtr == nullptr)
            return nullptr;

        void* ptr = (void*)((uintptr_t)newBlockPtr + oldPointerOffset);
        if (!IsAligned(ptr, alignment))
        {
            void* newPtr = detail::Baselib_Memory_AlignPointer(newBlockPtr, alignment);
            ::memmove(newPtr, ptr, newSize);
            ptr = newPtr;
        }
        detail::Baselib_Memory_StoreBlockPointerInHeader(ptr, newBlockPtr);
        return ptr;
    }

    BASELIB_INLINE_IMPL void  Baselib_Memory_AlignedFree(void* ptr)
    {
        if (ptr == nullptr)
            return;
        Baselib_Memory_Free(detail::Baselib_Memory_ReadBlockPointerInHeader(ptr));
    }
}
