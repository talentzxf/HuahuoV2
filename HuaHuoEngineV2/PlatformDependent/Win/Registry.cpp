#include "UnityPrefix.h"

#include "Registry.h"
#include "WinUnicode.h"
#include <processenv.h>
#include <winreg.h>
#include <winerror.h>

#include <tchar.h>

#if UNITY_EDITOR && !UNITY_EXTERNAL_TOOL
#ifdef _UNICODE
#include <atlstr.h>
#endif
#endif

#pragma warning(disable:4530) // exception handler used

using core::string;
using std::vector;

namespace
{
    inline REGSAM internalGetRegistryViewFlag(registry::RegistryView view)
    {
        switch (view)
        {
            case registry::kRegistryView32:
                return KEY_WOW64_32KEY;
            case registry::kRegistryView64:
                return KEY_WOW64_64KEY;
            //case registry::kRegistryViewDefault:
            default:
                return 0;
        }
    }

    HKEY internalOpenKey(HKEY root, LPCWSTR path, registry::RegistryView view)
    {
        HKEY hkey;
        DWORD res = ::RegOpenKeyExW(root, path, 0, KEY_QUERY_VALUE | internalGetRegistryViewFlag(view), &hkey);
        if (res == ERROR_SUCCESS)
            return hkey;
        // if that failed, try reading from 64 bit registry (but only if default registry view was requested)
        if (view != registry::kRegistryViewDefault)
            return NULL;
        res = ::RegOpenKeyExW(root, path, 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &hkey);
        if (res == ERROR_SUCCESS)
            return hkey;
        return NULL;
    }

    static bool internalGetString(HKEY root, LPCWSTR path, LPCWSTR key, core::wstring& outVal, registry::RegistryView view)
    {
        HKEY hkey = internalOpenKey(root, path, view);
        if (!hkey)
            return false;

        // The key is opened. Query the value.
        WCHAR buffer[2048];

        DWORD dataType;
        DWORD dataLength = sizeof(buffer);
        DWORD res = ::RegQueryValueExW(hkey, key, NULL, &dataType, (LPBYTE)buffer, &dataLength);
        ::RegCloseKey(hkey);
        // In MULTI_SZ case only the first string will be returned
        if ((dataType == REG_SZ || dataType == REG_MULTI_SZ) && dataLength >= sizeof(WCHAR) && res == ERROR_SUCCESS)
        {
            const DWORD outLength = dataLength / sizeof(WCHAR) - 1;
            outVal.assign(buffer, buffer + outLength);
            return true;
        }
        if ((dataType == REG_EXPAND_SZ) && dataLength >= 1 && res == ERROR_SUCCESS)
        {
            // expand the string
            WCHAR exBuffer[ARRAYSIZE(buffer)];
            if (!::ExpandEnvironmentStringsW(buffer, exBuffer, ARRAYSIZE(exBuffer) - 1))
            {
                // just return the unexpanded string
                const DWORD outLength = dataLength / sizeof(WCHAR) - 1;
                outVal.assign(buffer, buffer + outLength);
                return true;
            }
            else
            {
                // return the expanded buffer
                outVal = exBuffer;
                return true;
            }
        }
        return false;
    }

    bool internalGetUInt32(HKEY root, LPCWSTR path, LPCWSTR key, unsigned int& outVal, registry::RegistryView view)
    {
        HKEY hkey = internalOpenKey(root, path, view);
        if (!hkey)
            return false;

        // The key is opened. Query the value.
        DWORD dataType;
        DWORD dataLength = 4;
        DWORD res = ::RegQueryValueExW(hkey, key, 0, &dataType, (LPBYTE)&outVal, &dataLength);
        ::RegCloseKey(hkey);
        if ((dataType == REG_BINARY || dataType == REG_DWORD) && dataLength == 4 && res == ERROR_SUCCESS)
            return true;
        else
            return false;
    }

    bool internalSetString(HKEY root, LPCWSTR path, LPCWSTR key, LPCWSTR val, registry::RegistryView view)
    {
        HKEY hkey;

        DWORD res = ::RegCreateKeyExW(root, path, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE | internalGetRegistryViewFlag(view), NULL, &hkey, NULL);
        if (res != ERROR_SUCCESS)
            return false;

        res = ::RegSetValueExW(hkey, key, 0, REG_SZ, (const BYTE*)val, static_cast<DWORD>(wcslen(val) * sizeof(WCHAR)));
        ::RegCloseKey(hkey);
        return res == ERROR_SUCCESS;
    }

    template<size_t size>
    inline bool internalStripPrefix(LPCWSTR& text, const WCHAR(&prefix)[size])
    {
        if (!_wcsnicmp(text, prefix, size - 1))
        {
            text += size - 1;
            return true;
        }
        return false;
    }

    HKEY internalGetRoot(LPCWSTR& path)
    {
        if (internalStripPrefix(path, L"HKEY_CURRENT_USER\\"))
            return HKEY_CURRENT_USER;
        if (internalStripPrefix(path, L"HKEY_LOCAL_MACHINE\\"))
            return HKEY_LOCAL_MACHINE;
        if (internalStripPrefix(path, L"HKEY_CLASSES_ROOT\\"))
            return HKEY_CLASSES_ROOT;
        return NULL;
    }

#if UNITY_EDITOR && !UNITY_EXTERNAL_TOOL

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 8191

    bool internalGetKeys(HKEY hKey, core::string_ref parentPath, dynamic_array<core::string>& outKeys, dynamic_array<core::string>& outValues)
    {
        TCHAR    achKey[MAX_KEY_LENGTH];// buffer for subkey name
        DWORD    cbName;               // size of name string
        TCHAR    achClass[MAX_PATH];// buffer for class name
        DWORD    cchClassName = MAX_PATH;// size of class string
        DWORD    cSubKeys = 0;         // number of subkeys
        DWORD    cbMaxSubKey;          // longest subkey size
        DWORD    cchMaxClass;          // longest class string
        DWORD    cValues;          // number of values for key
        DWORD    cchMaxValue;      // longest value name
        DWORD    cbMaxValueData;   // longest value data
        DWORD    cbSecurityDescriptor;// size of security descriptor
        FILETIME ftLastWriteTime;  // last write time

        DWORD i, rc;

#ifdef _UNICODE
        _tcscpy(achClass, CA2T(parentPath.c_str()));
        cchClassName = static_cast<DWORD>(_tcslen(achClass) + 1);
#else
        const size_t len = parentPath.copy(achClass, MAX_PATH);
        achClass[len] = '\0';
        cchClassName = strlen(achClass) + 1;
#endif

        rc = RegOpenKeyEx(hKey, achClass, 0, KEY_READ, &hKey);

        if (rc != ERROR_SUCCESS)
            return false;

        // query for count of children
        rc = RegQueryInfoKey(
            hKey,                // key handle
            achClass,            // buffer for class name
            &cchClassName,       // size of class string
            NULL,                // reserved
            &cSubKeys,           // number of subkeys
            &cbMaxSubKey,        // longest subkey size
            &cchMaxClass,        // longest class string
            &cValues,            // number of values for this key
            &cchMaxValue,        // longest value name
            &cbMaxValueData,     // longest value data
            &cbSecurityDescriptor,     // security descriptor
            &ftLastWriteTime);     // last write time

        if (rc != ERROR_SUCCESS)
        {
            RegCloseKey(hKey);
            return FALSE;
        }


        char achValue[MAX_VALUE_NAME];


        //Iterate child keys
        for (i = 0;
             i < cValues; i++)
        {
            //get child key name
            //dwNSize = dim(wszName);
            //dwCSize = dim(wszClass);
            DWORD cchValue = MAX_VALUE_NAME;
            cbName = MAX_KEY_LENGTH;
            achKey[0] = 0;
            achValue[0] = 0;
            rc = RegEnumValue(hKey, i,
                achKey,
                &cbName,
                NULL,
                NULL,
                reinterpret_cast<BYTE*>(achValue),
                &cchValue);


            if (rc != ERROR_SUCCESS)
            {
                continue;
            }

            outKeys.push_back(achKey);
            outValues.push_back(achValue);
        }

        RegCloseKey(hKey);
        return true;
    }

#endif
}  // anonymous namespace

core::string registry::getString(core::string_ref path, core::string_ref key, core::string_ref def, RegistryView view)
{
    TempWString widePath, wideKey, wideDef;
    ConvertUTF8ToWideString(path, widePath);
    ConvertUTF8ToWideString(key, wideKey);
    ConvertUTF8ToWideString(def, wideDef);
    const core::wstring wideRes = getString(widePath.c_str(), wideKey.c_str(), wideDef.c_str(), view);

    TempString res;
    ConvertWideToUTF8String(wideRes, res);
    return res;
}

core::wstring registry::getString(LPCWSTR path, LPCWSTR key, LPCWSTR def, RegistryView view)
{
    core::wstring res(kMemTempAlloc);
    const HKEY root = internalGetRoot(path);
    if (root != NULL)
    {
        if (internalGetString(root, path, key, res, view))
            return res;
    }
    else
    {
        if (internalGetString(HKEY_CURRENT_USER, path, key, res, view))
            return res;
        if (internalGetString(HKEY_LOCAL_MACHINE, path, key, res, view))
            return res;
        if (internalGetString(HKEY_CLASSES_ROOT, path, key, res, view))
            return res;
    }
    return TempWString(def == NULL ? L"" : def);
}

#if UNITY_EDITOR && !UNITY_EXTERNAL_TOOL
void registry::getKeyValues(core::string_ref key, dynamic_array<core::string>& outKeys, dynamic_array<core::string>& outValues)
{
    outKeys.resize_initialized(0);
    outValues.resize_initialized(0);
    internalGetKeys(HKEY_CURRENT_USER, key, outKeys, outValues);
}

#endif

unsigned int registry::getUInt32(core::string_ref path, core::string_ref key, unsigned int def, RegistryView view)
{
    TempWString widePath, wideKey;
    ConvertUTF8ToWideString(path, widePath);
    ConvertUTF8ToWideString(key, wideKey);
    return getUInt32(widePath.c_str(), wideKey.c_str(), def, view);
}

unsigned int registry::getUInt32(LPCWSTR path, LPCWSTR key, unsigned int def, RegistryView view)
{
    unsigned int res;
    const HKEY root = internalGetRoot(path);
    if (root != NULL)
    {
        if (internalGetUInt32(root, path, key, res, view))
            return res;
    }
    else
    {
        if (internalGetUInt32(HKEY_CURRENT_USER, path, key, res, view))
            return res;
        if (internalGetUInt32(HKEY_LOCAL_MACHINE, path, key, res, view))
            return res;
    }
    return def;
}

bool registry::setString(core::string_ref path, core::string_ref key, core::string_ref val, RegistryView view)
{
    TempWString widePath, wideKey, wideVal;
    ConvertUTF8ToWideString(path, widePath);
    ConvertUTF8ToWideString(key, wideKey);
    ConvertUTF8ToWideString(val, wideVal);
    return setString(widePath.c_str(), wideKey.c_str(), wideVal.c_str(), view);
}

bool registry::setString(LPCWSTR path, LPCWSTR key, LPCWSTR val, RegistryView view)
{
    return internalSetString(HKEY_CURRENT_USER, path, key, val, view);
}
