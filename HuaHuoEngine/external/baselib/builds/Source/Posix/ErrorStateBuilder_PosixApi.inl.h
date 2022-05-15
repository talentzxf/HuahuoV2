#pragma once

#include "Source/ErrorStateBuilder.h"

#include <errno.h>

struct WithErrno : ::detail::ErrorStateBuilder::InfoProvider<WithErrno>
{
    int value;

    WithErrno()
        : value(errno)
    {
    }

    WithErrno(int setValue)
        : value(setValue)
    {
    }

    inline void Fill(Baselib_ErrorState& state) const
    {
        state.nativeErrorCodeType = Baselib_ErrorState_NativeErrorCodeType_errno;
        state.nativeErrorCode = (uint64_t)value;
    }
};
