#pragma once

#include <windef.h>
#include <fileapi.h>
#include <handleapi.h>
#include <libloaderapi.h>
#include <imm.h>

namespace win
{
    template<typename THandle, BOOL(WINAPI *close)(THandle), THandle null>
    class Handle
    {
    private:
        THandle m_Handle;

    public:
        inline Handle() : m_Handle(null) {}
        inline Handle(THandle handle) : m_Handle(handle) {}

        inline ~Handle()
        {
            if (null != m_Handle)
            {
                BOOL const result = close(m_Handle);
                Assert(FALSE != result);
            }
        }

        inline operator THandle() const { return m_Handle; }
        inline bool IsOpen() const { return (null != m_Handle); }

        inline void Close()
        {
            if (null != m_Handle)
            {
                BOOL const result = close(m_Handle);
                Assert(FALSE != result);

                m_Handle = null;
            }
        }

        inline Handle const& Attach(THandle handle)
        {
            if (null != m_Handle)
            {
                BOOL const result = close(m_Handle);
                Assert(FALSE != result);
            }

            m_Handle = handle;

            return *this;
        }

        inline THandle Detach()
        {
            THandle handle = m_Handle;
            m_Handle = null;
            return handle;
        }

    private:
        inline Handle(Handle const&) {}
        inline Handle const& operator=(Handle const&) { return *this; }
    };


    typedef Handle<HANDLE, CloseHandle, NULL> EventHandle;
    typedef Handle<HANDLE, CloseHandle, NULL> TokenHandle;
    typedef Handle<HANDLE, CloseHandle, NULL> ThreadHandle;
    typedef Handle<HANDLE, CloseHandle, INVALID_HANDLE_VALUE> FileHandle;
    typedef Handle<HANDLE, FindClose, INVALID_HANDLE_VALUE> FindHandle;
    typedef Handle<HMODULE, FreeLibrary, NULL> LibraryHandle;


    #if !PLATFORM_WINRT

    template<typename THandleEx, typename THandle, BOOL(WINAPI *close)(THandleEx, THandle), THandle null>
    class HandleEx
    {
    private:
        THandleEx m_HandleEx;
        THandle m_Handle;

    public:
        inline HandleEx() : m_Handle(null) {}
        inline HandleEx(THandleEx handleEx, THandle handle) : m_HandleEx(handleEx), m_Handle(handle) {}

        inline ~HandleEx()
        {
            if (null != m_Handle)
            {
                BOOL const result = close(m_HandleEx, m_Handle);
                Assert(FALSE != result);
            }
        }

        inline operator THandle() const { return m_Handle; }
        inline bool IsOpen() const { return (null != m_Handle); }

        inline void Close()
        {
            if (null != m_Handle)
            {
                BOOL const result = close(m_HandleEx, m_Handle);
                Assert(FALSE != result);

                m_Handle = null;
            }
        }

        inline HandleEx const& Attach(THandleEx handleEx, THandle handle)
        {
            if (null != m_Handle)
            {
                BOOL const result = close(handleEx, m_Handle);
                Assert(FALSE != result);
            }

            m_HandleEx = handleEx;
            m_Handle = handle;

            return *this;
        }

        inline THandle Detach()
        {
            THandle handle = m_Handle;
            m_Handle = null;
            return handle;
        }

    private:
        inline HandleEx(HandleEx const&) {}
        inline HandleEx const& operator=(HandleEx const&) { return *this; }
    };


#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
    typedef HandleEx<HWND, HIMC, ImmReleaseContext, NULL> ImmGetContextHandle;
#endif //WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

    #endif
}
