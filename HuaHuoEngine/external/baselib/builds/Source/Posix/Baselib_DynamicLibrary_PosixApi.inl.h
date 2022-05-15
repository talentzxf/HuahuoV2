#pragma once

#include "Include/Baselib.h"
#include "Include/C/Baselib_DynamicLibrary.h"
#include "Include/C/Baselib_Process.h"
#include "Source/Baselib_InlineImplementation.h"
#include "Source/Baselib_Utilities.h"
#include "Source/Posix/ErrorStateBuilder_PosixApi.inl.h"

#include <dlfcn.h>

namespace PosixApi
{
    BASELIB_INLINE_IMPL Baselib_DynamicLibrary_Handle Baselib_DynamicLibrary_Open(
        const char* pathname,
        Baselib_ErrorState* errorState
    )
    {
        const auto dylib = dlopen(pathname, RTLD_LAZY | RTLD_LOCAL);
        if (dylib != nullptr)
            return ::detail::AsBaselibHandle<Baselib_DynamicLibrary_Handle>(dylib);
        else
        {
            errorState |= RaiseError(Baselib_ErrorCode_FailedToOpenDynamicLibrary) | WithFormattedString("dlerror() = %s", dlerror());
            return Baselib_DynamicLibrary_Handle_Invalid;
        }
    }

    BASELIB_INLINE_IMPL void* Baselib_DynamicLibrary_GetFunction(
        Baselib_DynamicLibrary_Handle handle,
        const char* functionName,
        Baselib_ErrorState* errorState
    )
    {
        const auto dylib = ::detail::AsNativeType<void*>(handle);

        // must clear the error code
        dlerror();

        const auto func = dlsym(dylib, functionName);

        // func == nullptr is a valid value, so need to check for dlerror
        if (func == nullptr && dlerror() != nullptr)
            errorState |= RaiseError(Baselib_ErrorCode_FunctionNotFound) | WithFormattedString("dlerror() = %s", dlerror());

        return func;
    }

    BASELIB_INLINE_IMPL void Baselib_DynamicLibrary_Close(
        Baselib_DynamicLibrary_Handle handle
    )
    {
        const auto dylib = ::detail::AsNativeType<void*>(handle);
        if (dlclose(dylib) != 0)
            Baselib_Process_Abort(Baselib_ErrorCode_UnexpectedError);
    }
}
