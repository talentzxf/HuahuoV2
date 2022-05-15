#pragma once

#include "Include/Baselib.h"

#include "Include/C/Baselib_RegisteredNetwork.h"

#include "Include/C/Baselib_Thread.h"
#include "Include/C/Baselib_Socket.h"
#include "Include/C/Baselib_Memory.h"
#include "Include/Cpp/mpmc_fixed_queue.h"
#include "Include/Cpp/EventSemaphore.h"
#include "Include/Cpp/CountdownTimer.h"

#include "Source/Baselib_InlineImplementation.h"
#include "Source/Baselib_ErrorState_Utils.h"

#include <vector>
#include <set>

namespace Common
{
    struct Baselib_RegisteredNetwork_Socket_UDP_Impl;

    class Baselib_RegisteredNetwork_CompletionQueue
    {
    public:
        Baselib_RegisteredNetwork_CompletionQueue(uint32_t queueSize)
            : results(queueSize)
        {}

        ~Baselib_RegisteredNetwork_CompletionQueue() = default;

        Baselib_RegisteredNetwork_CompletionQueueStatus WaitForResult(baselib::timeout_ms timeoutInMilliseconds)
        {
            return wait.TryTimedAcquire(timeoutInMilliseconds) ?
                Baselib_RegisteredNetwork_CompletionQueueStatus_ResultsAvailable :
                Baselib_RegisteredNetwork_CompletionQueueStatus_NoResultsAvailable;
        }

        // Push an element in a blocking manner.
        void PushResult(const Baselib_RegisteredNetwork_CompletionResult& result)
        {
            // even with only queueSize elements in the completion & request queing system,
            // try_push_back may fail in rare cases if there are temporary holes in the queue due to several threads popping elements.
            while (!results.try_push_back(result)) { Baselib_Thread_YieldExecution(); }
            wait.Set();
        }

        // Tries to pop a result from the completion queue.
        // If failed, the wait event resets, so that anyone waiting for the event needs to wait until something was pushed again.
        bool TryPopResult(Baselib_RegisteredNetwork_CompletionResult& result)
        {
            // try_pop_front may fail if there are several pushes in flight at once and some of them are not finished yet.
            // (see try_pop_front documentation)
            // Meaning that we may fail popping an element here despite already being completions in the queue. This does not pose a problem
            // though since the last of the parallel pushes will reset the event again (see PushResult method).
            if (!results.try_pop_front(result))
            {
                wait.Reset();
                return false;
            }
            return true;
        }

    private:
        baselib::mpmc_fixed_queue<Baselib_RegisteredNetwork_CompletionResult> results;
        // aligning this semaphore doesn't cost us anything since the fields in the fixed_queue are already aligned
        alignas(PLATFORM_CACHE_LINE_SIZE) baselib::EventSemaphore wait;
    };

    class Baselib_RegisteredNetwork_RequestQueue_Impl
    {
    public:
        Baselib_RegisteredNetwork_RequestQueue_Impl(uint32_t queueSize)
            : highPrio(queueSize)
            , normalPrio(queueSize)
        {
        }

        ~Baselib_RegisteredNetwork_RequestQueue_Impl() = default;

        using ProcessCallback = bool(Baselib_RegisteredNetwork_Socket_UDP_Impl * socket, const Baselib_RegisteredNetwork_Request& request);

        // return true if there is more work to do
        Baselib_RegisteredNetwork_ProcessStatus ProcessRequest(Baselib_RegisteredNetwork_Socket_UDP_Impl* socket, ProcessCallback callback)
        {
            Baselib_RegisteredNetwork_Request req;

            // first processing high prio queue
            // Note that for a single thread, highPrio queue has always only a single element, with multiple threads though it can fill up easily!
            bool anyRequestAvailable = highPrio.try_pop_front(req);
            if (!anyRequestAvailable)
                anyRequestAvailable = normalPrio.try_pop_front(req);
            if (!anyRequestAvailable)
                return Baselib_RegisteredNetwork_ProcessStatus_NonePendingImmediately;

            if (!callback(socket, req))
            {
                // Reschedule it ASAP on failure, so we don't impact latency too much. Ideally, we would want to put it first in the queue.
                // If element was in normal priority queue, we put it into high priority so it will be processed first next time.
                // try_push_back may fail in rare cases if there are temporary holes in the queue due to several threads popping elements.
                while (!highPrio.try_push_back(req)) { Baselib_Thread_YieldExecution(); }

                // Apparently we should try later again. Doing our processing immediately again will likely not help.
                return Baselib_RegisteredNetwork_ProcessStatus_NonePendingImmediately;
            }

            // We successfully processed a request, assume there is more to come.
            return Baselib_RegisteredNetwork_ProcessStatus_Pending;
        }

        // Push an element in a blocking manner.
        void PushRequest(const Baselib_RegisteredNetwork_Request& req)
        {
            // even with only queueSize elements in the completion & request queing system,
            // try_push_back may fail in rare cases if there are temporary holes in the queue due to several threads popping elements.
            while (!normalPrio.try_push_back(req)) { Baselib_Thread_YieldExecution(); }
        }

    private:
        baselib::mpmc_fixed_queue<Baselib_RegisteredNetwork_Request> highPrio;
        baselib::mpmc_fixed_queue<Baselib_RegisteredNetwork_Request> normalPrio;
    };

    struct Baselib_RegisteredNetwork_Socket_UDP_Impl
    {
        Baselib_RegisteredNetwork_Socket_UDP_Impl(Baselib_Socket_Handle baselibSocket, uint32_t sendRequestQueueSize, uint32_t recvRequestQueueSize)
            : baselibSocket(baselibSocket)
            , sendRq(sendRequestQueueSize)
            , sendCq(sendRequestQueueSize)
            , sendNumFreeSlots(sendRequestQueueSize)
            , recvRq(recvRequestQueueSize)
            , recvCq(recvRequestQueueSize)
            , recvNumFreeSlots(recvRequestQueueSize)
        {
        }

        ~Baselib_RegisteredNetwork_Socket_UDP_Impl()
        {
            Baselib_Socket_Close(baselibSocket);
        }

        uint32_t ScheduleSend(const Baselib_RegisteredNetwork_Request* requests, uint32_t requestsCount)
        {
            for (uint32_t i = 0; i < requestsCount; ++i)
            {
                if (!AcquireQueueSlot(sendNumFreeSlots))
                    return i;
                sendRq.PushRequest(requests[i]);
            }
            return requestsCount;
        }

        uint32_t ScheduleRecv(const Baselib_RegisteredNetwork_Request* requests, uint32_t requestsCount)
        {
            for (uint32_t i = 0; i < requestsCount; ++i)
            {
                if (!AcquireQueueSlot(recvNumFreeSlots))
                    return i;
                recvRq.PushRequest(requests[i]);
            }
            return requestsCount;
        }

        Baselib_RegisteredNetwork_ProcessStatus ProcessSendRequest()       { return sendRq.ProcessRequest(this, ProcessSend); }
        Baselib_RegisteredNetwork_ProcessStatus ProcessReceiveRequest()    { return recvRq.ProcessRequest(this, ProcessRecv); }

        Baselib_RegisteredNetwork_CompletionQueueStatus WaitForCompletedSend(baselib::timeout_ms timeoutInMilliseconds) { return sendCq.WaitForResult(timeoutInMilliseconds); }
        Baselib_RegisteredNetwork_CompletionQueueStatus WaitForCompletedRecv(baselib::timeout_ms timeoutInMilliseconds) { return recvCq.WaitForResult(timeoutInMilliseconds); }

        uint32_t DequeueSend(Baselib_RegisteredNetwork_CompletionResult* results, uint32_t resultsCount)
        {
            uint32_t numProcessedResults = 0;
            for (; numProcessedResults < resultsCount && sendCq.TryPopResult(results[numProcessedResults]); ++numProcessedResults)
                sendNumFreeSlots.fetch_add(1, baselib::memory_order_relaxed);
            return numProcessedResults;
        }

        uint32_t DequeueRecv(Baselib_RegisteredNetwork_CompletionResult* results, uint32_t resultsCount)
        {
            uint32_t numProcessedResults = 0;
            for (; numProcessedResults < resultsCount && recvCq.TryPopResult(results[numProcessedResults]); ++numProcessedResults)
                recvNumFreeSlots.fetch_add(1, baselib::memory_order_relaxed);
            return numProcessedResults;
        }

        void GetNetworkAddress(Baselib_NetworkAddress* dstAddress, Baselib_ErrorState* errorState) const
        {
            Baselib_Socket_GetAddress(baselibSocket, dstAddress, errorState);
        }

    private:
        static bool ProcessSend(Baselib_RegisteredNetwork_Socket_UDP_Impl* socket, const Baselib_RegisteredNetwork_Request& request)
        {
            Baselib_Socket_Message message;
            message.data = request.payload.data;
            message.dataLen = request.payload.size;
            message.address = (Baselib_NetworkAddress*)request.remoteEndpoint.slice.data;

            Baselib_ErrorState errorState = Baselib_ErrorState_Create();
            uint32_t resultsCount = Baselib_Socket_UDP_Send(socket->baselibSocket, &message, 1, &errorState);
            bool errorRaised = Baselib_ErrorState_ErrorRaised(&errorState);
            if (resultsCount == 0 && !errorRaised)
                return false;
            BaselibAssert(resultsCount == 1 || errorRaised);

            Baselib_RegisteredNetwork_CompletionResult result;
            result.requestUserdata = request.requestUserdata;
            result.status = errorRaised ? Baselib_RegisteredNetwork_CompletionStatus_Failed : Baselib_RegisteredNetwork_CompletionStatus_Success;
            result.bytesTransferred = message.dataLen;

            socket->sendCq.PushResult(result);
            return true;
        }

        static bool ProcessRecv(Baselib_RegisteredNetwork_Socket_UDP_Impl* socket, const Baselib_RegisteredNetwork_Request& request)
        {
            Baselib_Socket_Message message;
            message.data = request.payload.data;
            message.dataLen = request.payload.size;
            message.address = (Baselib_NetworkAddress*)request.remoteEndpoint.slice.data;

            Baselib_ErrorState errorState = Baselib_ErrorState_Create();
            uint32_t resultsCount = Baselib_Socket_UDP_Recv(socket->baselibSocket, &message, 1, &errorState);
            bool errorRaised = Baselib_ErrorState_ErrorRaised(&errorState);
            if (resultsCount == 0 && !errorRaised)
                return false;
            BaselibAssert(resultsCount == 1 || errorRaised);

            Baselib_RegisteredNetwork_CompletionResult result;
            result.requestUserdata = request.requestUserdata;
            result.status = errorRaised ? Baselib_RegisteredNetwork_CompletionStatus_Failed : Baselib_RegisteredNetwork_CompletionStatus_Success;
            result.bytesTransferred = message.dataLen;

            socket->recvCq.PushResult(result);
            return true;
        }

        // return true if we acquired slot in FIFO's to process one request
        // this guarantees that pushes to different FIFO's in scheduler will never fail
        // which means we will never drop any requests during processing
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

        const Baselib_Socket_Handle baselibSocket;

        Common::Baselib_RegisteredNetwork_RequestQueue_Impl sendRq;
        Common::Baselib_RegisteredNetwork_CompletionQueue sendCq;
        baselib::atomic<uint32_t> sendNumFreeSlots; // Total number of free send requests & completions

        Common::Baselib_RegisteredNetwork_RequestQueue_Impl recvRq;
        Common::Baselib_RegisteredNetwork_CompletionQueue recvCq;
        baselib::atomic<uint32_t> recvNumFreeSlots; // Total number of free recv requests & completions
    };

    BASELIB_INLINE_IMPL Baselib_RegisteredNetwork_Buffer Baselib_RegisteredNetwork_Buffer_Register(
        Baselib_Memory_PageAllocation pageAllocation,
        Baselib_ErrorState* errorState
    )
    {
        UNUSED(errorState);
        return { pageAllocation.ptr, pageAllocation };
    }

    BASELIB_INLINE_IMPL void Baselib_RegisteredNetwork_Buffer_Deregister(
        Baselib_RegisteredNetwork_Buffer  buffer
    )
    {
        UNUSED(buffer);
    }

    BASELIB_INLINE_IMPL Baselib_RegisteredNetwork_Endpoint Baselib_RegisteredNetwork_Endpoint_Create(
        const Baselib_NetworkAddress*         srcAddress,
        Baselib_RegisteredNetwork_BufferSlice dstSlice,
        Baselib_ErrorState*                   errorState
    )
    {
        static_assert(sizeof(Baselib_NetworkAddress) <= Baselib_RegisteredNetwork_Endpoint_MaxSize,
            "Network address is bigger than the max size for an endpoint");

        Baselib_NetworkAddress* const dstAddress = (Baselib_NetworkAddress*)dstSlice.data;
        if (srcAddress)
            *dstAddress = *srcAddress;
        else
            *dstAddress = Baselib_NetworkAddress_Empty();
        dstSlice.size = sizeof(Baselib_NetworkAddress);

        return {dstSlice};
    }

    BASELIB_INLINE_IMPL void Baselib_RegisteredNetwork_Endpoint_GetNetworkAddress(
        Baselib_RegisteredNetwork_Endpoint endpoint,
        Baselib_NetworkAddress*            dstAddress,
        Baselib_ErrorState*                errorState
    )
    {
        memcpy(dstAddress, endpoint.slice.data, sizeof(Baselib_NetworkAddress));
    }

    BASELIB_INLINE_IMPL Common::Baselib_RegisteredNetwork_Socket_UDP_Impl* Baselib_RegisteredNetwork_Socket_UDP_Create(
        const Baselib_NetworkAddress*       bindAddress,
        Baselib_NetworkAddress_AddressReuse addressReuse,
        uint32_t                            sendQueueSize,
        uint32_t                            recvQueueSize,
        Baselib_ErrorState*                 errorState
    )
    {
        Baselib_NetworkAddress_Family family;
        Baselib_NetworkAddress_Decode(bindAddress, &family, nullptr, 0, nullptr, errorState);
        auto baselibSocket = Baselib_Socket_Create(family, Baselib_Socket_Protocol_UDP, errorState);
        Baselib_Socket_Bind(baselibSocket, bindAddress, addressReuse, errorState);

        if (Baselib_ErrorState_ErrorRaised(errorState))
        {
            Baselib_Socket_Close(baselibSocket);
            return nullptr;
        }

        void* memory = Baselib_Memory_AlignedAllocate(sizeof(Baselib_RegisteredNetwork_Socket_UDP_Impl), alignof(Baselib_RegisteredNetwork_Socket_UDP_Impl));
        return new(memory) Baselib_RegisteredNetwork_Socket_UDP_Impl(baselibSocket, sendQueueSize, recvQueueSize);
    }

    BASELIB_INLINE_IMPL uint32_t Baselib_RegisteredNetwork_Socket_UDP_ScheduleSend(
        Common::Baselib_RegisteredNetwork_Socket_UDP_Impl* socket,
        const Baselib_RegisteredNetwork_Request*           requests,
        uint32_t                                           requestsCount,
        Baselib_ErrorState*                                errorState
    )
    {
        return socket->ScheduleSend(requests, requestsCount);
    }

    BASELIB_INLINE_IMPL uint32_t Baselib_RegisteredNetwork_Socket_UDP_ScheduleRecv(
        Common::Baselib_RegisteredNetwork_Socket_UDP_Impl* socket,
        const Baselib_RegisteredNetwork_Request*           requests,
        uint32_t                                           requestsCount,
        Baselib_ErrorState*                                errorState
    )
    {
        return socket->ScheduleRecv(requests, requestsCount);
    }

    BASELIB_INLINE_IMPL Baselib_RegisteredNetwork_ProcessStatus Baselib_RegisteredNetwork_Socket_UDP_ProcessSend(
        Common::Baselib_RegisteredNetwork_Socket_UDP_Impl* socket,
        Baselib_ErrorState*                  errorState
    )
    {
        return socket->ProcessSendRequest();
    }

    BASELIB_INLINE_IMPL Baselib_RegisteredNetwork_ProcessStatus Baselib_RegisteredNetwork_Socket_UDP_ProcessRecv(
        Common::Baselib_RegisteredNetwork_Socket_UDP_Impl* socket,
        Baselib_ErrorState*                  errorState
    )
    {
        return socket->ProcessReceiveRequest();
    }

    BASELIB_INLINE_IMPL Baselib_RegisteredNetwork_CompletionQueueStatus Baselib_RegisteredNetwork_Socket_UDP_WaitForCompletedSend(
        Common::Baselib_RegisteredNetwork_Socket_UDP_Impl* socket,
        uint32_t                                           timeoutInMilliseconds,
        Baselib_ErrorState*                                errorState
    )
    {
        return socket->WaitForCompletedSend(baselib::timeout_ms(timeoutInMilliseconds));
    }

    BASELIB_INLINE_IMPL Baselib_RegisteredNetwork_CompletionQueueStatus Baselib_RegisteredNetwork_Socket_UDP_WaitForCompletedRecv(
        Common::Baselib_RegisteredNetwork_Socket_UDP_Impl* socket,
        uint32_t                                           timeoutInMilliseconds,
        Baselib_ErrorState*                                errorState
    )
    {
        return socket->WaitForCompletedRecv(baselib::timeout_ms(timeoutInMilliseconds));
    }

    BASELIB_INLINE_IMPL uint32_t Baselib_RegisteredNetwork_Socket_UDP_DequeueSend(
        Common::Baselib_RegisteredNetwork_Socket_UDP_Impl*        socket,
        Baselib_RegisteredNetwork_CompletionResult*               results,
        uint32_t                                                  resultsCount,
        Baselib_ErrorState*                                       errorState
    )
    {
        return socket->DequeueSend(results, resultsCount);
    }

    BASELIB_INLINE_IMPL uint32_t Baselib_RegisteredNetwork_Socket_UDP_DequeueRecv(
        Common::Baselib_RegisteredNetwork_Socket_UDP_Impl*        socket,
        Baselib_RegisteredNetwork_CompletionResult*               results,
        uint32_t                                                  resultsCount,
        Baselib_ErrorState*                                       errorState
    )
    {
        return socket->DequeueRecv(results, resultsCount);
    }

    BASELIB_INLINE_IMPL void Baselib_RegisteredNetwork_Socket_UDP_GetNetworkAddress(
        Common::Baselib_RegisteredNetwork_Socket_UDP_Impl* socket,
        Baselib_NetworkAddress*                            dstAddress,
        Baselib_ErrorState*                                errorState
    )
    {
        socket->GetNetworkAddress(dstAddress, errorState);
    }

    BASELIB_INLINE_IMPL void Baselib_RegisteredNetwork_Socket_UDP_Close(
        Common::Baselib_RegisteredNetwork_Socket_UDP_Impl* socket
    )
    {
        socket->~Baselib_RegisteredNetwork_Socket_UDP_Impl();
        Baselib_Memory_AlignedFree(socket);
    }
}
