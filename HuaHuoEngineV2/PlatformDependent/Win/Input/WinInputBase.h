#pragma once

#include <windef.h>

namespace win
{
    struct __declspec(novtable)IInputBase
    {
        virtual ~IInputBase() {}

        virtual bool Open(HWND window) = 0;
        virtual void Close(void) = 0;

        virtual bool Activate(bool active) = 0;
        virtual bool ToggleFullscreen(bool fullscreen, HWND window) = 0;

        virtual LRESULT OnMessage(HWND window, UINT message, WPARAM wParam, LPARAM lParam, BOOL& handled) = 0;
    };

    void ResetMouseCapture();
    void SetMouseCapture(HWND window);
    void ReleaseMouseCapture();
}
