#include "UnityPrefix.h"
#include "Runtime/Utilities/CopyPaste.h"
#include "WinUnicode.h"
#include <windows.h>

core::string GetCopyBuffer()
{
    core::string result;

    if (!IsClipboardFormatAvailable(CF_UNICODETEXT))
        return result;
    if (!OpenClipboard(NULL))
        return result;

    HGLOBAL bufferHandle = GetClipboardData(CF_UNICODETEXT);
    if (bufferHandle != NULL)
    {
        wchar_t* buffer = reinterpret_cast<wchar_t*>(GlobalLock(bufferHandle));
        if (buffer != NULL)
        {
            // convert from wide string to UTF8
            size_t utf8size = wcslen(buffer) * 4;
            char* utf8 = new char[utf8size + 1];
            WideCharToMultiByte(CP_UTF8, 0, buffer, -1, utf8, utf8size + 1, NULL, NULL);
            result = utf8;
            delete[] utf8;

            GlobalUnlock(bufferHandle);
        }
    }
    CloseClipboard();

    return result;
}

void SetCopyBuffer(const core::string &utf8string)
{
    // Open and empty clipboard
    if (!OpenClipboard(NULL))
        return;
    EmptyClipboard();

    // Allocate buffer
    size_t bufferSize = utf8string.size() + 1;
    HGLOBAL bufferHandle = GlobalAlloc(GMEM_MOVEABLE, bufferSize * sizeof(wchar_t));
    if (bufferHandle == NULL)
    {
        CloseClipboard();
        return;
    }

    // Copy text into buffer
    wchar_t* buffer = reinterpret_cast<wchar_t*>(GlobalLock(bufferHandle));
    UTF8ToWide(utf8string.c_str(), buffer, bufferSize);
    GlobalUnlock(bufferHandle);

    // Put it on the clipboard
    SetClipboardData(CF_UNICODETEXT, bufferHandle);
    CloseClipboard();
}
