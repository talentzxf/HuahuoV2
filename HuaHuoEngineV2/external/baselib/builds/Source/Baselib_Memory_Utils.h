#pragma once

#include "Include/C/Baselib_Memory.h"
#include "Include/C/Baselib_Process.h"
#include "Include/Cpp/Algorithm.h"
#include "Source/ErrorStateBuilder.h"
#include <limits>
#include <algorithm>

static inline bool IsAligned(const void* value, uint64_t alignment)
{
    return (reinterpret_cast<uintptr_t>(value) & (alignment - 1)) == 0;
}

static inline void* NextAlignedPointerIfNotAligned(const void* value, uint64_t alignment)
{
    return reinterpret_cast<void*>((reinterpret_cast<uintptr_t>(value) + alignment - 1) & ~(alignment - 1));
}

static inline size_t Baselib_MemoryState_Utils_GetReservedByteCountWithAlignmentPadding(uint64_t pageSize, uint64_t pageCount, uint64_t alignmentInMultipleOfPageSize, Baselib_ErrorState* errorState)
{
    // We assume that Baselib_ErrorState_Validate_PageRange was called prior to this.
    const uint64_t paddingInMultipleOfPageSize = alignmentInMultipleOfPageSize - 1;
    if (baselib::Algorithm::DoesAdditionOverflow(pageSize, paddingInMultipleOfPageSize) ||
        baselib::Algorithm::DoesMultiplicationOverflow(paddingInMultipleOfPageSize + pageCount, pageSize))
    {
        errorState |= RaiseError(Baselib_ErrorCode_UnsupportedAlignment);
        return 0;
    }

    const uint64_t reservedByteCountWithAlignmentPadding = (paddingInMultipleOfPageSize + pageCount) * pageSize;

    // Total byte count always needs to fit in a size_t!
    if (!std::is_same<size_t, uint64_t>::value && reservedByteCountWithAlignmentPadding > std::numeric_limits<size_t>::max())
    {
        errorState |= RaiseError(Baselib_ErrorCode_UnsupportedAlignment);
        return 0;
    }

    return (size_t)(reservedByteCountWithAlignmentPadding);
}
