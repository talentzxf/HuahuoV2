#pragma once

#include "Include/C/Baselib_DynamicLibrary.h"
#include "Source/ArgumentValidator.h"

static inline bool IsValid(const Baselib_DynamicLibrary_Handle& handle)
{
    return handle.handle != Baselib_DynamicLibrary_Handle_Invalid.handle;
}

BASELIB_C_INTERFACE
{
    Baselib_DynamicLibrary_Handle Baselib_DynamicLibrary_Open(
        const char* pathname,
        Baselib_ErrorState* errorState
    )
    {
        errorState |= Validate(AsPointer(pathname));
        if (Baselib_ErrorState_ErrorRaised(errorState))
            return Baselib_DynamicLibrary_Handle_Invalid;

        return platform::Baselib_DynamicLibrary_Open(pathname, errorState);
    }

    void* Baselib_DynamicLibrary_GetFunction(
        Baselib_DynamicLibrary_Handle handle,
        const char* functionName,
        Baselib_ErrorState* errorState
    )
    {
        errorState |= Validate(handle);
        errorState |= Validate(AsPointer(functionName));
        if (Baselib_ErrorState_ErrorRaised(errorState))
            return nullptr;
        return platform::Baselib_DynamicLibrary_GetFunction(handle, functionName, errorState);
    }

    void Baselib_DynamicLibrary_Close(
        Baselib_DynamicLibrary_Handle handle
    )
    {
        if (!IsValid(handle))
            return;
        return platform::Baselib_DynamicLibrary_Close(handle);
    }
}
