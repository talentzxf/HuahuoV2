#include "AsyncReadManager.h"

#if !USE_MULTITHREADED_ASYNC_READ

#include "OpenFileCache.h"
#include "Memory/MemoryMacros.h"

static OpenFileCache* gOpenFileCache = NULL;

void InitializeAsyncReadManager()
{
    Assert(gOpenFileCache == NULL);
    gOpenFileCache = HUAHUO_NEW_AS_ROOT(OpenFileCache, kMemManager, "Managers", "AsyncReadManager") ();

#if ENABLE_PROFILER
    InitializeAsyncReadManagerMetrics();
#if UNITY_EDITOR
    GlobalCallbacks::Get().exitPlayModeAfterOnEnableInEditMode.Register(AsyncReadManagerMetrics_OnExitPlayModeStatic);
#endif
#endif
}

void CleanupAsyncReadManager()
{
#if ENABLE_PROFILER
    ShutdownAsyncReadManagerMetrics();
#endif
    Assert(gOpenFileCache != NULL);
    HUAHUO_DELETE(gOpenFileCache, kMemManager);
}

void AsyncReadRequest(AsyncReadCommand* request)
{
    // ADD_READCOMMANDMETRIC(GetAsyncReadManagerMetrics(), request, FileReadType::Sync, 0);
    SyncReadRequest(request);
}

void AsyncReadWaitDone(AsyncReadCommand* request)
{
}

void SyncReadRequest(AsyncReadCommand* request)
{
    File* file = gOpenFileCache->OpenCached(request->fileName);
    bool readOk = false;
    if (file != NULL)
    {
        // UPDATE_READCOMMANDMETRIC_ONREAD(GetAsyncReadManagerMetrics(), request, 0);
        readOk = file->Read(request->offset, request->buffer, request->size) == request->size;
    }

    AsyncReadCommand::Status newStatus = readOk ? AsyncReadCommand::kCompleted : AsyncReadCommand::kFailure;
    if (request->userCallback != NULL)
        request->userCallback(*request, newStatus);
    else
        request->status = newStatus;
    // UPDATE_READCOMMANDMETRIC_ONCOMPLETE(GetAsyncReadManagerMetrics(), request, newStatus, 0);
}

void AsyncReadForceCloseFile(const std::string& path)
{
    gOpenFileCache->ForceClose(path);
}

void AsyncReadForceCloseAllFiles()
{
    gOpenFileCache->ForceCloseAll();
}

#endif
