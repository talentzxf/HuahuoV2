#pragma once

#include "Include/Baselib.h"

#include "Source/Common/Baselib_RegisteredNetwork_Common.inl.h"
#include "Source/WinApi/Baselib_RegisteredNetwork_WinApi.inl.h"

namespace WinApi_WithFallback
{
    struct Baselib_RegisteredNetwork_Socket_UDP_Impl
    {
        Baselib_RegisteredNetwork_Socket_UDP_Impl() = delete;
    };

    static inline WinApi::Baselib_RegisteredNetwork_Socket_UDP_Impl* GetWinApiSocketImpl(Baselib_RegisteredNetwork_Socket_UDP_Impl* socket)
    { return reinterpret_cast<WinApi::Baselib_RegisteredNetwork_Socket_UDP_Impl*>(socket); }
    static inline Common::Baselib_RegisteredNetwork_Socket_UDP_Impl* GetCommonSocketImpl(Baselib_RegisteredNetwork_Socket_UDP_Impl* socket)
    { return reinterpret_cast<Common::Baselib_RegisteredNetwork_Socket_UDP_Impl*>(socket); }

    BASELIB_INLINE_IMPL Baselib_RegisteredNetwork_Buffer Baselib_RegisteredNetwork_Buffer_Register(
        Baselib_Memory_PageAllocation pageAllocation,
        Baselib_ErrorState* errorState
    )
    {
        if (WinApi::RIOSockets::IsRioAvailable())
            return WinApi::Baselib_RegisteredNetwork_Buffer_Register(pageAllocation, errorState);
        return Common::Baselib_RegisteredNetwork_Buffer_Register(pageAllocation, errorState);
    }

    BASELIB_INLINE_IMPL void Baselib_RegisteredNetwork_Buffer_Deregister(
        Baselib_RegisteredNetwork_Buffer buffer
    )
    {
        if (WinApi::RIOSockets::IsRioAvailable())
            return WinApi::Baselib_RegisteredNetwork_Buffer_Deregister(buffer);
        return Common::Baselib_RegisteredNetwork_Buffer_Deregister(buffer);
    }

    BASELIB_INLINE_IMPL Baselib_RegisteredNetwork_Endpoint Baselib_RegisteredNetwork_Endpoint_Create(
        const Baselib_NetworkAddress*         srcAddress,
        Baselib_RegisteredNetwork_BufferSlice dstSlice,
        Baselib_ErrorState*                   errorState
    )
    {
        if (WinApi::RIOSockets::IsRioAvailable())
            return WinApi::Baselib_RegisteredNetwork_Endpoint_Create(srcAddress, dstSlice, errorState);
        return Common::Baselib_RegisteredNetwork_Endpoint_Create(srcAddress, dstSlice, errorState);
    }

    BASELIB_INLINE_IMPL void Baselib_RegisteredNetwork_Endpoint_GetNetworkAddress(
        Baselib_RegisteredNetwork_Endpoint endpoint,
        Baselib_NetworkAddress*            dstAddress,
        Baselib_ErrorState*                errorState
    )
    {
        if (WinApi::RIOSockets::IsRioAvailable())
            return WinApi::Baselib_RegisteredNetwork_Endpoint_GetNetworkAddress(endpoint, dstAddress, errorState);
        return Common::Baselib_RegisteredNetwork_Endpoint_GetNetworkAddress(endpoint, dstAddress, errorState);
    }

    BASELIB_INLINE_IMPL WinApi_WithFallback::Baselib_RegisteredNetwork_Socket_UDP_Impl* Baselib_RegisteredNetwork_Socket_UDP_Create(
        const Baselib_NetworkAddress*       bindAddress,
        Baselib_NetworkAddress_AddressReuse endpointReuse,
        uint32_t                            sendQueueSize,
        uint32_t                            recvQueueSize,
        Baselib_ErrorState*                 errorState
    )
    {
        if (WinApi::RIOSockets::IsRioAvailable())
            return (WinApi_WithFallback::Baselib_RegisteredNetwork_Socket_UDP_Impl*)WinApi::Baselib_RegisteredNetwork_Socket_UDP_Create(
                bindAddress, endpointReuse, sendQueueSize, recvQueueSize, errorState);
        return (WinApi_WithFallback::Baselib_RegisteredNetwork_Socket_UDP_Impl*)Common::Baselib_RegisteredNetwork_Socket_UDP_Create(
            bindAddress, endpointReuse, sendQueueSize, recvQueueSize, errorState);
    }

    BASELIB_INLINE_IMPL uint32_t Baselib_RegisteredNetwork_Socket_UDP_ScheduleSend(
        WinApi_WithFallback::Baselib_RegisteredNetwork_Socket_UDP_Impl* socket,
        const Baselib_RegisteredNetwork_Request*                        requests,
        uint32_t                                                        requestsCount,
        Baselib_ErrorState*                                             errorState
    )
    {
        if (WinApi::RIOSockets::IsRioAvailable())
            return WinApi::Baselib_RegisteredNetwork_Socket_UDP_ScheduleSend(GetWinApiSocketImpl(socket), requests, requestsCount, errorState);
        return Common::Baselib_RegisteredNetwork_Socket_UDP_ScheduleSend(GetCommonSocketImpl(socket), requests, requestsCount, errorState);
    }

    BASELIB_INLINE_IMPL uint32_t Baselib_RegisteredNetwork_Socket_UDP_ScheduleRecv(
        WinApi_WithFallback::Baselib_RegisteredNetwork_Socket_UDP_Impl* socket,
        const Baselib_RegisteredNetwork_Request*                        requests,
        uint32_t                                                        requestsCount,
        Baselib_ErrorState*                                             errorState
    )
    {
        if (WinApi::RIOSockets::IsRioAvailable())
            return WinApi::Baselib_RegisteredNetwork_Socket_UDP_ScheduleRecv(GetWinApiSocketImpl(socket), requests, requestsCount, errorState);
        return Common::Baselib_RegisteredNetwork_Socket_UDP_ScheduleRecv(GetCommonSocketImpl(socket), requests, requestsCount, errorState);
    }

    BASELIB_INLINE_IMPL Baselib_RegisteredNetwork_ProcessStatus Baselib_RegisteredNetwork_Socket_UDP_ProcessSend(
        WinApi_WithFallback::Baselib_RegisteredNetwork_Socket_UDP_Impl* socket,
        Baselib_ErrorState*                                errorState
    )
    {
        if (WinApi::RIOSockets::IsRioAvailable())
            return WinApi::Baselib_RegisteredNetwork_Socket_UDP_ProcessSend(GetWinApiSocketImpl(socket), errorState);
        return Common::Baselib_RegisteredNetwork_Socket_UDP_ProcessSend(GetCommonSocketImpl(socket), errorState);
    }

    BASELIB_INLINE_IMPL Baselib_RegisteredNetwork_ProcessStatus Baselib_RegisteredNetwork_Socket_UDP_ProcessRecv(
        WinApi_WithFallback::Baselib_RegisteredNetwork_Socket_UDP_Impl* socket,
        Baselib_ErrorState*                                errorState
    )
    {
        if (WinApi::RIOSockets::IsRioAvailable())
            return WinApi::Baselib_RegisteredNetwork_Socket_UDP_ProcessRecv(GetWinApiSocketImpl(socket), errorState);
        return Common::Baselib_RegisteredNetwork_Socket_UDP_ProcessRecv(GetCommonSocketImpl(socket), errorState);
    }

    BASELIB_INLINE_IMPL Baselib_RegisteredNetwork_CompletionQueueStatus Baselib_RegisteredNetwork_Socket_UDP_WaitForCompletedSend(
        WinApi_WithFallback::Baselib_RegisteredNetwork_Socket_UDP_Impl* socket,
        uint32_t                                           timeoutInMilliseconds,
        Baselib_ErrorState*                                errorState
    )
    {
        if (WinApi::RIOSockets::IsRioAvailable())
            return WinApi::Baselib_RegisteredNetwork_Socket_UDP_WaitForCompletedSend(GetWinApiSocketImpl(socket), timeoutInMilliseconds, errorState);
        return Common::Baselib_RegisteredNetwork_Socket_UDP_WaitForCompletedSend(GetCommonSocketImpl(socket), timeoutInMilliseconds, errorState);
    }

    BASELIB_INLINE_IMPL Baselib_RegisteredNetwork_CompletionQueueStatus Baselib_RegisteredNetwork_Socket_UDP_WaitForCompletedRecv(
        WinApi_WithFallback::Baselib_RegisteredNetwork_Socket_UDP_Impl* socket,
        uint32_t                                           timeoutInMilliseconds,
        Baselib_ErrorState*                                errorState
    )
    {
        if (WinApi::RIOSockets::IsRioAvailable())
            return WinApi::Baselib_RegisteredNetwork_Socket_UDP_WaitForCompletedRecv(GetWinApiSocketImpl(socket), timeoutInMilliseconds, errorState);
        return Common::Baselib_RegisteredNetwork_Socket_UDP_WaitForCompletedRecv(GetCommonSocketImpl(socket), timeoutInMilliseconds, errorState);
    }

    BASELIB_INLINE_IMPL uint32_t Baselib_RegisteredNetwork_Socket_UDP_DequeueSend(
        WinApi_WithFallback::Baselib_RegisteredNetwork_Socket_UDP_Impl*        socket,
        Baselib_RegisteredNetwork_CompletionResult*               results,
        uint32_t                                                  resultsCount,
        Baselib_ErrorState*                                       errorState
    )
    {
        if (WinApi::RIOSockets::IsRioAvailable())
            return WinApi::Baselib_RegisteredNetwork_Socket_UDP_DequeueSend(GetWinApiSocketImpl(socket), results, resultsCount, errorState);
        return Common::Baselib_RegisteredNetwork_Socket_UDP_DequeueSend(GetCommonSocketImpl(socket), results, resultsCount, errorState);
    }

    BASELIB_INLINE_IMPL uint32_t Baselib_RegisteredNetwork_Socket_UDP_DequeueRecv(
        WinApi_WithFallback::Baselib_RegisteredNetwork_Socket_UDP_Impl*        socket,
        Baselib_RegisteredNetwork_CompletionResult*               results,
        uint32_t                                                  resultsCount,
        Baselib_ErrorState*                                       errorState
    )
    {
        if (WinApi::RIOSockets::IsRioAvailable())
            return WinApi::Baselib_RegisteredNetwork_Socket_UDP_DequeueRecv(GetWinApiSocketImpl(socket), results, resultsCount, errorState);
        return Common::Baselib_RegisteredNetwork_Socket_UDP_DequeueRecv(GetCommonSocketImpl(socket), results, resultsCount, errorState);
    }

    BASELIB_INLINE_IMPL void Baselib_RegisteredNetwork_Socket_UDP_GetNetworkAddress(
        WinApi_WithFallback::Baselib_RegisteredNetwork_Socket_UDP_Impl* socket,
        Baselib_NetworkAddress*                            dstAddress,
        Baselib_ErrorState*                                errorState
    )
    {
        if (WinApi::RIOSockets::IsRioAvailable())
            return WinApi::Baselib_RegisteredNetwork_Socket_UDP_GetNetworkAddress(GetWinApiSocketImpl(socket), dstAddress, errorState);
        return Common::Baselib_RegisteredNetwork_Socket_UDP_GetNetworkAddress(GetCommonSocketImpl(socket), dstAddress, errorState);
    }

    BASELIB_INLINE_IMPL void Baselib_RegisteredNetwork_Socket_UDP_Close(
        WinApi_WithFallback::Baselib_RegisteredNetwork_Socket_UDP_Impl* socket
    )
    {
        if (WinApi::RIOSockets::IsRioAvailable())
            return WinApi::Baselib_RegisteredNetwork_Socket_UDP_Close(GetWinApiSocketImpl(socket));
        return Common::Baselib_RegisteredNetwork_Socket_UDP_Close(GetCommonSocketImpl(socket));
    }
}
