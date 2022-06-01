#pragma once

#include "Include/Baselib.h"
#include "Include/C/Baselib_DynamicLibrary.h"
#include "Include/C/Baselib_Process.h"
#include "Source/Baselib_InlineImplementation.h"
#include "Source/Baselib_Utilities.h"
#include "Source/WinApi/StringConversions_WinApi.inl.h"
#include "Source/WinApi/ErrorStateBuilder_WinApi.inl.h"

#include <windows.h>

namespace WinApi
{
    BASELIB_INLINE_IMPL Baselib_DynamicLibrary_Handle Baselib_DynamicLibrary_Open(
        const char* pathname,
        Baselib_ErrorState* errorState
    )
    {
        const auto pathnameW = WinApi_StringConversions_UTF8ToUTF16(pathname);
        #if PLATFORM_WINRT
        const auto module = LoadPackagedLibrary(pathnameW.c_str(), 0);
        #else
        const auto module = LoadLibraryW(pathnameW.c_str());
        #endif

        if (module != NULL)
            return ::detail::AsBaselibHandle<Baselib_DynamicLibrary_Handle>(module);
        else
        {
            errorState |= RaiseError(Baselib_ErrorCode_FailedToOpenDynamicLibrary) | WithGetLastError();
            return Baselib_DynamicLibrary_Handle_Invalid;
        }
    }

    BASELIB_INLINE_IMPL void* Baselib_DynamicLibrary_GetFunction(
        Baselib_DynamicLibrary_Handle handle,
        const char* functionName,
        Baselib_ErrorState* errorState
    )
    {
        const auto module = ::detail::AsNativeType<HMODULE>(handle);
        const auto func = GetProcAddress(module, functionName);
        if (func == NULL)
        {
            const auto lastError = GetLastError();
            if (lastError == ERROR_PROC_NOT_FOUND)
                errorState |= RaiseError(Baselib_ErrorCode_FunctionNotFound) | WithGetLastError(lastError);
            else
                errorState |= RaiseError(Baselib_ErrorCode_UnexpectedError) | WithGetLastError(lastError);
        }
        return func;
    }

    BASELIB_INLINE_IMPL void Baselib_DynamicLibrary_Close(
        Baselib_DynamicLibrary_Handle handle
    )
    {
        const auto module = ::detail::AsNativeType<HMODULE>(handle);
        if (FreeLibrary(module) == 0)
            Baselib_Process_Abort(Baselib_ErrorCode_UnexpectedError);
    }
}
