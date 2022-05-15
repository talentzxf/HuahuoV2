#pragma once

#include "Include/Baselib.h"

#include "Include/C/Baselib_RegisteredNetwork.h"
#include "Source/Baselib_ErrorState_Utils.h"
#include "Source/ArgumentValidator.h"

static inline bool IsValid(const Baselib_RegisteredNetwork_BufferSlice& slice)
{
    return slice.id != Baselib_RegisteredNetwork_Buffer_Id_Invalid;
}

static inline bool IsValid(const Baselib_RegisteredNetwork_Endpoint& endpoint)
{
    return endpoint.slice.id != Baselib_RegisteredNetwork_Buffer_Id_Invalid && endpoint.slice.data != nullptr && endpoint.slice.size != 0;
}

static inline bool IsValid(const Baselib_Memory_PageAllocation& pageAllocation)
{
    return pageAllocation.ptr != nullptr;
}

static inline bool IsValid(const Baselib_RegisteredNetwork_Socket_UDP& socket)
{
    return socket.handle != Baselib_RegisteredNetwork_Socket_UDP_Invalid.handle;
}

BASELIB_C_INTERFACE
{
    struct Baselib_RegisteredNetwork_Socket_UDP_Impl : public platform::Baselib_RegisteredNetwork_Socket_UDP_Impl {};

    Baselib_RegisteredNetwork_Buffer Baselib_RegisteredNetwork_Buffer_Register(
        Baselib_Memory_PageAllocation pageAllocation,
        Baselib_ErrorState* errorState
    )
    {
        errorState |= Validate(pageAllocation);
        if (Baselib_ErrorState_ErrorRaised(errorState))
            return { nullptr, pageAllocation };

        return platform::Baselib_RegisteredNetwork_Buffer_Register(pageAllocation, errorState);
    }

    void Baselib_RegisteredNetwork_Buffer_Deregister(
        Baselib_RegisteredNetwork_Buffer  buffer
    )
    {
        return platform::Baselib_RegisteredNetwork_Buffer_Deregister(buffer);
    }

    Baselib_RegisteredNetwork_Endpoint Baselib_RegisteredNetwork_Endpoint_Create(
        const Baselib_NetworkAddress*         srcAddress,
        Baselib_RegisteredNetwork_BufferSlice dstSlice,
        Baselib_ErrorState*                   errorState
    )
    {
        errorState |= Validate(dstSlice);
        if (dstSlice.size < Baselib_RegisteredNetwork_Endpoint_MaxSize)
            errorState |= RaiseError(Baselib_ErrorCode_InvalidBufferSize); // TODO argument name?
        if (Baselib_ErrorState_ErrorRaised(errorState))
            return Baselib_RegisteredNetwork_Endpoint_Empty();

        return platform::Baselib_RegisteredNetwork_Endpoint_Create(srcAddress, dstSlice, errorState);
    }

    void Baselib_RegisteredNetwork_Endpoint_GetNetworkAddress(
        Baselib_RegisteredNetwork_Endpoint endpoint,
        Baselib_NetworkAddress*            dstAddress,
        Baselib_ErrorState*                errorState
    )
    {
        errorState |= Validate(endpoint);
        errorState |= Validate(AsPointer(dstAddress));
        if (Baselib_ErrorState_ErrorRaised(errorState))
            return;

        return platform::Baselib_RegisteredNetwork_Endpoint_GetNetworkAddress(endpoint, dstAddress, errorState);
    }

    Baselib_RegisteredNetwork_Socket_UDP Baselib_RegisteredNetwork_Socket_UDP_Create(
        const Baselib_NetworkAddress*       bindAddress,
        Baselib_NetworkAddress_AddressReuse endpointReuse,
        uint32_t                            sendQueueSize,
        uint32_t                            recvQueueSize,
        Baselib_ErrorState*                 errorState
    )
    {
        errorState |= Validate(AsPointer(bindAddress));
        if (sendQueueSize == 0 && recvQueueSize == 0)
            errorState |= RaiseInvalidArgument(recvQueueSize);
        if (Baselib_ErrorState_ErrorRaised(errorState))
            return Baselib_RegisteredNetwork_Socket_UDP_Invalid;

        platform::Baselib_RegisteredNetwork_Socket_UDP_Impl* socket = platform::Baselib_RegisteredNetwork_Socket_UDP_Create(
            bindAddress,
            endpointReuse,
            sendQueueSize,
            recvQueueSize,
            errorState
        );
        return Baselib_RegisteredNetwork_Socket_UDP { static_cast<Baselib_RegisteredNetwork_Socket_UDP_Impl*>(socket) };
    }

    uint32_t Baselib_RegisteredNetwork_Socket_UDP_ScheduleSend(
        Baselib_RegisteredNetwork_Socket_UDP     socket,
        const Baselib_RegisteredNetwork_Request* requests,
        uint32_t                                 requestsCount,
        Baselib_ErrorState*                      errorState
    )
    {
        errorState |= Validate(socket);
        if (Baselib_ErrorState_ErrorRaised(errorState))
            return 0;
        if (requests == nullptr || requestsCount == 0)
            return 0;

        return platform::Baselib_RegisteredNetwork_Socket_UDP_ScheduleSend(socket.handle, requests, requestsCount, errorState);
    }

    uint32_t Baselib_RegisteredNetwork_Socket_UDP_ScheduleRecv(
        Baselib_RegisteredNetwork_Socket_UDP     socket,
        const Baselib_RegisteredNetwork_Request* requests,
        uint32_t                                 requestsCount,
        Baselib_ErrorState*                      errorState
    )
    {
        errorState |= Validate(socket);
        if (Baselib_ErrorState_ErrorRaised(errorState))
            return 0;
        if (requests == nullptr || requestsCount == 0)
            return 0;

        return platform::Baselib_RegisteredNetwork_Socket_UDP_ScheduleRecv(socket.handle, requests, requestsCount, errorState);
    }

    Baselib_RegisteredNetwork_ProcessStatus Baselib_RegisteredNetwork_Socket_UDP_ProcessSend(
        Baselib_RegisteredNetwork_Socket_UDP socket,
        Baselib_ErrorState*                  errorState
    )
    {
        errorState |= Validate(socket);
        if (Baselib_ErrorState_ErrorRaised(errorState))
            return Baselib_RegisteredNetwork_ProcessStatus_NonePendingImmediately;

        return platform::Baselib_RegisteredNetwork_Socket_UDP_ProcessSend(socket.handle, errorState);
    }

    Baselib_RegisteredNetwork_ProcessStatus Baselib_RegisteredNetwork_Socket_UDP_ProcessRecv(
        Baselib_RegisteredNetwork_Socket_UDP socket,
        Baselib_ErrorState*                  errorState
    )
    {
        errorState |= Validate(socket);
        if (Baselib_ErrorState_ErrorRaised(errorState))
            return Baselib_RegisteredNetwork_ProcessStatus_NonePendingImmediately;

        return platform::Baselib_RegisteredNetwork_Socket_UDP_ProcessRecv(socket.handle, errorState);
    }

    Baselib_RegisteredNetwork_CompletionQueueStatus Baselib_RegisteredNetwork_Socket_UDP_WaitForCompletedSend(
        Baselib_RegisteredNetwork_Socket_UDP socket,
        uint32_t                             timeoutInMilliseconds,
        Baselib_ErrorState*                  errorState
    )
    {
        errorState |= Validate(socket);
        if (Baselib_ErrorState_ErrorRaised(errorState))
            return Baselib_RegisteredNetwork_CompletionQueueStatus_NoResultsAvailable;

        return platform::Baselib_RegisteredNetwork_Socket_UDP_WaitForCompletedSend(
            socket.handle,
            timeoutInMilliseconds,
            errorState
        );
    }

    Baselib_RegisteredNetwork_CompletionQueueStatus Baselib_RegisteredNetwork_Socket_UDP_WaitForCompletedRecv(
        Baselib_RegisteredNetwork_Socket_UDP socket,
        uint32_t                             timeoutInMilliseconds,
        Baselib_ErrorState*                  errorState
    )
    {
        errorState |= Validate(socket);
        if (Baselib_ErrorState_ErrorRaised(errorState))
            return Baselib_RegisteredNetwork_CompletionQueueStatus_NoResultsAvailable;

        return platform::Baselib_RegisteredNetwork_Socket_UDP_WaitForCompletedRecv(
            socket.handle,
            timeoutInMilliseconds,
            errorState
        );
    }

    uint32_t Baselib_RegisteredNetwork_Socket_UDP_DequeueSend(
        Baselib_RegisteredNetwork_Socket_UDP        socket,
        Baselib_RegisteredNetwork_CompletionResult* results,
        uint32_t                                    resultsCount,
        Baselib_ErrorState*                         errorState
    )
    {
        errorState |= Validate(socket);
        if (Baselib_ErrorState_ErrorRaised(errorState))
            return false;
        if (results == nullptr || resultsCount == 0)
            return false;

        return platform::Baselib_RegisteredNetwork_Socket_UDP_DequeueSend(
            socket.handle,
            results,
            resultsCount,
            errorState
        );
    }

    uint32_t Baselib_RegisteredNetwork_Socket_UDP_DequeueRecv(
        Baselib_RegisteredNetwork_Socket_UDP        socket,
        Baselib_RegisteredNetwork_CompletionResult* results,
        uint32_t                                    resultsCount,
        Baselib_ErrorState*                         errorState
    )
    {
        errorState |= Validate(socket);
        if (Baselib_ErrorState_ErrorRaised(errorState))
            return false;
        if (results == nullptr || resultsCount == 0)
            return false;

        return platform::Baselib_RegisteredNetwork_Socket_UDP_DequeueRecv(
            socket.handle,
            results,
            resultsCount,
            errorState
        );
    }

    void Baselib_RegisteredNetwork_Socket_UDP_GetNetworkAddress(
        Baselib_RegisteredNetwork_Socket_UDP socket,
        Baselib_NetworkAddress*              dstAddress,
        Baselib_ErrorState*                  errorState
    )
    {
        errorState |= Validate(socket);
        errorState |= Validate(AsPointer(dstAddress));
        if (Baselib_ErrorState_ErrorRaised(errorState))
            return;

        platform::Baselib_RegisteredNetwork_Socket_UDP_GetNetworkAddress(socket.handle, dstAddress, errorState);
    }

    void Baselib_RegisteredNetwork_Socket_UDP_Close(
        Baselib_RegisteredNetwork_Socket_UDP socket
    )
    {
        if (socket.handle == Baselib_RegisteredNetwork_Socket_UDP_Invalid.handle)
            return;

        platform::Baselib_RegisteredNetwork_Socket_UDP_Close(socket.handle);
    }
}
