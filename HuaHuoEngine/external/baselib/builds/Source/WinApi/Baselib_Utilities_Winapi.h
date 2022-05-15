#pragma once

#include "../Baselib_Utilities.h"

#include <windef.h>
#include <WinBase.h>

namespace WinApi
{
namespace detail
{
    template<typename BaselibHandle>
    static inline HANDLE AsHANDLE(BaselibHandle& baselibType)
    {
        return ::detail::AsNativeType<HANDLE>(baselibType);
    }

    static inline DWORD ResolveErrorCode(const DWORD error)
    {
        return error == WAIT_FAILED ? GetLastError() : error;
    }
}
}
