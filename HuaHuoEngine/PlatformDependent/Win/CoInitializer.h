#pragma once
#include <ObjBase.h>

struct CoInitializer
{
    CoInitializer()
    {
        hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    }

    ~CoInitializer()
    {
        if (SUCCEEDED(hr))
            CoUninitialize();
    }

    operator bool() const
    {
        return SUCCEEDED(hr);
    }

private:
    HRESULT hr;
};
