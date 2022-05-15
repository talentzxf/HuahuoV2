#include "CrashHandlerCommPipe.h"
#include <windows.h>

class CancelSynchronousIO
{
public:
    CancelSynchronousIO()
    {
        m_hKernel32 = LoadLibraryA("Kernel32.dll");
        m_Proc = (CANCEL_SYNCHRONOUS_IO_PROC)GetProcAddress(m_hKernel32, "CancelSynchronousIo");
    }

    ~CancelSynchronousIO()
    {
        FreeLibrary(m_hKernel32);
    }

    bool IsAvailable() const
    {
        return m_Proc != nullptr;
    }

    int Cancel(HANDLE hThread) const
    {
        int r = m_Proc(hThread);
        if (r == 0)
            return GetLastError();
        else
            return 0;
    }

private:
    typedef BOOL (WINAPI * CANCEL_SYNCHRONOUS_IO_PROC)(HANDLE);
    CANCEL_SYNCHRONOUS_IO_PROC m_Proc;
    HMODULE m_hKernel32;
};

CrashHandlerCommPipe::CrashHandlerCommPipe(HANDLE hPipe, CommPipeMessageHandler* handler)
    : m_hPipe(hPipe)
    , m_hThread(nullptr)
    , m_Handler(handler)
    , m_Abort(false)
{
    InitializeCriticalSection(&m_CS);
    m_hThread = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)CommThreadProc, this, 0, nullptr);
}

CrashHandlerCommPipe::~CrashHandlerCommPipe()
{
    if (m_hPipe != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_hPipe);
        m_hPipe = INVALID_HANDLE_VALUE;
    }

    if (m_hThread != INVALID_HANDLE_VALUE)
    {
        Stop();

        CloseHandle(m_hThread);
        m_hThread = INVALID_HANDLE_VALUE;
    }

    DeleteCriticalSection(&m_CS);
}

int CrashHandlerCommPipe::Stop()
{
    if (m_hThread != INVALID_HANDLE_VALUE)
    {
        // Make sure all pending data is read
        FlushFileBuffers(m_hPipe);

        // Abort the read loop and close the pipe
        m_Abort = true;
        CloseHandle(m_hPipe);
        m_hPipe = INVALID_HANDLE_VALUE;

        // Abort any pending synchronous reads
        CancelSynchronousIO cancelIO;
        if (cancelIO.IsAvailable())
        {
            // Attempt to cancel the thread while the thread still lives
            int r;
            while ((r = cancelIO.Cancel(m_hThread)) != 0 &&
                   WaitForSingleObject(m_hThread, 0) == WAIT_TIMEOUT)
            {
                // there's no outstanding request (yet)
                // Wait a little while to see if we return gracefully
                WaitForSingleObject(m_hThread, 1000);
            }

            // Wait for the thread to terminate gracefully after potential IO request abort
            WaitForSingleObject(m_hThread, INFINITE);

            DWORD exitCode = 0;
            GetExitCodeThread(m_hThread, &exitCode);

            return static_cast<int>(exitCode);
        }
        else
        {
            // Wait a little while to see if we return gracefully
            if (WaitForSingleObject(m_hThread, 1000) == WAIT_OBJECT_0)
            {
                DWORD exitCode = 0;
                GetExitCodeThread(m_hThread, &exitCode);
                return static_cast<int>(exitCode);
            }

            // This OS doesn't support IO cancellation (i.e. XP) so we simply have to terminate the thread.
            // Let's at least make sure we're not executing any callback code by checking the critical section.
            EnterCriticalSection(&m_CS);
            TerminateThread(m_hThread, 0);
            LeaveCriticalSection(&m_CS);

            return 0;
        }
    }

    return 0;
}

bool CrashHandlerCommPipe::ReadCompleteChunk(HANDLE hFile, void* data, size_t dataSize)
{
    const char* bytes = reinterpret_cast<const char*>(data);
    size_t read = 0;

    while (read < dataSize)
    {
#if UNITY_64
        DWORD chunkSize = static_cast<DWORD>(std::min(dataSize - read, 0xffffffffull));
#else
        DWORD chunkSize = static_cast<DWORD>(dataSize - read);
#endif

        DWORD thisRead = 0;
        bool ok = ReadFile(hFile, (LPVOID)bytes, chunkSize, &thisRead, nullptr);
        if (!ok)
            return false;

        read += thisRead;
        bytes += thisRead;
    }

    return true;
}

bool CrashHandlerCommPipe::ReadCompleteMessage(HANDLE hFile, std::vector<char>& message)
{
    // Read the size of the message
    size_t messageLength = 0;
    if (!ReadCompleteChunk(hFile, &messageLength, sizeof(messageLength)))
        return false;

    // If the message is obviously garbage, discard it
    if (messageLength > 0x7fffffffull)
        return false;

    // Allocate space for the entire message
    message.resize(messageLength);

    // If the message is empty, handle that specially
    if (messageLength == 0)
    {
        message.push_back('\0');
        return true;
    }

    // Read the message
    if (!ReadCompleteChunk(hFile, message.data(), messageLength))
        return false;

    // Check for final null terminator
    if (message[message.size() - 1] != '\0')
        message.push_back('\0');

    return true;
}

DWORD CALLBACK CrashHandlerCommPipe::CommThreadProc(CrashHandlerCommPipe* pipe)
{
    std::vector<char> buf;

    while (!pipe->m_Abort)
    {
        buf.resize(0);
        bool readOK = ReadCompleteMessage(pipe->m_hPipe, buf);
        if (buf.size() > 0)
        {
            EnterCriticalSection(&pipe->m_CS);
            pipe->m_Handler->ProcessMessage(buf.data(), buf.size());
            LeaveCriticalSection(&pipe->m_CS);
        }
        else if (!readOK)
        {
            int err = GetLastError();
            if (err == ERROR_BROKEN_PIPE ||
                err == ERROR_OPERATION_ABORTED ||
                err == ERROR_NO_MORE_ITEMS)
            {
                // Pipe was closed/canceled. No worries.
                return 0;
            }

            return err;
        }
    }

    return 0;
}
