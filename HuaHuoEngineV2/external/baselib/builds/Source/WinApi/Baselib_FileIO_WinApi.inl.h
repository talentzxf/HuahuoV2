#pragma once

#include "Include/C/Baselib_FileIO.h"
#include "Source/Baselib_InlineImplementation.h"
#include "Source/WinApi/Baselib_ErrorState_WinApi.inl.h"
#include "Source/Baselib_Utilities.h"

#include "Source/Baselib_FileIO_AsyncEmulation.inl.h"

namespace WinApi
{
namespace detail
{
    class WinApiAsyncEmulation : public Baselib_FileIO_AsyncEmulation
    {
    public:
        void* operator new(size_t size)
        {
            BaselibAssert(size >= sizeof(WinApiAsyncEmulation));
            return Baselib_Memory_AlignedAllocate(sizeof(WinApiAsyncEmulation), alignof(WinApiAsyncEmulation));
        }

        void operator delete(void* memory)
        {
            Baselib_Memory_AlignedFree(memory);
        }

        WinApiAsyncEmulation() = default;
        virtual ~WinApiAsyncEmulation() = default;

        uintptr_t SyncOpen(const char* pathname, uint64_t* outFileSize, Baselib_ErrorState* errorState)
        {
            const DWORD fileAttributes = FILE_ATTRIBUTE_NORMAL;
            const DWORD fileFlags = 0;
            const DWORD desiredAccess = GENERIC_READ;
            const DWORD shareMode = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
            const DWORD creationDisposition = OPEN_EXISTING;

            // CreateFileEx is not available on UWP
            // but CreateFile2 is not available on Win7
            HANDLE fd = INVALID_HANDLE_VALUE;
        #if PLATFORM_WINRT
            CREATEFILE2_EXTENDED_PARAMETERS params;
            memset(&params, 0, sizeof(params));
            params.dwSize = sizeof(params);
            params.dwFileAttributes = fileAttributes;
            params.dwFileFlags = fileFlags;
            fd = CreateFile2(
                WinApi_StringConversions_UTF8ToUTF16(pathname).c_str(),
                desiredAccess,
                shareMode,
                creationDisposition,
                &params
            );
        #else
            fd = CreateFileW(
                WinApi_StringConversions_UTF8ToUTF16(pathname).c_str(),
                desiredAccess,
                shareMode,
                NULL,
                creationDisposition,
                fileAttributes | fileFlags,
                NULL
            );
        #endif

            if (fd == INVALID_HANDLE_VALUE)
            {
                Baselib_ErrorCode errorCode = Baselib_ErrorCode_IOError;
                switch (GetLastError())
                {
                    case ERROR_FILE_NOT_FOUND:
                        errorCode = Baselib_ErrorCode_InvalidPathname;
                        break;
                    case ERROR_ACCESS_DENIED:
                    {
                        auto fileAttributes = GetFileAttributesW(WinApi_StringConversions_UTF8ToUTF16(pathname).c_str());
                        if ((fileAttributes != INVALID_FILE_ATTRIBUTES) && (fileAttributes == FILE_ATTRIBUTE_DIRECTORY))
                            errorCode = Baselib_ErrorCode_InvalidPathname;
                        else
                            errorCode = Baselib_ErrorCode_RequestedAccessIsNotAllowed;
                        break;
                    }
                    default:
                        break;
                }
                errorState |= RaiseError(errorCode) | WithGetLastError();
                return reinterpret_cast<uintptr_t>(INVALID_HANDLE_VALUE);
            }

            LARGE_INTEGER size;
            if (GetFileSizeEx(fd, &size) == TRUE)
                *outFileSize = size.QuadPart;
            else
            {
                errorState |= RaiseError(Baselib_ErrorCode_IOError) | WithGetLastError();
                CloseHandle(fd); // result is ignored
                return reinterpret_cast<uintptr_t>(INVALID_HANDLE_VALUE);
            }

            return reinterpret_cast<uintptr_t>(fd);
        }

        uint32_t SyncRead(uintptr_t descriptor, uint64_t offset, void* buffer, uint32_t size, Baselib_ErrorState* errorState)
        {
            auto fd = reinterpret_cast<HANDLE>(descriptor);

            LARGE_INTEGER offsetLI = {};
            offsetLI.QuadPart = offset;

            OVERLAPPED overlapped = {};
            overlapped.Offset = offsetLI.LowPart;
            overlapped.OffsetHigh = offsetLI.HighPart;

            DWORD bytes = 0;
            if (ReadFile(
                fd,
                buffer,
                size,
                &bytes,
                &overlapped
                ) == FALSE)
            {
                errorState |= RaiseError(Baselib_ErrorCode_IOError) | WithGetLastError();
                return 0;
            }

            return bytes;
        }

        void SyncClose(uintptr_t descriptor, Baselib_ErrorState* errorState)
        {
            auto fd = reinterpret_cast<HANDLE>(descriptor);
            if (fd == INVALID_HANDLE_VALUE)
                return;
            if (CloseHandle(fd) == FALSE)
                errorState |= RaiseError(Baselib_ErrorCode_IOError) | WithGetLastError();
        }
    };
}

    BASELIB_INLINE_IMPL Baselib_FileIO_EventQueue Baselib_FileIO_EventQueue_Create(void)
    {
        return ::detail::AsBaselibHandle<Baselib_FileIO_EventQueue>(new detail::WinApiAsyncEmulation());
    }

    BASELIB_INLINE_IMPL void Baselib_FileIO_EventQueue_Free(Baselib_FileIO_EventQueue eq)
    {
        auto emulation = ::detail::AsNativeType<detail::WinApiAsyncEmulation*>(eq);
        delete emulation;
    }

    BASELIB_INLINE_IMPL uint64_t Baselib_FileIO_EventQueue_Dequeue(
        Baselib_FileIO_EventQueue        eq,
        Baselib_FileIO_EventQueue_Result results[],
        uint64_t                         count,
        uint32_t                         timeoutInMilliseconds
    )
    {
        auto emulation = ::detail::AsNativeType<detail::WinApiAsyncEmulation*>(eq);
        return emulation->Dequeue(results, count, timeoutInMilliseconds);
    }

    BASELIB_INLINE_IMPL Baselib_FileIO_File Baselib_FileIO_File_Open(
        Baselib_FileIO_EventQueue eq,
        const char*               pathname,
        uint64_t                  userdata,
        Baselib_FileIO_Priority   priority
    )
    {
        auto emulation = ::detail::AsNativeType<detail::WinApiAsyncEmulation*>(eq);
        auto file = emulation->AsyncOpen(pathname, userdata, priority);
        return {static_cast<void*>(file)};
    }

    BASELIB_INLINE_IMPL void Baselib_FileIO_File_Read(
        Baselib_FileIO_File        fileHandle,
        Baselib_FileIO_ReadRequest requests[],
        uint64_t                   count,
        uint64_t                   userdata,
        Baselib_FileIO_Priority    priority
    )
    {
        auto file = ::detail::AsNativeType<detail::WinApiAsyncEmulation::AsyncOperation*>(fileHandle);
        file->parent->AsyncRead(file, requests, count, userdata, priority);
    }

    BASELIB_INLINE_IMPL void Baselib_FileIO_File_Close(
        Baselib_FileIO_File fileHandle
    )
    {
        auto file = ::detail::AsNativeType<detail::WinApiAsyncEmulation::AsyncOperation*>(fileHandle);
        file->parent->AsyncClose(file);
    }
}
