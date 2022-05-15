#pragma once

#include "Source/Darwin/Baselib_Debug_DarwinApi.inl.h"
#include "Source/Darwin/Baselib_SystemSemaphore_DarwinApi.inl.h"
#include "Source/Darwin/Baselib_Thread_DarwinApi.inl.h"
#include "Source/Darwin/Baselib_Timer_DarwinApi.inl.h"
#include "Source/Darwin/Baselib_Memory_DarwinApi.inl.h"

#include "Source/Posix/Baselib_ErrorState_PosixApi.inl.h"
#include "Source/Posix/Baselib_Thread_PosixApi.inl.h"
#include "Source/Posix/Baselib_ThreadLocalStorage_PosixApi.inl.h"
#include "Source/Posix/Baselib_Timer_PosixApi.inl.h"
#include "Source/Posix/Baselib_Socket_PosixApi.inl.h"
#include "Source/Posix/Baselib_FileIO_PosixApi.inl.h"
#include "Source/Posix/Baselib_DynamicLibrary_PosixApi.inl.h"

#include "Source/C99/Baselib_Process_C99Api.inl.h"
#include "Source/C99/Baselib_Memory_C99Api.inl.h"

#include "Source/Cpp11/Baselib_Timer_Cpp11.inl.h"

#include "Source/Common/Baselib_RegisteredNetwork_Common.inl.h"

#if PLATFORM_FUTEX_NATIVE_SUPPORT
    #include "Source/Darwin/Baselib_SystemFutex_DarwinApi.inl.h"
#else
    #include "Source/Cpp11/Baselib_SystemFutex_Cpp11.inl.h"
#endif

namespace platform
{
    // Baselib_Debug
    using DarwinApi::Baselib_Debug_IsDebuggerAttached;

    // Baselib_ErrorState
    using PosixApi::Baselib_ErrorState_Explain;

    // Baselib_Process
    using C99Api::Baselib_Process_Abort;

    // Baselib_SystemSemaphore
    using DarwinApi::Baselib_SystemSemaphore_Create;
    using DarwinApi::Baselib_SystemSemaphore_Acquire;
    using DarwinApi::Baselib_SystemSemaphore_TryAcquire;
    using DarwinApi::Baselib_SystemSemaphore_TryTimedAcquire;
    using DarwinApi::Baselib_SystemSemaphore_Release;
    using DarwinApi::Baselib_SystemSemaphore_Free;

    // Baselib_Memory
    using PosixApi::Baselib_Memory_GetPageSizeInfo;
    using C99Api::Baselib_Memory_Allocate;
    using C99Api::Baselib_Memory_Reallocate;
    using C99Api::Baselib_Memory_Free;
    using C99Api::Baselib_Memory_AlignedAllocate;
    using C99Api::Baselib_Memory_AlignedReallocate;
    using C99Api::Baselib_Memory_AlignedFree;
    using PosixApi::Baselib_Memory_AllocatePages;
    using PosixApi::Baselib_Memory_ReleasePages;
    using DarwinApi::Baselib_Memory_SetPageState;

    // Baselib_RegisteredNetwork
    using Common::Baselib_RegisteredNetwork_Socket_UDP_Impl;
    using Common::Baselib_RegisteredNetwork_Buffer_Register;
    using Common::Baselib_RegisteredNetwork_Buffer_Deregister;
    using Common::Baselib_RegisteredNetwork_Endpoint_Create;
    using Common::Baselib_RegisteredNetwork_Endpoint_GetNetworkAddress;
    using Common::Baselib_RegisteredNetwork_Socket_UDP_Create;
    using Common::Baselib_RegisteredNetwork_Socket_UDP_ScheduleRecv;
    using Common::Baselib_RegisteredNetwork_Socket_UDP_ScheduleSend;
    using Common::Baselib_RegisteredNetwork_Socket_UDP_ProcessSend;
    using Common::Baselib_RegisteredNetwork_Socket_UDP_ProcessRecv;
    using Common::Baselib_RegisteredNetwork_Socket_UDP_WaitForCompletedSend;
    using Common::Baselib_RegisteredNetwork_Socket_UDP_WaitForCompletedRecv;
    using Common::Baselib_RegisteredNetwork_Socket_UDP_DequeueRecv;
    using Common::Baselib_RegisteredNetwork_Socket_UDP_DequeueSend;
    using Common::Baselib_RegisteredNetwork_Socket_UDP_GetNetworkAddress;
    using Common::Baselib_RegisteredNetwork_Socket_UDP_Close;

    // Baselib_TLS
    using PosixApi::Baselib_TLS_Alloc;
    using PosixApi::Baselib_TLS_Free;

    // Baselib_Timer
    using DarwinApi::Baselib_Timer_GetTicksToNanosecondsConversionRatio;
    using DarwinApi::Baselib_Timer_GetHighPrecisionTimerTicks;
    using Cpp11Api::Baselib_Timer_WaitForAtLeast;

    // Baselib_Socket
    using PosixApi::Baselib_Socket_Create;
    using PosixApi::Baselib_Socket_Bind;
    using PosixApi::Baselib_Socket_TCP_Connect;
    using PosixApi::Baselib_Socket_Poll;
    using PosixApi::Baselib_Socket_GetAddress;
    using PosixApi::Baselib_Socket_TCP_Listen;
    using PosixApi::Baselib_Socket_TCP_Accept;
    using PosixApi::Baselib_Socket_UDP_Send;
    using PosixApi::Baselib_Socket_TCP_Send;
    using PosixApi::Baselib_Socket_UDP_Recv;
    using PosixApi::Baselib_Socket_TCP_Recv;
    using PosixApi::Baselib_Socket_Close;

    // Baselib_SystemFutex
#if PLATFORM_FUTEX_NATIVE_SUPPORT
    using DarwinApi::Baselib_SystemFutex_Wait;
    using DarwinApi::Baselib_SystemFutex_Notify;
#else
    using Cpp11Api::Baselib_SystemFutex_Wait;
    using Cpp11Api::Baselib_SystemFutex_Notify;
#endif

    // Baselib_Thread
    constexpr auto Baselib_Thread_Create = PosixApi::Baselib_Thread_Create<DarwinApi::Thread_SetNameForCurrentThread>;
    using PosixApi::Baselib_Thread_Join;
    using PosixApi::Baselib_Thread_YieldExecution;
    using PosixApi::Baselib_Thread_GetCurrentThreadId;
    BASELIB_INLINE_IMPL bool Baselib_Thread_SupportsThreads() { return true; }

    // Baselib_FileIO
    using PosixApi::Baselib_FileIO_EventQueue_Create;
    using PosixApi::Baselib_FileIO_EventQueue_Free;
    using PosixApi::Baselib_FileIO_EventQueue_Dequeue;
    using PosixApi::Baselib_FileIO_File_Open;
    using PosixApi::Baselib_FileIO_File_Read;
    using PosixApi::Baselib_FileIO_File_Close;

    // Baselib_DynamicLibrary
    using PosixApi::Baselib_DynamicLibrary_Open;
    using PosixApi::Baselib_DynamicLibrary_GetFunction;
    using PosixApi::Baselib_DynamicLibrary_Close;
}

#include "Source/CProxy/Baselib_Debug_CProxy.inl.h"
#include "Source/CProxy/Baselib_ErrorState_CProxy.inl.h"
#include "Source/CProxy/Baselib_SystemFutex_CProxy.inl.h"
#include "Source/CProxy/Baselib_Memory_CProxy.inl.h"
#include "Source/CProxy/Baselib_Process_CProxy.inl.h"
#include "Source/CProxy/Baselib_RegisteredNetwork_CProxy.inl.h"
#include "Source/CProxy/Baselib_Socket_CProxy.inl.h"
#include "Source/CProxy/Baselib_SystemSemaphore_CProxy.inl.h"
#include "Source/CProxy/Baselib_ThreadLocalStorage_CProxy.inl.h"
#include "Source/CProxy/Baselib_Thread_CProxy.inl.h"
#include "Source/CProxy/Baselib_Timer_CProxy.inl.h"
#include "Source/CProxy/Baselib_FileIO_CProxy.inl.h"
#include "Source/CProxy/Baselib_DynamicLibrary_CProxy.inl.h"
