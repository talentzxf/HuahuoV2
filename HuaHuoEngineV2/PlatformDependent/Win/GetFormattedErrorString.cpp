#include "UnityPrefix.h"
#include "PlatformDependent/Win/GetFormattedErrorString.h"

#include <windef.h>
#include <winbase.h>

#if !defined(FORMAT_MESSAGE_ALLOCATE_BUFFER)
// Admittedly a hack but oh, well
#define FORMAT_MESSAGE_ALLOCATE_BUFFER 0x00000100
#endif

namespace detail
{
    struct scoped_local_free
    {
        scoped_local_free(HLOCAL mem) : m_locmemory(mem) {}
        ~scoped_local_free()
        {
        #if PLATFORM_WINRT || PLATFORM_XBOXONE
            // This is not strictly correct, but it works in Metro
            ::HeapFree(GetProcessHeap(), 0, m_locmemory);
        #else
            ::LocalFree(m_locmemory);
        #endif
        }

        template<typename Result>
        Result get() const
        {
            return static_cast<Result>(m_locmemory);
        }

    private:
        HLOCAL m_locmemory;
    };
} // namespace detail

// Formats a message string using the specified message and variable list of arguments.
core::wstring GetFormattedString(LPCWSTR format, ...)
{
    LPWSTR pBuffer = NULL;
    va_list args = NULL;
    va_start(args, format);

    ::FormatMessageW(
        FORMAT_MESSAGE_FROM_STRING |
        FORMAT_MESSAGE_ALLOCATE_BUFFER,
        format,
        0,
        0,
        (LPWSTR)&pBuffer,
        0,
        &args);

    va_end(args);

    detail::scoped_local_free dealloc(pBuffer);
    return TempWString(dealloc.get<LPWSTR>());
}

core::wstring GetHResultErrorMessageW(HRESULT hrError)
{
    typedef core::wstring::pointer pointer_type;

    DWORD const dw = static_cast<DWORD>(hrError);

    // Retrieve the system error message for the last-error code
    pointer_type lpMsgBuf = 0;
    ::FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (pointer_type) & lpMsgBuf,
        0, NULL);

    detail::scoped_local_free dealloc(lpMsgBuf);
    return GetFormattedString(L"Operation has failed with error %1!#x!: %2!s!", dw, lpMsgBuf);
}

core::string GetHResultErrorMessage(HRESULT hrError)
{
    return WideToUtf8(GetHResultErrorMessageW(hrError));
}
