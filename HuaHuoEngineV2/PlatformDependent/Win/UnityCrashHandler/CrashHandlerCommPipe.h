#pragma once

#include "UnityPrefix.h"
#include <windef.h>
#include <synchapi.h>

class CommPipeMessageHandler
{
public:
    virtual void ProcessMessage(const char* payload, size_t payloadSize) = 0;
};

class CrashHandlerCommPipe
{
public:
    CrashHandlerCommPipe(HANDLE hPipe, CommPipeMessageHandler* handler);
    ~CrashHandlerCommPipe();

    int Stop();

    HANDLE ThreadHandle() const { return m_hThread; }

private:
    static bool ReadCompleteChunk(HANDLE hFile, void* data, size_t dataSize);
    static bool ReadCompleteMessage(HANDLE hFile, std::vector<char>& message);
    static DWORD CALLBACK CommThreadProc(CrashHandlerCommPipe* pipe);

    HANDLE m_hPipe;
    HANDLE m_hThread;
    CommPipeMessageHandler* m_Handler;
    CRITICAL_SECTION m_CS;
    volatile bool m_Abort;
};
