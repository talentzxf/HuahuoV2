#pragma once

#define USE_MULTITHREADED_ASYNC_READ (ENABLE_MULTITHREADED_CODE && !UNITY_EXTERNAL_TOOL)

#if USE_MULTITHREADED_ASYNC_READ
#include "Runtime/Threads/Event.h"
#endif

#include "Job/JobTypes.h"
// #include "Runtime/VirtualFileSystem/VirtualFileSystemTypes.h"
#include "AssetContext.h"
#include "Utilities/File.h"

struct ReadCommand
{
    UInt8* buffer;
    size_t offset;
    UInt64 size;
};

struct AsyncReadCommand
{
    // Keep these in sync with AsyncReadManager.bindings.cs
    enum Status         { kCompleted = 0, kInProgress = 1, kFailure = 2, kNotUsed = 3 };
    enum CommandType    { kAsyncRead = 0, kCancelled = 3 };
    enum Priority       { kPriorityLow = 0, kPriorityHigh = 1 };
    typedef void(*UserCallbackType)(AsyncReadCommand &cmd, Status status);


    std::string   fileName; // file name to read from (Must not be touched while request is in flight.)
    UInt8*        buffer;   // buffer to read into. Memory must be provided by caller. (Must not be touched while request is in flight.)
    size_t        size;     // size to read (Must not be touched while request is in flight.)
    size_t        offset;   // seek position in the file (Must not be touched while request is in flight.)

    // status of the read command.
    // Query this to see if a command is still in progress or has already completed.
    // This value is set by the reading thread and may not be modified while a command is in flight.
    Status        status;

    // This value is read by the reading thread and can be changed at any point.
    //But is not guaranteed to be picked up if changed in midflight.
    CommandType   command;

    // Specify multiple reads in a single operation
    ReadCommand*  batchReads;
    unsigned int  batchReadCount;

    // Custom Data not used by Async but by Callers.
    void*         userData;

    // WARNING: If a user callback is specified, the status will not be set on the command since the user code
    // may choose to recycle the command in the callback. The user callback can set the status if it desires.
    UserCallbackType userCallback;

    Priority       priority;

    FileReadFlags   flags;

    // Info about the asset currently being read
#if ENABLE_PROFILER
private:
    AssetContext assetContext;
#endif

public:
    inline void SetAssetContext(UInt32 profilerFlowId, AssetSubsystem system, const std::string& loadingAssetName, SInt32 persistentTypeID)
    {
#if ENABLE_PROFILER
        assetContext = { persistentTypeID, loadingAssetName, profilerFlowId, system };
#endif
    }

    inline void SetAssetContext(AssetSubsystem system)
    {
#if ENABLE_PROFILER
        assetContext = { system };
#endif
    }

    inline void SetAssetContext(const AssetContext& _assetContext)
    {
#if ENABLE_PROFILER
        assetContext = _assetContext;
#endif
    }

#if ENABLE_PROFILER
    inline AssetSubsystem GetAssetSubsystem() const
    {
        return assetContext.subsystem;
    }

    inline core::string GetAssetSubsystemName() const
    {
        return assetContext.GetAssetSubsystemName();
    }

    inline const core::string& GetAssetName() const
    {
        return assetContext.assetName;
    }

    inline SInt32 GetAssetTypeID() const
    {
        return assetContext.assetTypeID;
    }

    UInt32 GetProfilerFlowId() const
    {
        return assetContext.profilerFlowId;
    }

    void SetProfilerFlowId(UInt32 profilerFlow)
    {
        assetContext.profilerFlowId = profilerFlow;
    }

#endif

    AsyncReadCommand() { Reset(); }
    // void SetMemoryLabel(MemLabelId label) { fileName.set_memory_label(label); }
    void Reset()
    {
        fileName.clear();
        buffer = NULL;
        size = 0;
        offset = (UInt64)0;
        status = kNotUsed;
        command = kAsyncRead;
        userData = NULL;
        userCallback = NULL;
        priority = kPriorityLow;
        batchReads = NULL;
        batchReadCount = 0;
        flags = kFileReadNoFlags;

#if ENABLE_PROFILER
        assetContext = {};
#endif
    }
};

// Request an async read.
// request and request.buffer may not be freed or touched while the request is in flight.
// When the Request has been processed. request.status will be set from another thread to indicate that it has completed / failed.
void AsyncReadRequest(AsyncReadCommand* request);

// Request an sync read. Executes read request immediately
void SyncReadRequest(AsyncReadCommand* request);

// Close the file at path
void AsyncReadForceCloseFile(const std::string& path);
void AsyncReadForceCloseAllFiles();

// Intialization and Cleanup. Must be called on application init / shutdown.
void InitializeAsyncReadManager();
void CleanupAsyncReadManager();

//BIND_MANAGED_TYPE_NAME(AsyncReadCommand::Status, Unity_IO_LowLevel_Unsafe_ReadStatus);
//BIND_MANAGED_TYPE_NAME(AsyncReadCommand::Priority, Unity_IO_LowLevel_Unsafe_Priority);
//BIND_MANAGED_TYPE_NAME(AssetSubsystem, Unity_IO_LowLevel_Unsafe_AssetLoadingSubsystem);
//BIND_MANAGED_TYPE_NAME(ReadCommand, Unity_IO_LowLevel_Unsafe_ReadCommand);
