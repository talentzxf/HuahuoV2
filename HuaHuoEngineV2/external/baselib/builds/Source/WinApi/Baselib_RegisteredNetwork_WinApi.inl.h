#pragma once

#include "Include/Baselib.h"

#include "Include/C/Baselib_RegisteredNetwork.h"

#include "Include/Cpp/CountdownTimer.h"
#include "Include/Cpp/Lock.h"
#include "Include/Cpp/Atomic.h"

#include "Source/STLMemoryUtils.h"
#include "Source/WinApi/Baselib_Socket_WinApi.inl.h"

#include <winsock2.h>
#include <Mswsock.h>

namespace WinApi
{
    class RIOSockets
    {
    public:
        static bool IsRioAvailable()
        {
            return Get().isRioAvailable;
        }

        static const RIO_EXTENSION_FUNCTION_TABLE& GetFunctionTable()
        {
            BaselibAssert(Get().isRioAvailable);
            return Get().rioFunctionTable;
        }

    private:

        static RIOSockets& Get()
        {
            // According to C++ standard only one thread will ever execute the init of this global variable.
            static RIOSockets rioSockets;
            return rioSockets;
        }

        RIO_EXTENSION_FUNCTION_TABLE rioFunctionTable;
        bool isRioAvailable = false;

        RIOSockets()
        {
            // We assume the application did WSA startup already
            SOCKET ioctlSocket = ::WSASocketW(AF_INET6, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, WSA_FLAG_REGISTERED_IO);
            if (ioctlSocket == INVALID_SOCKET)
                return;

            DWORD nBytes = 0;
            GUID functionTableId = WSAID_MULTIPLE_RIO;
            const int rioFunctionTableLookup = ::WSAIoctl(ioctlSocket, SIO_GET_MULTIPLE_EXTENSION_FUNCTION_POINTER, &functionTableId, sizeof(GUID),
                (void**)(&rioFunctionTable), sizeof(rioFunctionTable), &nBytes, NULL, NULL);

            const int ioctlSocketClose = ::closesocket(ioctlSocket);
            BaselibAssert(ioctlSocketClose == ERROR_SUCCESS);

            isRioAvailable = (rioFunctionTableLookup == ERROR_SUCCESS);
        }
    };

    struct Baselib_RegisteredNetwork_Socket_UDP_Impl
    {
        struct CompletionQueue
        {
            bool Init(uint32_t queueSize, Baselib_ErrorState* errorState)
            {
                if (queueSize == 0)
                    return true;

                completionEvent = WSACreateEvent();
                if (completionEvent == WSA_INVALID_EVENT)
                {
                    Baselib_ErrorCode baselibErrorCode = Baselib_ErrorCode_UnexpectedError;
                    int lastWSAError = WSAGetLastError();
                    if (lastWSAError == WSA_NOT_ENOUGH_MEMORY)
                        baselibErrorCode = Baselib_ErrorCode_OutOfSystemResources;
                    errorState |= RaiseError(baselibErrorCode) | WithGetLastError(lastWSAError);
                    return false;
                }

                RIO_NOTIFICATION_COMPLETION completion;
                completion.Type = RIO_EVENT_COMPLETION;
                completion.Event.EventHandle = completionEvent;
                completion.Event.NotifyReset = TRUE;

                cq = RIOSockets::GetFunctionTable().RIOCreateCompletionQueue((DWORD)queueSize, &completion);
                if (cq == RIO_INVALID_CQ)
                {
                    Baselib_ErrorCode baselibErrorCode = Baselib_ErrorCode_UnexpectedError;
                    int lastWSAError = WSAGetLastError();
                    if (lastWSAError == WSAENOBUFS)
                        baselibErrorCode = Baselib_ErrorCode_OutOfSystemResources;
                    errorState |= RaiseError(baselibErrorCode) | WithGetLastError(lastWSAError);
                    return false;
                }

                return true;
            }

            ~CompletionQueue()
            {
                if (completionEvent != (WSAEVENT)WSA_INVALID_HANDLE)
                {
                    // WSACloseEvent does NOT work with WSA_INVALID_HANDLE!
                    BOOL closeResult = WSACloseEvent(completionEvent);
                    BaselibAssert(closeResult == TRUE, "Failed to close event object, WSAGetLastError: %i", WSAGetLastError());
                }
                RIOSockets::GetFunctionTable().RIOCloseCompletionQueue(cq);
            }

            bool WaitForResult(uint32_t timeoutInMilliseconds, Baselib_ErrorState* errorState)
            {
                if (cq == RIO_INVALID_CQ)
                    return false;

                int errorCode = RIOSockets::GetFunctionTable().RIONotify(cq);
                if (errorCode != ERROR_SUCCESS)
                {
                    errorState |= RaiseError(Baselib_ErrorCode_UnexpectedError) | WithGetLastError();
                    return false;
                }
                const baselib::CountdownTimer timeoutTimer = baselib::CountdownTimer::StartNew(baselib::timeout_ms(timeoutInMilliseconds));
                do
                {
                    const DWORD result = WSAWaitForMultipleEvents(1, &completionEvent, TRUE, timeoutInMilliseconds, FALSE);
                    if (result == WSA_WAIT_EVENT_0)
                        return true;
                    // According to spec the only other thing we can get is WSA_WAIT_IO_COMPLETION, but only if fAlertable==TRUE
                    BaselibAssert(result == WSA_WAIT_TIMEOUT, "Unexpected result on call to WSAWaitForMultipleEvents, WSAGetLastError: %i", WSAGetLastError());

                    timeoutInMilliseconds = timeoutTimer.GetTimeLeftInMilliseconds().count();
                }
                while (timeoutInMilliseconds);

                return false;
            }

            uint32_t Dequeue(Baselib_RegisteredNetwork_CompletionResult* results, uint32_t resultsCount, baselib::atomic<uint32_t>& freeSlotsCounter)
            {
                if (cq == RIO_INVALID_CQ) return 0;

                // Dequeueing on completion queue is not threadsafe, do a "weak lock" in accordance to our spec.
                uint32_t completedResults = 0;
                cqLock.TryAcquireScoped([this, results, resultsCount, &completedResults, &freeSlotsCounter]
                {
                    RIORESULT rioResults[16];

                    while (completedResults < resultsCount)
                    {
                        uint32_t availableResults = resultsCount - completedResults;
                        uint32_t resultsToFill = availableResults < 16 ? availableResults : 16;
                        ULONG dequeueResult = RIOSockets::GetFunctionTable().RIODequeueCompletion(cq, rioResults, static_cast<ULONG>(resultsToFill));

                        BaselibAssert(dequeueResult != RIO_CORRUPT_CQ);
                        if (dequeueResult == RIO_CORRUPT_CQ)
                            return;
                        if (dequeueResult == 0)
                            return;
                        freeSlotsCounter.fetch_add(dequeueResult);

                        for (uint32_t i = 0; i < dequeueResult; ++i)
                        {
                            auto& palResult = results[completedResults];
                            auto& rioResult = rioResults[i];
                            // WSAEMSGSIZE happens if our buffer too large or too small for this message - regardless we count this as a success!
                            palResult.status           = (rioResult.Status == ERROR_SUCCESS || rioResult.Status == WSAEMSGSIZE) ?
                                Baselib_RegisteredNetwork_CompletionStatus_Success :
                                Baselib_RegisteredNetwork_CompletionStatus_Failed;
                            palResult.bytesTransferred = rioResult.BytesTransferred;
                            palResult.requestUserdata  = reinterpret_cast<void*>(rioResult.RequestContext);
                            completedResults++;
                        }
                    }
                });

                return completedResults;
            }

            WSAEVENT completionEvent = (WSAEVENT)WSA_INVALID_HANDLE;
            RIO_CQ cq = RIO_INVALID_CQ;
            baselib::Lock cqLock;
        };

        static Baselib_RegisteredNetwork_Socket_UDP_Impl* Create(
            const Baselib_NetworkAddress*       bindAddress,
            Baselib_NetworkAddress_AddressReuse endpointReuse,
            uint32_t                            sendQueueSize,
            uint32_t                            recvQueueSize,
            Baselib_ErrorState*                 errorState
        )
        {
            // Allocate socket. Use smart pointer to auto-free on early exit.
            auto socket = baselib::make_unique<Baselib_RegisteredNetwork_Socket_UDP_Impl>();

            // Create socket
            Baselib_NetworkAddress_Family family;
            Baselib_NetworkAddress_Decode(bindAddress, &family, nullptr, 0, nullptr, errorState);
            socket->baselibSocket = WinApi::Baselib_Socket_Create_WSASocket(family, Baselib_Socket_Protocol_UDP, WSA_FLAG_REGISTERED_IO, errorState);
            WinApi::Baselib_Socket_Bind(socket->baselibSocket, bindAddress, endpointReuse, errorState);
            if (Baselib_ErrorState_ErrorRaised(errorState))
                return nullptr;

            // Create completion queues
            if (!socket->sendCq.Init(sendQueueSize, errorState))
                return nullptr;
            if (!socket->recvCq.Init(recvQueueSize, errorState))
                return nullptr;

            // Create request queue.
            socket->rq = RIOSockets::GetFunctionTable().RIOCreateRequestQueue(
                socket->baselibSocket.handle,
                (ULONG)recvQueueSize,
                1,
                (ULONG)sendQueueSize,
                1,
                recvQueueSize ? socket->recvCq.cq : socket->sendCq.cq, // Need to specify non-null queue.
                sendQueueSize ? socket->sendCq.cq : socket->recvCq.cq, // Need to specify non-null queue.
                nullptr
            );
            if (socket->rq == RIO_INVALID_RQ)
            {
                int error = WSAGetLastError();
                errorState |= RaiseError(error == WSAENOBUFS ? Baselib_ErrorCode_OutOfSystemResources : Baselib_ErrorCode_UnexpectedError) | WithGetLastError(error);
                return nullptr;
            }

            socket->sendNumFreeSlots = sendQueueSize;
            socket->recvNumFreeSlots = recvQueueSize;
            return socket.release();
        }

        ~Baselib_RegisteredNetwork_Socket_UDP_Impl()
        {
            WinApi::Baselib_Socket_Close(baselibSocket);
        }

        uint32_t ScheduleSend(
            const Baselib_RegisteredNetwork_Request* packets,
            uint32_t requestsCount,
            Baselib_ErrorState* errorState)
        {
            // Sending is not threadsafe since it touches the send request queue!
            uint32_t numProcessedRequests = 0;
            rqLock.AcquireScoped([this, &numProcessedRequests, packets, requestsCount, errorState]()
            {
                for (; numProcessedRequests < requestsCount; ++numProcessedRequests)
                {
                    if (!AcquireQueueSlot(sendNumFreeSlots))
                        return;

                    auto& packet = packets[numProcessedRequests];

                    RIO_BUF payload = BufferSliceToRio(packet.payload);
                    RIO_BUF remoteEndpoint = BufferSliceToRio(packet.remoteEndpoint.slice);

                    BOOL result = RIOSockets::GetFunctionTable().RIOSendEx(
                        rq,
                        (packet.payload.id != Baselib_RegisteredNetwork_Buffer_Id_Invalid) ? &payload : NULL,
                        (packet.payload.id != Baselib_RegisteredNetwork_Buffer_Id_Invalid) ? 1 : 0,
                        NULL,
                        (packet.remoteEndpoint.slice.id != Baselib_RegisteredNetwork_Buffer_Id_Invalid) ? &remoteEndpoint : NULL,
                        NULL,
                        NULL,
                        RIO_MSG_DEFER,
                        packet.requestUserdata
                    );

                    // Since we protect the queue(s) with our own atomic counter, this should always succeed!
                    if (result == FALSE)
                    {
                        errorState |= RaiseError(Baselib_ErrorCode_UnexpectedError) | WithGetLastError(WSAGetLastError());
                        return;
                    }
                }
            });

            return numProcessedRequests;
        }

        uint32_t ScheduleRecv(
            const Baselib_RegisteredNetwork_Request* packets,
            uint32_t requestsCount,
            Baselib_ErrorState* errorState)
        {
            // Receiving is not threadsafe since it touches the request queue!
            uint32_t numProcessedRequests = 0;
            rqLock.AcquireScoped([this, &numProcessedRequests, packets, requestsCount, errorState]()
            {
                for (; numProcessedRequests < requestsCount; ++numProcessedRequests)
                {
                    if (!AcquireQueueSlot(recvNumFreeSlots))
                        return;

                    auto& packet = packets[numProcessedRequests];

                    RIO_BUF payload = BufferSliceToRio(packet.payload);
                    RIO_BUF remoteEndpoint = BufferSliceToRio(packet.remoteEndpoint.slice);

                    BOOL result = RIOSockets::GetFunctionTable().RIOReceiveEx(
                        rq,
                        (packet.payload.id != Baselib_RegisteredNetwork_Buffer_Id_Invalid) ? &payload : NULL,
                        (packet.payload.id != Baselib_RegisteredNetwork_Buffer_Id_Invalid) ? 1 : 0,
                        NULL,
                        (packet.remoteEndpoint.slice.id != Baselib_RegisteredNetwork_Buffer_Id_Invalid) ? &remoteEndpoint : NULL,
                        NULL,
                        NULL,
                        RIO_MSG_DEFER,
                        packet.requestUserdata
                    );

                    // Since we protect the queue(s) with our own atomic counter, this should always succeed!
                    if (result == FALSE)
                    {
                        errorState |= RaiseError(Baselib_ErrorCode_UnexpectedError) | WithGetLastError(WSAGetLastError());
                        return;
                    }
                }
            });

            return numProcessedRequests;
        }

        void ProcessSendRequest()
        {
            // No lock required here due to RIO_MSG_COMMIT_ONLY. See documentation:
            // Unlike other calls to the RIOSendEx function, when the RIO_MSG_COMMIT_ONLY flag is set calls to the RIOSendEx function do not
            // need to be serialized. For a single RIO_RQ, the RIOSendEx function can be called with RIO_MSG_COMMIT_ONLY on one thread while
            // calling the RIOSendEx function on another thread.
            BOOL result = RIOSockets::GetFunctionTable().RIOSend(
                rq,
                nullptr,
                0,
                RIO_MSG_COMMIT_ONLY,
                nullptr
            );
            BaselibAssert(result == TRUE);
        }

        void ProcessRecvRequest()
        {
            // No lock required here due to RIO_MSG_COMMIT_ONLY. See documentation:
            // Unlike other calls to the RIOSendEx function, when the RIO_MSG_COMMIT_ONLY flag is set calls to the RIOSendEx function do not
            // need to be serialized. For a single RIO_RQ, the RIOSendEx function can be called with RIO_MSG_COMMIT_ONLY on one thread while
            // calling the RIOSendEx function on another thread.
            BOOL result = RIOSockets::GetFunctionTable().RIOReceive(
                rq,
                nullptr,
                0,
                RIO_MSG_COMMIT_ONLY,
                nullptr
            );
            BaselibAssert(result == TRUE);
        }

        Baselib_RegisteredNetwork_CompletionQueueStatus WaitForCompletedSend(uint32_t timeoutInMilliseconds, Baselib_ErrorState* errorState)
        {
            return sendCq.WaitForResult(timeoutInMilliseconds, errorState) ?
                Baselib_RegisteredNetwork_CompletionQueueStatus_ResultsAvailable : Baselib_RegisteredNetwork_CompletionQueueStatus_NoResultsAvailable;
        }

        Baselib_RegisteredNetwork_CompletionQueueStatus WaitForCompletedRecv(uint32_t timeoutInMilliseconds, Baselib_ErrorState* errorState)
        {
            return recvCq.WaitForResult(timeoutInMilliseconds, errorState) ?
                Baselib_RegisteredNetwork_CompletionQueueStatus_ResultsAvailable : Baselib_RegisteredNetwork_CompletionQueueStatus_NoResultsAvailable;
        }

        uint32_t DequeueSend(Baselib_RegisteredNetwork_CompletionResult* results, uint32_t resultsCount)
        {
            return sendCq.Dequeue(results, resultsCount, sendNumFreeSlots);
        }

        uint32_t DequeueRecv(Baselib_RegisteredNetwork_CompletionResult* results, uint32_t resultsCount)
        {
            return recvCq.Dequeue(results, resultsCount, recvNumFreeSlots);
        }

        void GetNetworkAddress(Baselib_NetworkAddress* dstAddress, Baselib_ErrorState* errorState)
        {
            WinApi::Baselib_Socket_GetAddress(baselibSocket, dstAddress, errorState);
        }

    private:
        static RIO_BUF BufferSliceToRio(Baselib_RegisteredNetwork_BufferSlice slice)
        {
            RIO_BUF buf;
            buf.BufferId = static_cast<RIO_BUFFERID>(slice.id);
            buf.Offset = static_cast<ULONG>(slice.offset);
            buf.Length = static_cast<ULONG>(slice.size);
            return buf;
        }

        static bool AcquireQueueSlot(baselib::atomic<uint32_t>& numFreeSlots)
        {
            uint32_t fullyFreeSlotsCount = numFreeSlots.load(baselib::memory_order_relaxed);
            do
            {
                if (fullyFreeSlotsCount == 0)
                    return false;
            }
            while (!numFreeSlots.compare_exchange_weak(
                fullyFreeSlotsCount,
                fullyFreeSlotsCount - 1,
                baselib::memory_order_relaxed,
                baselib::memory_order_relaxed
            ));
            return true;
        }

        Baselib_Socket_Handle baselibSocket;
        RIO_RQ rq;
        baselib::Lock rqLock;
        CompletionQueue recvCq;
        baselib::atomic<uint32_t> sendNumFreeSlots;
        CompletionQueue sendCq;
        baselib::atomic<uint32_t> recvNumFreeSlots;
    };

    BASELIB_INLINE_IMPL Baselib_RegisteredNetwork_Buffer Baselib_RegisteredNetwork_Buffer_Register(
        Baselib_Memory_PageAllocation pageAllocation,
        Baselib_ErrorState* errorState
    )
    {
        const RIO_BUFFERID id = RIOSockets::GetFunctionTable().RIORegisterBuffer(static_cast<PCHAR>(pageAllocation.ptr), static_cast<DWORD>(pageAllocation.pageCount * pageAllocation.pageSize));
        if (id == RIO_INVALID_BUFFERID)
        {
            int error = WSAGetLastError();
            switch (error)
            {
                case WSAEFAULT:
                    errorState |= RaiseError(Baselib_ErrorCode_InvalidAddressRange) | WithGetLastError(error);
                    break;
                default:
                    errorState |= RaiseError(Baselib_ErrorCode_UnexpectedError) | WithGetLastError(error);
                    BaselibAssert(id != RIO_INVALID_BUFFERID);
            }
            return Baselib_RegisteredNetwork_Buffer { Baselib_RegisteredNetwork_Buffer_Id_Invalid, pageAllocation };
        }
        return Baselib_RegisteredNetwork_Buffer{ id, pageAllocation };
    }

    BASELIB_INLINE_IMPL void Baselib_RegisteredNetwork_Buffer_Deregister(
        Baselib_RegisteredNetwork_Buffer  buffer
    )
    {
        return RIOSockets::GetFunctionTable().RIODeregisterBuffer(static_cast<RIO_BUFFERID>(buffer.id));
    }

    BASELIB_INLINE_IMPL Baselib_RegisteredNetwork_Endpoint Baselib_RegisteredNetwork_Endpoint_Create(
        const Baselib_NetworkAddress*         srcAddress,
        Baselib_RegisteredNetwork_BufferSlice dstSlice,
        Baselib_ErrorState*                   errorState
    )
    {
        static_assert(sizeof(SOCKADDR_INET) <= Baselib_RegisteredNetwork_Endpoint_MaxSize,
            "Socket address size is bigger than the max size for an endpoint");

        Baselib_ErrorState addrConversionErrorState = Baselib_ErrorState_Create();
        WinApi::detail::socketaddr socketAddr(srcAddress, &addrConversionErrorState);

        static_assert(std::is_same<std::remove_reference<decltype(*socketAddr.inet())>::type, SOCKADDR_INET>::value,
            "WinApi::detail::socketaddr is not returning a pointer to SOCKADDR_INET");

        if (!Baselib_ErrorState_ErrorRaised(&addrConversionErrorState))
            memcpy(dstSlice.data, socketAddr.inet(), sizeof(SOCKADDR_INET));
        else
            memset(dstSlice.data, 0, sizeof(SOCKADDR_INET));
        dstSlice.size = sizeof(SOCKADDR_INET);

        return {dstSlice};
    }

    BASELIB_INLINE_IMPL void Baselib_RegisteredNetwork_Endpoint_GetNetworkAddress(
        Baselib_RegisteredNetwork_Endpoint endpoint,
        Baselib_NetworkAddress*            dstAddress,
        Baselib_ErrorState*                errorState
    )
    {
        WinApi::detail::socketaddr* socketAddr = reinterpret_cast<WinApi::detail::socketaddr*>(endpoint.slice.data);

        // Check if this is a valid memory block - if it's all zeroed out it wasn't written to yet!
        static const WinApi::detail::socketaddr emptyAddr;
        if (memcmp(socketAddr, emptyAddr.inet(), sizeof(SOCKADDR_INET)) == 0)
        {
            *dstAddress = Baselib_NetworkAddress_Empty();
            return;
        }

        socketAddr->toBaselib(dstAddress, errorState);
    }

    BASELIB_INLINE_IMPL WinApi::Baselib_RegisteredNetwork_Socket_UDP_Impl* Baselib_RegisteredNetwork_Socket_UDP_Create(
        const Baselib_NetworkAddress*       bindAddress,
        Baselib_NetworkAddress_AddressReuse endpointReuse,
        uint32_t                            sendQueueSize,
        uint32_t                            recvQueueSize,
        Baselib_ErrorState*                 errorState
    )
    {
        return Baselib_RegisteredNetwork_Socket_UDP_Impl::Create(
            bindAddress,
            endpointReuse,
            sendQueueSize,
            recvQueueSize,
            errorState);
    }

    BASELIB_INLINE_IMPL uint32_t Baselib_RegisteredNetwork_Socket_UDP_ScheduleRecv(
        WinApi::Baselib_RegisteredNetwork_Socket_UDP_Impl* socket,
        const Baselib_RegisteredNetwork_Request*           requests,
        uint32_t                                           requestsCount,
        Baselib_ErrorState*                                errorState
    )
    {
        return socket->ScheduleRecv(requests, requestsCount, errorState);
    }

    BASELIB_INLINE_IMPL uint32_t Baselib_RegisteredNetwork_Socket_UDP_ScheduleSend(
        WinApi::Baselib_RegisteredNetwork_Socket_UDP_Impl* socket,
        const Baselib_RegisteredNetwork_Request*           requests,
        uint32_t                                           requestsCount,
        Baselib_ErrorState*                                errorState
    )
    {
        return socket->ScheduleSend(requests, requestsCount, errorState);
    }

    BASELIB_INLINE_IMPL Baselib_RegisteredNetwork_ProcessStatus Baselib_RegisteredNetwork_Socket_UDP_ProcessSend(
        WinApi::Baselib_RegisteredNetwork_Socket_UDP_Impl* socket,
        Baselib_ErrorState*                                errorState
    )
    {
        UNUSED(errorState);
        socket->ProcessSendRequest();
        return Baselib_RegisteredNetwork_ProcessStatus_NonePendingImmediately;
    }

    BASELIB_INLINE_IMPL Baselib_RegisteredNetwork_ProcessStatus Baselib_RegisteredNetwork_Socket_UDP_ProcessRecv(
        WinApi::Baselib_RegisteredNetwork_Socket_UDP_Impl* socket,
        Baselib_ErrorState*                                errorState
    )
    {
        UNUSED(errorState);
        socket->ProcessRecvRequest();
        return Baselib_RegisteredNetwork_ProcessStatus_NonePendingImmediately;
    }

    BASELIB_INLINE_IMPL Baselib_RegisteredNetwork_CompletionQueueStatus Baselib_RegisteredNetwork_Socket_UDP_WaitForCompletedSend(
        WinApi::Baselib_RegisteredNetwork_Socket_UDP_Impl* socket,
        uint32_t                                           timeoutInMilliseconds,
        Baselib_ErrorState*                                errorState
    )
    {
        return socket->WaitForCompletedSend(timeoutInMilliseconds, errorState);
    }

    BASELIB_INLINE_IMPL Baselib_RegisteredNetwork_CompletionQueueStatus Baselib_RegisteredNetwork_Socket_UDP_WaitForCompletedRecv(
        WinApi::Baselib_RegisteredNetwork_Socket_UDP_Impl* socket,
        uint32_t                                           timeoutInMilliseconds,
        Baselib_ErrorState*                                errorState
    )
    {
        return socket->WaitForCompletedRecv(timeoutInMilliseconds, errorState);
    }

    BASELIB_INLINE_IMPL uint32_t Baselib_RegisteredNetwork_Socket_UDP_DequeueSend(
        WinApi::Baselib_RegisteredNetwork_Socket_UDP_Impl* socket,
        Baselib_RegisteredNetwork_CompletionResult*        results,
        uint32_t                                           resultsCount,
        Baselib_ErrorState*                                errorState
    )
    {
        UNUSED(errorState);
        return socket->DequeueSend(results, resultsCount);
    }

    BASELIB_INLINE_IMPL uint32_t Baselib_RegisteredNetwork_Socket_UDP_DequeueRecv(
        WinApi::Baselib_RegisteredNetwork_Socket_UDP_Impl* socket,
        Baselib_RegisteredNetwork_CompletionResult*        results,
        uint32_t                                           resultsCount,
        Baselib_ErrorState*                                errorState
    )
    {
        UNUSED(errorState);
        return socket->DequeueRecv(results, resultsCount);
    }

    BASELIB_INLINE_IMPL void Baselib_RegisteredNetwork_Socket_UDP_GetNetworkAddress(
        WinApi::Baselib_RegisteredNetwork_Socket_UDP_Impl* socket,
        Baselib_NetworkAddress*                            dstAddress,
        Baselib_ErrorState*                                errorState
    )
    {
        socket->GetNetworkAddress(dstAddress, errorState);
    }

    BASELIB_INLINE_IMPL void Baselib_RegisteredNetwork_Socket_UDP_Close(
        WinApi::Baselib_RegisteredNetwork_Socket_UDP_Impl* socket
    )
    {
        socket->~Baselib_RegisteredNetwork_Socket_UDP_Impl();
        Baselib_Memory_Free(socket);
    }
}
