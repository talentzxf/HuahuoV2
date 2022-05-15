#pragma once

#include "Source/ErrorStateBuilder.h"

#include <errhandlingapi.h>

struct WithGetLastError : ::detail::ErrorStateBuilder::InfoProvider<WithGetLastError>
{
    DWORD value;

    WithGetLastError()
        : value(GetLastError())
    {
    }

    WithGetLastError(DWORD setValue)
        : value(setValue)
    {
    }

    inline void Fill(Baselib_ErrorState& state) const
    {
        state.nativeErrorCodeType = Baselib_ErrorState_NativeErrorCodeType_GetLastError;
        state.nativeErrorCode = (uint64_t)value;
    }
};
