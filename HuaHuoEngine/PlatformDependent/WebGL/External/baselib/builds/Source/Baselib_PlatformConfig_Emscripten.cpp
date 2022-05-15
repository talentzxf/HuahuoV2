#include "Include/Baselib.h"
#include "Source/Posix/Baselib_FileIO_PosixApi.inl.h"
#include "Source/Posix/Baselib_Memory_PosixApi.inl.h"
#include "Source/Posix/Baselib_Timer_PosixApi.inl.h"
#include "Source/Posix/Baselib_DynamicLibrary_PosixApi.inl.h"

#include "Source/C11/Baselib_Memory_C11Api.inl.h"
#include "Source/C99/Baselib_Memory_C99Api.inl.h"
#include "Source/C99/Baselib_Process_C99Api.inl.h"

#include "Source/Common/Baselib_RegisteredNetwork_Common.inl.h"

#include "Baselib_Memory_Emscripten.inl.h"

#ifndef __EMSCRIPTEN_PTHREADS__
#include "Source/NoThreads/Baselib_SystemFutex_NoThreads.inl.h"
#include "Source/NoThreads/Baselib_SystemSemaphore_NoThreads.inl.h"
#include "Source/NoThreads/Baselib_Thread_NoThreads.inl.h"
#include "Source/NoThreads/Baselib_ThreadLocalStorage_NoThreads.inl.h"

namespace Emscripten
{
    using namespace NoThreads;
}
#else
#include "Baselib_SystemFutex_Emscripten.inl.h"
#include "Baselib_SystemSemaphore_Emscripten.inl.h"
#include "Baselib_Thread_Emscripten.inl.h"
#include "Baselib_ThreadLocalStorage_Emscripten.inl.h"

namespace Emscripten
{
    using namespace Emscripten_WithPThreads;
}
#endif

namespace platform
{
    // Baselib_Debug
    // Custom implementation, see Baselib_Debug_Emscripten.cpp

    // Baselib_Process
    using C99Api::Baselib_Process_Abort;

    // Baselib_SystemSemaphore
    using Emscripten::Baselib_SystemSemaphore_Create;
    using Emscripten::Baselib_SystemSemaphore_Acquire;
    using Emscripten::Baselib_SystemSemaphore_TryAcquire;
    using Emscripten::Baselib_SystemSemaphore_TryTimedAcquire;
    using Emscripten::Baselib_SystemSemaphore_Release;
    using Emscripten::Baselib_SystemSemaphore_Free;

    // Baselib_Memory
    using PosixApi::Baselib_Memory_GetPageSizeInfo;
    using C99Api::Baselib_Memory_Allocate;
    using C99Api::Baselib_Memory_Reallocate;
    using C99Api::Baselib_Memory_Free;
    using C11Api::Baselib_Memory_AlignedAllocate;
    using C11Api::Baselib_Memory_AlignedReallocate;
    using C11Api::Baselib_Memory_AlignedFree;
    using Emscripten::Baselib_Memory_AllocatePages;
    using Emscripten::Baselib_Memory_SetPageState;
    using PosixApi::Baselib_Memory_ReleasePages;

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

    // Baselib_Socket
    // Custom implementation, see Baselib_Socket_Emscripten.cpp

    // Baselib_SystemFutex
    using Emscripten::Baselib_SystemFutex_Wait;
    using Emscripten::Baselib_SystemFutex_Notify;

    // Baselib_TLS
    using Emscripten::Baselib_TLS_Alloc;
    using Emscripten::Baselib_TLS_Free;

    // Baselib_Timer
    // Custom implementation, see Baselib_Timer_Emscripten.cpp

    // Baselib_Thread
    using Emscripten::Baselib_Thread_Create;
    using Emscripten::Baselib_Thread_Join;
    using Emscripten::Baselib_Thread_YieldExecution;
    using Emscripten::Baselib_Thread_GetCurrentThreadId;
    using Emscripten::Baselib_Thread_SupportsThreads;

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

#include "Source/CProxy/Baselib_FileIO_CProxy.inl.h"
#include "Source/CProxy/Baselib_Memory_CProxy.inl.h"
#include "Source/CProxy/Baselib_Process_CProxy.inl.h"
#include "Source/CProxy/Baselib_RegisteredNetwork_CProxy.inl.h"
#include "Source/CProxy/Baselib_SystemFutex_CProxy.inl.h"
#include "Source/CProxy/Baselib_SystemSemaphore_CProxy.inl.h"
#include "Source/CProxy/Baselib_Thread_CProxy.inl.h"
#include "Source/CProxy/Baselib_ThreadLocalStorage_CProxy.inl.h"
#include "Source/CProxy/Baselib_DynamicLibrary_CProxy.inl.h"
