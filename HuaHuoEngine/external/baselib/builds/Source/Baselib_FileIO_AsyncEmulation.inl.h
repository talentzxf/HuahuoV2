#pragma once

#include "Include/Baselib.h"
#include "Include/C/Baselib_FileIO.h"
#include "Include/Cpp/mpsc_node_queue.h"
#include "Include/Cpp/HighCapacitySemaphore.h"

class Baselib_FileIO_AsyncEmulation;

namespace Baselib_FileIO_AsyncEmulation_Detail
{
    class AsyncOperation : public baselib::mpsc_node
    {
    public:
        void* operator new(size_t size)
        {
            // Rely on https://docs.microsoft.com/en-us/previous-versions/6ewkz86d(v%3Dvs.140) for now
            static_assert(alignof(AsyncOperation) <= alignof(double), "Must be naturally aligned");
            BaselibAssert(size >= sizeof(AsyncOperation));
            void* memory = Baselib_Memory_Allocate(sizeof(AsyncOperation));
            BaselibAssert((reinterpret_cast<uintptr_t>(memory) & (alignof(AsyncOperation) - 1)) == 0);
            return memory;
        }

        void operator delete(void* memory)
        {
            Baselib_Memory_Free(memory);
        }

        static AsyncOperation* Open(Baselib_FileIO_AsyncEmulation* parent, uint64_t userdata, Baselib_FileIO_Priority priority, char* pathname)
        {
            auto op = new AsyncOperation(parent, Baselib_FileIO_EventQueue_OpenFile, userdata, priority);
            op->open.status = false;
            op->open.handle = 0;
            op->open.fileSize = 0;
            op->open.closed = false;
            op->open.pathname = pathname;
            return op;
        }

        static AsyncOperation* Read(Baselib_FileIO_AsyncEmulation* parent, uint64_t userdata, Baselib_FileIO_Priority priority, AsyncOperation* file, Baselib_FileIO_ReadRequest request)
        {
            auto op = new AsyncOperation(parent, Baselib_FileIO_EventQueue_ReadFile, userdata, priority);
            op->read.file = file;
            op->read.request = request;
            return op;
        }

        Baselib_FileIO_AsyncEmulation*             parent;
        const Baselib_FileIO_EventQueue_ResultType type;
        const uint64_t                             userdata;
        const Baselib_FileIO_Priority              priority;
        baselib::atomic<uint64_t>                  refcount;

        union
        {
            struct
            {
                baselib::atomic<bool>      status;
                baselib::atomic<uintptr_t> handle;
                baselib::atomic<uint64_t>  fileSize;
                baselib::atomic<bool>      closed;
                char*                      pathname;
            } open;

            struct
            {
                AsyncOperation*            file;
                Baselib_FileIO_ReadRequest request;
            } read;
        };

    private:
        AsyncOperation(Baselib_FileIO_AsyncEmulation* setParent, Baselib_FileIO_EventQueue_ResultType setType, uint64_t setUserdata, Baselib_FileIO_Priority setPriority)
            : parent(setParent)
            , type(setType)
            , userdata(setUserdata)
            , priority(setPriority)
            , refcount(1)
        {
        }
    };

    class EventNode : public baselib::mpsc_node
    {
    public:
        EventNode(Baselib_FileIO_EventQueue_Result setEvent)
            : event(setEvent)
        {
        }

        void* operator new(size_t size)
        {
            // Rely on https://docs.microsoft.com/en-us/previous-versions/6ewkz86d(v%3Dvs.140) for now
            static_assert(alignof(EventNode) <= alignof(double), "Must be naturally aligned");
            BaselibAssert(size >= sizeof(EventNode));
            void* memory = Baselib_Memory_Allocate(sizeof(EventNode));
            BaselibAssert((reinterpret_cast<uintptr_t>(memory) & (alignof(EventNode) - 1)) == 0);
            return memory;
        }

        void operator delete(void* memory)
        {
            Baselib_Memory_Free(memory);
        }

        Baselib_FileIO_EventQueue_Result event;
    };
}

class Baselib_FileIO_AsyncEmulation
{
public:
    Baselib_FileIO_AsyncEmulation() = default;

    virtual ~Baselib_FileIO_AsyncEmulation() = default;

    Baselib_FileIO_AsyncEmulation(const Baselib_FileIO_AsyncEmulation&) = delete;

    Baselib_FileIO_AsyncEmulation(Baselib_FileIO_AsyncEmulation&&) = delete;

    Baselib_FileIO_AsyncEmulation& operator=(const Baselib_FileIO_AsyncEmulation&) = delete;

    Baselib_FileIO_AsyncEmulation& operator=(Baselib_FileIO_AsyncEmulation&&) = delete;

    virtual uintptr_t SyncOpen(const char* pathname, uint64_t* outFileSize, Baselib_ErrorState* errorState) = 0;

    virtual uint32_t SyncRead(uintptr_t descriptor, uint64_t offset, void* buffer, uint32_t size, Baselib_ErrorState* errorState) = 0;

    virtual void SyncClose(uintptr_t descriptor, Baselib_ErrorState* errorState) = 0;

    uint64_t Dequeue(
        Baselib_FileIO_EventQueue_Result results[],
        uint64_t                         count,
        uint32_t                         timeoutInMilliseconds
    )
    {
        for (uint64_t i = 0; i < count; ++i)
        {
            if (i == 0)
            {
                if (!eventsCounter.TryTimedAcquire(baselib::timeout_ms(timeoutInMilliseconds)))
                    return i;
            }
            else
            {
                if (!eventsCounter.TryAcquire())
                    return i;
            }

            auto event = events.try_pop_front();
            while (!event)
            {
                Baselib_Thread_YieldExecution();
                event = events.try_pop_front();
            }
            BaselibAssert(event);

            results[i] = event->event;
            delete event;
        }
        return count;
    }

    using AsyncOperation = Baselib_FileIO_AsyncEmulation_Detail::AsyncOperation;

    AsyncOperation* AsyncOpen(
        const char*               pathname,
        uint64_t                  userdata,
        Baselib_FileIO_Priority   priority
    )
    {
        size_t pathnameCopyLen = strlen(pathname) + 1;
        char* pathnameCopy = reinterpret_cast<char*>(Baselib_Memory_Allocate(pathnameCopyLen));
        snprintf(pathnameCopy, pathnameCopyLen, "%s", pathname);

        auto op = AsyncOperation::Open(this, userdata, priority, pathnameCopy);
        // required for opening a file if close is scheduled immediately after
        IncrRef(op);

        // Priority is ignored and file open will happen ASAP.
        // This is needed to avoid a deadlock when
        // open is invoked with normal prio and read with high prio.
        // In that case failing read would starve open.
        ScheduleInternalPriority(op);
        return op;
    }

    void AsyncRead(
        AsyncOperation*            file,
        Baselib_FileIO_ReadRequest requests[],
        uint64_t                   count,
        uint64_t                   userdata,
        Baselib_FileIO_Priority    priority
    )
    {
        BaselibAssert(file->open.closed == false);
        for (uint64_t i = 0; i < count; ++i)
        {
            IncrRef(file);
            Schedule(AsyncOperation::Read(this, userdata, priority, file, requests[i]), priority);
        }
    }

    void AsyncClose(AsyncOperation* file)
    {
        if (!file->open.closed)
        {
            file->open.closed = true;
            DecrRef(file);
        }
    }

protected:
    using EventNode = Baselib_FileIO_AsyncEmulation_Detail::EventNode;

    baselib::mpsc_node_queue<EventNode> events;
    baselib::mpsc_node_queue<AsyncOperation> fifoInternal, fifoHigh, fifoNormal;
    baselib::HighCapacitySemaphore eventsCounter;

    void Notify(Baselib_FileIO_EventQueue_Result event)
    {
        events.push_back(new EventNode(event));
        eventsCounter.Release(1);
    }

    void RequestPump()
    {
        Baselib_FileIO_EventQueue_Result ev = {};
        ev.type = Baselib_FileIO_EventQueue_Callback;
        ev.callback.callback = [](uint64_t ptr) {
                auto emulation = static_cast<Baselib_FileIO_AsyncEmulation*>(reinterpret_cast<void*>(ptr));
                emulation->Pump();
            };
        ev.userdata = reinterpret_cast<uint64_t>(this);
        Notify(ev);
    }

    void Schedule(AsyncOperation* op, Baselib_FileIO_Priority prio)
    {
        switch (prio)
        {
            case Baselib_FileIO_Priority_High:
                fifoHigh.push_back(op);
                break;
            case Baselib_FileIO_Priority_Normal:
                fifoNormal.push_back(op);
                break;
        }
        RequestPump();
    }

    void ScheduleInternalPriority(AsyncOperation* op)
    {
        fifoInternal.push_back(op);
        RequestPump();
    }

    AsyncOperation* Next()
    {
        // try_pop_front can fail under contention,
        // but we only have limited amount of pumps,
        // so we need to take extra effort here.
        AsyncOperation* op = nullptr;
        while (!fifoInternal.empty())
        {
            if ((op = fifoInternal.try_pop_front()))
                return op;
            Baselib_Thread_YieldExecution();
        }
        while (!fifoHigh.empty())
        {
            if ((op = fifoHigh.try_pop_front()))
                return op;
            Baselib_Thread_YieldExecution();
        }
        while (!fifoNormal.empty())
        {
            if ((op = fifoNormal.try_pop_front()))
                return op;
            Baselib_Thread_YieldExecution();
        }
        return op;
    }

    void IncrRef(AsyncOperation* op) const
    {
        op->refcount++;
    }

    void DecrRef(AsyncOperation* op)
    {
        BaselibAssert(op->refcount > 0);

        if ((op->refcount--) == 1)
        {
            // Some operations might need blocking cleanup,
            // but DecrRef could be used from any thread.
            // So we schedule it instead.
            // In other cases we destroy object at the spot.
            switch (op->type)
            {
                case Baselib_FileIO_EventQueue_OpenFile:
                    Schedule(op, Baselib_FileIO_Priority_Normal);
                    break;
                default:
                    delete op;
                    break;
            }
        }
    }

    void Pump()
    {
        auto op = Next();
        BaselibAssert(op != nullptr);
        if (!op)
            return;

        if (op->refcount == 0)
        {
            switch (op->type)
            {
                case Baselib_FileIO_EventQueue_OpenFile:
                {
                    Baselib_FileIO_EventQueue_Result ev = {};
                    ev.type = Baselib_FileIO_EventQueue_CloseFile;
                    ev.userdata = op->userdata;
                    ev.errorState = Baselib_ErrorState_Create();
                    BaselibAssert(op->open.status == true);
                    SyncClose(op->open.handle, &ev.errorState);
                    op->open.status = 0;
                    Notify(ev);

                    Baselib_Memory_Free(op->open.pathname);
                    op->open.pathname = nullptr;

                    break;
                }
                default:
                    break;
            }

            delete op;
        }
        else
        {
            switch (op->type)
            {
                case Baselib_FileIO_EventQueue_OpenFile:
                {
                    Baselib_FileIO_EventQueue_Result ev = {};
                    ev.type = op->type;
                    ev.userdata = op->userdata;
                    ev.errorState = Baselib_ErrorState_Create();

                    uint64_t fileSize = 0;
                    op->open.handle = SyncOpen(op->open.pathname, &fileSize, &ev.errorState);
                    op->open.fileSize = fileSize;
                    op->open.status = true;
                    DecrRef(op);

                    ev.openFile.fileSize = fileSize;
                    Notify(ev);

                    break;
                }
                case Baselib_FileIO_EventQueue_ReadFile:
                {
                    if (op->read.file->open.status)
                    {
                        Baselib_FileIO_EventQueue_Result ev = {};
                        ev.type = op->type;
                        ev.userdata = op->userdata;
                        ev.errorState = Baselib_ErrorState_Create();

                        uint64_t offset = op->read.request.offset;
                        uint32_t size = op->read.request.size;
                        uint64_t fileSize = op->read.file->open.fileSize;

                        if (offset > fileSize)
                        {
                            ev.readFile.bytesTransfered = 0;
                        }
                        else
                        {
                            if (offset + size > fileSize)
                            {
                                uint64_t size64 = fileSize - offset;
                                BaselibAssert(size64 <= UINT32_MAX);
                                size = static_cast<uint32_t>(size64);
                            }

                            ev.readFile.bytesTransfered = SyncRead(
                                op->read.file->open.handle,
                                offset,
                                op->read.request.buffer,
                                size,
                                &ev.errorState
                            );
                        }
                        Notify(ev);
                        DecrRef(op->read.file);
                        DecrRef(op);
                    }
                    else
                    {
                        // In single threaded environment, reads will always happen after opens.
                        // In multithreaded environment, there might be a small delay between
                        // open completing and reads starting to proceed.
                        // Rescheduling to that back will speculatively
                        // allow other reads on other fd to potentially proceed.
                        Schedule(op, op->priority);
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }
};
