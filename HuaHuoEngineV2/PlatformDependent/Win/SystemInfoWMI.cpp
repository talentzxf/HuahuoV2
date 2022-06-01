#include "UnityPrefix.h"
#include "WmiWin32.h"
#pragma warning(disable:4530) // exception handler used

#if UNITY_EXTERNAL_TOOL
// Integer typedefs, needed for InstanceID, Word.h, etc. Not sure exactly why we don't just include UnityPrefix.h above, but changing it now has hard-to-predict consequences.
// TODO: Review this whole file and see if we can just include UnityPrefix.h above safely
typedef signed short SInt16;
typedef unsigned short UInt16;
typedef unsigned char UInt8;
typedef signed char SInt8;
typedef signed int SInt32;
typedef unsigned int UInt32;
typedef unsigned long long UInt64;
typedef signed long long SInt64;
#endif

#define PLATFORM_WIN 1
#include "PlatformDependent/Win/BStrHolder.h"
#include "Runtime/Misc/SystemInfo.h"
#include "Runtime/Logging/LogAssert.h"
#include "Runtime/Utilities/vector_map.h"

#include "Modules/TLS/Public/TLS.h"

namespace systeminfo
{
    typedef UNITY_VECTOR (kMemTempAlloc, core::string) MacAddressVector_t;
    MacAddressVector_t GetMacAddressesWmi(bool connectorPresent, bool returnOnFirstAddress);

    core::string InsertColonEveryTwoChars(const core::string& macAddr)
    {
        core::string macAddrWithColons(kMemTempAlloc);
        for (int i = 0; i < macAddr.length(); i++)
        {
            macAddrWithColons += macAddr[i];
            if (i % 2 && i < (macAddr.length() - 1))
                macAddrWithColons += ':';
        }
        return macAddrWithColons;
    }
}

systeminfo::MacAddressVector_t systeminfo::GetMacAddressesWmi(bool excludeNonPCIAdapters, bool returnOnFirstAddress)
{
    MacAddressVector_t mac_addresses;
    WmiWin32 wmi;
    bool const wmiOpened = wmi.Open(_bstr_t(L"root\\StandardCimv2"));
    if (!wmiOpened)
    {
        // Printing these errors is handled in the WmiWin32 class
        return mac_addresses;
    }

    win::ComPtr<IEnumWbemClassObject> Enumerator;
    core::wstring queryStr(kMemTempAlloc);
    queryStr =
        L"SELECT Name,InterfaceDescription,PermanentAddress,NetworkAddresses,PNPDeviceID"
        L"    FROM MSFT_NetAdapter";
    HRESULT hr = wmi.GetServices()->ExecQuery(L"WQL", (_bstr_t)queryStr.c_str(), WBEM_FLAG_FORWARD_ONLY, NULL, &Enumerator);
    if (FAILED(hr))
    {
        ErrorStringMsg("Query '%S' failed. Error code = 0x%08x\n", L"MSFT_NetAdapter", hr);
        return mac_addresses;
    }

    ULONG retcnt = 0;
    core::string name(kMemTempAlloc);
    core::string ifDesc(kMemTempAlloc);
    core::string macAddr(kMemTempAlloc);
    core::string PNPDeviceID(kMemTempAlloc);
    win::ComPtr<IWbemClassObject> netAdapter;
    while (S_OK == Enumerator->Next(WBEM_INFINITE, 1, &netAdapter, &retcnt))
    {
        name = WmiWin32::GetString(netAdapter.Get(), L"Name");
        ifDesc = WmiWin32::GetString(netAdapter.Get(), L"InterfaceDescription");
        macAddr = WmiWin32::GetString(netAdapter.Get(), L"PermanentAddress");
        ToLowerInplace(macAddr);
        macAddr = InsertColonEveryTwoChars(macAddr);

        PNPDeviceID = WmiWin32::GetString(netAdapter.Get(), L"PNPDeviceID");

        _variant_t varNetworkAddresses;
        hr = netAdapter->Get(L"NetworkAddresses", 0, &varNetworkAddresses, NULL, NULL);
        if (!FAILED(hr))
        {
            // NetworkAddresses[0] supercedes PermanentAddress when found (always?)
            SAFEARRAY* pSA = varNetworkAddresses.parray;
            BStrHolder bstr;
            LONG Index = 0;
            hr = SafeArrayGetElement(pSA, &Index, &bstr);
            if (!FAILED(hr))
            {
                core::string macAddr2(kMemTempAlloc);
                ConvertWideToUTF8String(bstr, macAddr2);
                ToLowerInplace(macAddr2);
                macAddr = InsertColonEveryTwoChars(macAddr2);
            }
        }

        if (excludeNonPCIAdapters && PNPDeviceID.substr(0, 3) != "PCI")
            continue;

        if (!macAddr.empty())
        {
            mac_addresses.push_back(macAddr);
            if (returnOnFirstAddress)
                return mac_addresses;
        }
    }
    return mac_addresses;
}

#if defined(UNITY_EXTERNAL_TOOL) && !UNITY_EXTERNAL_TOOL

core::string systeminfo::GetBIOSIdentifier()
{
    core::string value(kMemTempAlloc);
    WmiWin32 wmi;

    bool const wmiOpened = wmi.Open();
    Assert(wmiOpened);

    if (wmiOpened)
        value = wmi.GetProperties(L"Win32_BIOS", L"SerialNumber");

    return value;
}

core::string GetDeviceIdFallback()
{
    // open registry key

    static LPCWSTR path = L"Software\\Unity Technologies";
    static LPCWSTR valueName = L"DeviceId";
    LSTATUS status;
    HKEY key;

    status = RegCreateKeyExW(HKEY_CURRENT_USER, path, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ, NULL, &key, NULL);
    Assert(ERROR_SUCCESS == status);

    if (ERROR_SUCCESS != status)
    {
        goto error;
    }

    // get id value

    DWORD type;
    WCHAR id[128];
    DWORD size = ARRAYSIZE(id);

    status = RegQueryValueExW(key, valueName, 0, &type, reinterpret_cast<LPBYTE>(id), &size);

    if (ERROR_SUCCESS != status)
    {
        Assert(ERROR_FILE_NOT_FOUND == status);

        // reopen registry key with write permission

        status = RegCloseKey(key);
        Assert(ERROR_SUCCESS == status);

        status = RegOpenKeyExW(HKEY_CURRENT_USER, path, 0, KEY_WRITE, &key);
        Assert(ERROR_SUCCESS == status);

        if (ERROR_SUCCESS != status)
        {
            goto error;
        }

        // generate uuid

        GUID guid;

        HRESULT const hr = CoCreateGuid(&guid);
        Assert(SUCCEEDED(hr));

        if (FAILED(hr))
        {
            goto error;
        }

        // convert uuid to string

        int const length = StringFromGUID2(guid, id, ARRAYSIZE(id));
        Assert(0 != length);

        if (0 == length)
        {
            goto error;
        }

        // store value

        status = RegSetValueExW(key, valueName, 0, REG_SZ, reinterpret_cast<BYTE const*>(id), (length * sizeof(WCHAR)));
        Assert(ERROR_SUCCESS == status);

        if (ERROR_SUCCESS != status)
        {
            goto error;
        }
    }

    // close registry key

    status = RegCloseKey(key);
    Assert(ERROR_SUCCESS == status);

    // convert wide value to utf8

    {
        core::string idUtf8(kMemTempAlloc);
        ConvertWideToUTF8String(id, idUtf8);
        if (!idUtf8.empty())
        {
            return idUtf8;
        }
    }

    // failed

error:

    return core::string("{00000000-0000-0000-0000-000000000000}", kMemTempAlloc);
}

char const* systeminfo::GetDeviceUniqueIdentifier()
{
    // return cached value

    static core::string deviceId;

    if (!deviceId.empty())
    {
        return deviceId.c_str();
    }

    // generate unique string from wmi

    core::string value;
    WmiWin32 wmi;

    bool const wmiOpened = wmi.Open();
    Assert(wmiOpened);

    if (wmiOpened)
    {
        value += wmi.GetProperties(L"Win32_BaseBoard", L"SerialNumber");
        value += wmi.GetProperties(L"Win32_BIOS", L"SerialNumber");

        // windows serial number alone is not enough because there might be illegal windows copies with the same serial number

        if (!value.empty())
        {
            value += wmi.GetProperties(L"Win32_OperatingSystem", L"SerialNumber");
        }
    }

    // get unique string from registry if wmi failed

    if (value.empty())
    {
        value = GetDeviceIdFallback();
    }

    // generate hash
    unitytls_errorstate err = unitytls_errorstate_create();
    UInt8 md5Hash[20];
    unitytls_hash_compute(UNITYTLS_HASH_TYPE_SHA1, reinterpret_cast<const UInt8*>(value.data()), value.size(), md5Hash, sizeof(md5Hash), &err);
    deviceId = BytesToHexString(md5Hash, sizeof(md5Hash));

    return deviceId.c_str();
}

char const* systeminfo::GetDeviceModel()
{
    // return cached value

    static core::string deviceModel;

    if (!deviceModel.empty())
    {
        return deviceModel.c_str();
    }

    // cache value

    WmiWin32 wmi;
    bool const wmiOpened = wmi.Open();
    AssertMsg(wmiOpened, "Failed to open WMI");

    if (wmiOpened)
    {
        core::string manufacturer = wmi.GetProperties(L"Win32_ComputerSystem", L"Manufacturer");
        core::string model = wmi.GetProperties(L"Win32_ComputerSystem", L"Model");

        // Use fallback code only if we get none of these
        if (!(manufacturer.empty() && model.empty()))
        {
            deviceModel += model.empty() ? "Unknown" : model;
            if (!manufacturer.empty())
                deviceModel += " (" + manufacturer + ")";
        }
    }

    // fallback
    if (deviceModel.empty())
    {
        core::string const processor = systeminfo::GetProcessorType();
        int const memory = systeminfo::GetPhysicalMemoryMB();

        deviceModel = Format("%s (%d MB)", processor.c_str(), memory);
    }

    // done

    return deviceModel.c_str();
}

#endif
