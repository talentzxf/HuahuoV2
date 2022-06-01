#pragma once

#include "Include/C/Baselib_FileIO.h"
#include "Source/Baselib_InlineImplementation.h"
#include "Source/Posix/ErrorStateBuilder_PosixApi.inl.h"
#include "Source/Baselib_Utilities.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "Source/Baselib_FileIO_AsyncEmulation.inl.h"

namespace PosixApi
{
namespace detail
{
    class PosixAsyncEmulation : public Baselib_FileIO_AsyncEmulation
    {
    public:
        void* operator new(size_t size)
        {
            BaselibAssert(size >= sizeof(PosixAsyncEmulation));
            return Baselib_Memory_AlignedAllocate(sizeof(PosixAsyncEmulation), alignof(PosixAsyncEmulation));
        }

        void operator delete(void* memory)
        {
            Baselib_Memory_AlignedFree(memory);
        }

        PosixAsyncEmulation() = default;
        virtual ~PosixAsyncEmulation() = default;

        uintptr_t SyncOpen(const char* pathname, uint64_t* outFileSize, Baselib_ErrorState* errorState)
        {
            int fd = open(pathname, O_RDONLY);
            if (fd == -1)
            {
                if (errno == ENOENT)
                    errorState |= RaiseError(Baselib_ErrorCode_InvalidPathname) | WithErrno(errno);
                else if (errno == EACCES)
                    errorState |= RaiseError(Baselib_ErrorCode_RequestedAccessIsNotAllowed) | WithErrno(errno);
                else
                    errorState |= RaiseError(Baselib_ErrorCode_IOError) | WithErrno(errno);
                return 0;
            }

            // fstat on descriptor is cheap compared to stat on pathname
            struct stat stat;
            if (fstat(fd, &stat) == -1)
            {
                errorState |= RaiseError(Baselib_ErrorCode_IOError) | WithErrno(errno);
                return 0;
            }

            // check for a directory is for free here
            if (S_ISDIR(stat.st_mode))
            {
                close(fd); // error is ignored
                errorState |= RaiseError(Baselib_ErrorCode_InvalidPathname) | WithErrno(errno);
                return 0;
            }

            *outFileSize = stat.st_size;
            return fd;
        }

        uint32_t SyncRead(uintptr_t descriptor, uint64_t offset, void* buffer, uint32_t size, Baselib_ErrorState* errorState)
        {
            auto bytes = pread(descriptor, buffer, size, offset);
            if (bytes == -1)
            {
                errorState |= RaiseError(Baselib_ErrorCode_IOError) | WithErrno(errno);
                return 0;
            }
            else
                return bytes;
        }

        void SyncClose(uintptr_t descriptor, Baselib_ErrorState* errorState)
        {
            if (close(descriptor) == -1)
                errorState |= RaiseError(Baselib_ErrorCode_IOError) | WithErrno(errno);
        }
    };
}

    BASELIB_INLINE_IMPL Baselib_FileIO_EventQueue Baselib_FileIO_EventQueue_Create(void)
    {
        return ::detail::AsBaselibHandle<Baselib_FileIO_EventQueue>(new detail::PosixAsyncEmulation());
    }

    BASELIB_INLINE_IMPL void Baselib_FileIO_EventQueue_Free(Baselib_FileIO_EventQueue eq)
    {
        auto emulation = ::detail::AsNativeType<detail::PosixAsyncEmulation*>(eq);
        delete emulation;
    }

    BASELIB_INLINE_IMPL uint64_t Baselib_FileIO_EventQueue_Dequeue(
        Baselib_FileIO_EventQueue        eq,
        Baselib_FileIO_EventQueue_Result results[],
        uint64_t                         count,
        uint32_t                         timeoutInMilliseconds
    )
    {
        auto emulation = ::detail::AsNativeType<detail::PosixAsyncEmulation*>(eq);
        return emulation->Dequeue(results, count, timeoutInMilliseconds);
    }

    BASELIB_INLINE_IMPL Baselib_FileIO_File Baselib_FileIO_File_Open(
        Baselib_FileIO_EventQueue eq,
        const char*               pathname,
        uint64_t                  userdata,
        Baselib_FileIO_Priority   priority
    )
    {
        auto emulation = ::detail::AsNativeType<detail::PosixAsyncEmulation*>(eq);
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
        auto file = ::detail::AsNativeType<detail::PosixAsyncEmulation::AsyncOperation*>(fileHandle);
        file->parent->AsyncRead(file, requests, count, userdata, priority);
    }

    BASELIB_INLINE_IMPL void Baselib_FileIO_File_Close(
        Baselib_FileIO_File fileHandle
    )
    {
        auto file = ::detail::AsNativeType<detail::PosixAsyncEmulation::AsyncOperation*>(fileHandle);
        file->parent->AsyncClose(file);
    }
}
