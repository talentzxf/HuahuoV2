#pragma once

#include "StackWalker.h"

#include <winnt.h>
#include "Runtime/Core/Format/Format.h"
#include "Runtime/Utilities/PathNameUtility.h"
#include "Runtime/Logging/LogAssert.h"


class MyStackWalker : public StackWalker
{
    int m_SkipFrames;
protected:
    MyStackWalker()
    {
        m_SkipFrames = 0;
    }

    virtual void OnSymInit(LPCSTR szSearchPath, DWORD symOptions, LPCSTR szUserName) {}
    virtual void OnLoadModule(LPCSTR img, LPCSTR mod, DWORD64 baseAddr, DWORD size, DWORD result, LPCSTR symType, LPCSTR pdbName, ULONGLONG fileVersion) {}
    virtual void OnCallstackEntry(CallstackEntryType eType, CallstackEntry &entry)
    {
        if (m_SkipFrames > 0)
        {
            m_SkipFrames--;
            return;
        }
        if ((eType != lastEntry) && (entry.offset != 0))
        {
            const char* moduleName = entry.moduleName[0] == 0 ? "(<unknown>)" : entry.moduleName;
            ConvertSeparatorsToUnity(entry.lineFileName);
            if (entry.lineFileName[0] != 0)
                OnOutput(core::Format("0x{0:p} ({1}) [{2}:{3}] {4} \n", (uintptr_t)entry.offset, moduleName, GetLastPathNameComponent(entry.lineFileName), entry.lineNumber, entry.undFullName).c_str());
            else
                OnOutput(core::Format("0x{0:p} ({1}) {2}\n", (uintptr_t)entry.offset, moduleName, entry.undFullName).c_str());
        }
    }

    virtual void OnDbgHelpErr(LPCSTR szFuncName, DWORD gle, DWORD64 addr) {}
public:
    void SkipFrames(int skipFrames)
    {
        m_SkipFrames = skipFrames;
    }
};

class StringStackWalker : public MyStackWalker
{
public:
    StringStackWalker() : result(NULL) {}

    void SetOutputString(core::string &_result) { result = &_result; }
    void ClearOutputString() { result = NULL; }
protected:
    core::string *result;

    virtual void OnOutput(LPCSTR szTest)
    {
        if (result)
            *result += szTest;
    }
};

class BufferStackWalker : public MyStackWalker
{
public:
    BufferStackWalker() : maxSize(0), buffer(NULL) {}

    void SetOutputBuffer(char *_buffer, int _maxSize)
    {
        buffer = _buffer;
        maxSize = _maxSize;
    }

    void ClearOutputBuffer() { buffer = NULL; }
protected:
    char *buffer;
    int maxSize;

    virtual void OnOutput(LPCSTR szTest)
    {
        if (buffer == NULL)
            return;

        while (maxSize > 1 && *szTest != '\0')
        {
            *(buffer++) = *(szTest++);
            maxSize--;
        }
        *buffer = '\0';
    }
};
