#pragma once

#include "Include/C/Baselib_FileIO.h"

BASELIB_C_INTERFACE
{
    Baselib_FileIO_EventQueue Baselib_FileIO_EventQueue_Create(void)
    {
        return platform::Baselib_FileIO_EventQueue_Create();
    }

    void Baselib_FileIO_EventQueue_Free(Baselib_FileIO_EventQueue eq)
    {
        if (eq.handle == Baselib_FileIO_EventQueue_Invalid.handle)
            return;
        return platform::Baselib_FileIO_EventQueue_Free(eq);
    }

    uint64_t Baselib_FileIO_EventQueue_Dequeue(
        Baselib_FileIO_EventQueue        eq,
        Baselib_FileIO_EventQueue_Result results[],
        uint64_t                         count,
        uint32_t                         timeoutInMilliseconds
    )
    {
        if (eq.handle == Baselib_FileIO_EventQueue_Invalid.handle)
            return 0;
        if (results == nullptr)
            return 0;
        if (count == 0)
            return 0;
        return platform::Baselib_FileIO_EventQueue_Dequeue(eq, results, count, timeoutInMilliseconds);
    }

    Baselib_FileIO_File Baselib_FileIO_File_Open(
        Baselib_FileIO_EventQueue eq,
        const char*               pathname,
        uint64_t                  userdata,
        Baselib_FileIO_Priority   priority
    )
    {
        if (eq.handle == Baselib_FileIO_EventQueue_Invalid.handle)
            return Baselib_FileIO_File_Invalid;
        if (pathname == nullptr)
            return Baselib_FileIO_File_Invalid;
        return platform::Baselib_FileIO_File_Open(eq, pathname, userdata, priority);
    }

    void Baselib_FileIO_File_Read(
        Baselib_FileIO_File        file,
        Baselib_FileIO_ReadRequest requests[],
        uint64_t                   count,
        uint64_t                   userdata,
        Baselib_FileIO_Priority    priority
    )
    {
        if (file.handle == Baselib_FileIO_File_Invalid.handle)
            return;
        if (requests == nullptr)
            return;
        if (count == 0)
            return;
        return platform::Baselib_FileIO_File_Read(file, requests, count, userdata, priority);
    }

    void Baselib_FileIO_File_Close(
        Baselib_FileIO_File file
    )
    {
        if (file.handle == Baselib_FileIO_File_Invalid.handle)
            return;
        return platform::Baselib_FileIO_File_Close(file);
    }
}
