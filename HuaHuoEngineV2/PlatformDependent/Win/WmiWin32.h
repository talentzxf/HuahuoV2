#pragma once

#include "ComPtr.h"

#include <comdef.h>
#include <wbemidl.h>

#include "WinUnicode.h"
#include "Runtime/Utilities/dynamic_array.h"

#pragma comment(lib, "wbemuuid.lib")

class WmiWin32
{
private:
    win::ComPtr<IWbemServices> m_Services;

public:
    inline ~WmiWin32()
    {
        Close();
    }

    win::ComPtr<IWbemServices> GetServices() { return m_Services; }

    bool Open(const BSTR strNetworkResource = _bstr_t(L"root\\cimv2"))
    {
        HRESULT hr;
        win::ComPtr<IWbemLocator> locator;

        // instantiate locator

        hr = CoCreateInstance(CLSID_WbemLocator, NULL, CLSCTX_ALL, __uuidof(IWbemLocator), &locator);
        Assert(SUCCEEDED(hr));

        if (FAILED(hr))
        {
            goto error;
        }

        // connect to wmi

        hr = locator->ConnectServer(strNetworkResource, NULL, NULL, NULL, 0, NULL, NULL, &m_Services);
        Assert(SUCCEEDED(hr));

        if (FAILED(hr))
        {
            goto error;
        }

        // set security levels for proxy

        hr = CoSetProxyBlanket(m_Services, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
        Assert(SUCCEEDED(hr));

        if (FAILED(hr))
        {
            goto error;
        }

        // done

        return true;

        // failed

    error:

        Close();
        return false;
    }

    inline void Close()
    {
        m_Services.Free();
    }

    core::string GetProperties(LPCWSTR className, LPCWSTR propertyName)
    {
        HRESULT hr;

        // get class objects

        win::ComPtr<IEnumWbemClassObject> objects;

        hr = m_Services->CreateInstanceEnum(_bstr_t(className), (WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY), NULL, &objects);
        Assert(SUCCEEDED(hr));

        if (FAILED(hr))
        {
            return core::string(kMemTempAlloc);
        }

        // get values

        dynamic_array<core::string> values(kMemTempAlloc);

        for (;;)
        {
            // get next class object

            win::ComPtr<IWbemClassObject> object;
            ULONG returned;

            hr = objects->Next(WBEM_INFINITE, 1, &object, &returned);
            Assert(SUCCEEDED(hr));

            if (FAILED(hr))
            {
                return core::string();
            }

            if (WBEM_S_NO_ERROR != hr)
            {
                break;
            }

            Assert(1 == returned);

            // append value if not empty

            core::string const value = GetString(object, propertyName);

            if (!value.empty())
            {
                values.push_back(value);
            }
        }

        // sort values

        std::sort(values.begin(), values.end());

        // concatenate values

        core::string result(kMemTempAlloc);

        for (dynamic_array<core::string>::const_iterator it = values.begin(); it != values.end(); ++it)
        {
            result += *it;
        }

        return result;
    }

    static core::string GetString(IWbemClassObject* object, LPCWSTR name)
    {
        Assert(NULL != object);
        Assert(NULL != name);

        HRESULT hr;

        // get value

        _variant_t value;

        hr = object->Get(name, 0, &value, NULL, NULL);
        Assert(SUCCEEDED(hr));

        if (FAILED(hr))
        {
            return TempString();
        }

        // make sure it's not null

        if (VT_NULL == V_VT(&value))
        {
            return TempString();
        }

        // convert value to bstr

        hr = VariantChangeType(&value, &value, 0, VT_BSTR);
        Assert(SUCCEEDED(hr));

        if (FAILED(hr))
        {
            return TempString();
        }

        // convert value to utf8

        TempString valueUtf8;
        ConvertWideToUTF8String(V_BSTR(&value), valueUtf8);

        // done

        return TempString(core::Trim(valueUtf8));
    }
};
