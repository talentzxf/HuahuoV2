#pragma once

#include "Source/WinApi/Baselib_Debug_WinApi.inl.h"

#include "Source/WinApi/Baselib_ErrorState_WinApi.inl.h"
#include "Source/WinApi/Baselib_Memory_WinApi.inl.h"
#include "Source/WinApi/Baselib_Socket_WinApi.inl.h"
#include "Source/WinApi/Baselib_SystemSemaphore_WinApi.inl.h"
#include "Source/WinApi/Baselib_Timer_WinApi.inl.h"
#include "Source/WinApi/Baselib_FileIO_WinApi.inl.h"
#include "Source/WinApi/Baselib_DynamicLibrary_WinApi.inl.h"

#include "Source/Cpp11/Baselib_Timer_Cpp11.inl.h"
#include "Source/C99/Baselib_Process_C99Api.inl.h"
#include "Source/C99/Baselib_Memory_C99Api.inl.h"

#include "Source/Common/Baselib_Time_Common.inl.h"

namespace platform
{
    // Baselib_Debug
    using WinApi::Baselib_Debug_IsDebuggerAttached;

    // Baselib_ErrorState
    using WinApi::Baselib_ErrorState_Explain;

    // Baselib_Process
    using C99Api::Baselib_Process_Abort;

    // Baselib_SystemSemaphore
    using WinApi::Baselib_SystemSemaphore_Create;
    using WinApi::Baselib_SystemSemaphore_Acquire;
    using WinApi::Baselib_SystemSemaphore_TryAcquire;
    using WinApi::Baselib_SystemSemaphore_TryTimedAcquire;
    using WinApi::Baselib_SystemSemaphore_Release;
    using WinApi::Baselib_SystemSemaphore_Free;

    // Baselib_Memory
    using WinApi::Baselib_Memory_GetPageSizeInfo;
    using C99Api::Baselib_Memory_Allocate;
    using C99Api::Baselib_Memory_Reallocate;
    using C99Api::Baselib_Memory_Free;
    using WinApi::Baselib_Memory_AlignedAllocate;
    using WinApi::Baselib_Memory_AlignedReallocate;
    using WinApi::Baselib_Memory_AlignedFree;
    using WinApi::Baselib_Memory_AllocatePages;
    using WinApi::Baselib_Memory_ReleasePages;
    using WinApi::Baselib_Memory_SetPageState;

    // Baselib_Timer
    using WinApi::Baselib_Timer_GetTicksToNanosecondsConversionRatio;
    using WinApi::Baselib_Timer_GetHighPrecisionTimerTicks;
    using Cpp11Api::Baselib_Timer_WaitForAtLeast;
    using Common::Baselib_Timer_GetTimeSinceStartupInSeconds;

    // Baselib_Socket
    using WinApi::Baselib_Socket_Create;
    using WinApi::Baselib_Socket_Bind;
    using WinApi::Baselib_Socket_TCP_Connect;
    using WinApi::Baselib_Socket_Poll;
    using WinApi::Baselib_Socket_GetAddress;
    using WinApi::Baselib_Socket_TCP_Listen;
    using WinApi::Baselib_Socket_TCP_Accept;
    using WinApi::Baselib_Socket_UDP_Send;
    using WinApi::Baselib_Socket_TCP_Send;
    using WinApi::Baselib_Socket_UDP_Recv;
    using WinApi::Baselib_Socket_TCP_Recv;
    using WinApi::Baselib_Socket_Close;

    // Baselib_FileIO
    using WinApi::Baselib_FileIO_EventQueue_Create;
    using WinApi::Baselib_FileIO_EventQueue_Free;
    using WinApi::Baselib_FileIO_EventQueue_Dequeue;
    using WinApi::Baselib_FileIO_File_Open;
    using WinApi::Baselib_FileIO_File_Read;
    using WinApi::Baselib_FileIO_File_Close;

    // Baselib_DynamicLibrary
    using WinApi::Baselib_DynamicLibrary_Open;
    using WinApi::Baselib_DynamicLibrary_GetFunction;
    using WinApi::Baselib_DynamicLibrary_Close;
}

#include "Source/CProxy/Baselib_Debug_CProxy.inl.h"
#include "Source/CProxy/Baselib_ErrorState_CProxy.inl.h"
#include "Source/CProxy/Baselib_Memory_CProxy.inl.h"
#include "Source/CProxy/Baselib_Process_CProxy.inl.h"
#include "Source/CProxy/Baselib_Socket_CProxy.inl.h"
#include "Source/CProxy/Baselib_SystemSemaphore_CProxy.inl.h"
#include "Source/CProxy/Baselib_Timer_CProxy.inl.h"
#include "Source/CProxy/Baselib_FileIO_CProxy.inl.h"
#include "Source/CProxy/Baselib_DynamicLibrary_CProxy.inl.h"
