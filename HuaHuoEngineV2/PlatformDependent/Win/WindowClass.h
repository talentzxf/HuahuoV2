#pragma once

#include "Runtime/Utilities/NonCopyable.h"

class WindowTraits
{
public:
    //inline static LPCWSTR GetClassName() { return NULL; }
    inline static UINT GetStyle() { return 0; }
    inline static int GetClassExtra() { return 0; }
    inline static int GetWindowExtra() { return 0; }
    inline static WNDPROC GetWindowProc() { return DefWindowProcW; }
    inline static HICON GetIcon() { return NULL; }
    inline static HCURSOR GetCursor() { return NULL; }
    inline static HBRUSH GetBackground() { return NULL; }
    inline static LPCWSTR GetMenuName() { return NULL; }
    inline static HICON GetSmallIcon() { return NULL; }
};


template<typename TWindowTraits>
class WindowClass : private NonCopyable
{
private:
    ATOM atom;

public:
    WindowClass()
        : atom(Register())
    {}

    ~WindowClass()
    {
        Unregister();
    }

    ATOM GetAtom() const
    {
        return atom;
    }

private:
    static ATOM Register()
    {
        WNDCLASSEXW wcex;

        wcex.cbSize = sizeof(wcex);
        wcex.style = TWindowTraits::GetStyle();
        wcex.lpfnWndProc = TWindowTraits::GetWindowProc();
        wcex.cbClsExtra = TWindowTraits::GetClassExtra();
        wcex.cbWndExtra = TWindowTraits::GetWindowExtra();
        wcex.hInstance = winutils::GetCurrentModuleHandle();
        wcex.hIcon = TWindowTraits::GetIcon();
        wcex.hCursor = TWindowTraits::GetCursor();
        wcex.hbrBackground = TWindowTraits::GetBackground();
        wcex.lpszMenuName = TWindowTraits::GetMenuName();
        wcex.lpszClassName = TWindowTraits::GetClassName();
        wcex.hIconSm = TWindowTraits::GetSmallIcon();

        ATOM atom = RegisterClassExW(&wcex);
        Assert(0 != atom);

        return atom;
    }

    void Unregister()
    {
        if (0 != this->atom)
        {
            BOOL const result = UnregisterClassW(TWindowTraits::GetClassName(), winutils::GetCurrentModuleHandle());
            Assert(FALSE != result);

            this->atom = 0;
        }
    }
};
