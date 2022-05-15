#pragma warning(disable:4530) // exception handler used
#include "UnityPrefix.h"

#if PLATFORM_STANDALONE || UNITY_EDITOR || UNITY_EXTERNAL_TOOL

#include "WinDriverUtils.h"
#include "Registry.h"
#include "FileInformation.h"
#include <WbemIdl.h>
#include <ObjBase.h>
#include "WinUnicode.h"
#include <tchar.h>
#include "DXGI.h"

// Define those ourselves, since some macro stuff excludes them from windows headers.
#define EOAC_NONE 0
WINOLEAPI CoSetProxyBlanket(IN IUnknown *pProxy, IN DWORD dwAuthnSvc, IN DWORD dwAuthzSvc, IN OLECHAR *pServerPrincName, IN DWORD dwAuthnLevel, IN DWORD dwImpLevel, IN RPC_AUTH_IDENTITY_HANDLE pAuthInfo, IN DWORD dwCapabilities);


bool windriverutils::GetDisplayDriverInfoRegistry(core::string* registryLoc, core::string* name, VersionInfo& version)
{
    // From MSDN KB "How to Check the Video Driver"
    // http://support.microsoft.com/Default.aspx?kbid=200435

    core::string video0Path = registry::getString("HARDWARE\\DEVICEMAP\\VIDEO", "\\Device\\Video0", "");
    if (video0Path.empty())
        return false;

    core::string::size_type pos = video0Path.find("CurrentControlSet");
    if (pos == core::string::npos)
        return false;

    core::string realVideoPath = "SYSTEM\\" + video0Path.substr(pos, video0Path.size() - pos);
    if (registryLoc)
        *registryLoc = realVideoPath;

    core::string driverNames = registry::getString(realVideoPath, "InstalledDisplayDrivers", "");
    if (driverNames.empty())
        return false;
    if (name)
        *name = driverNames;

    wchar_t widePath[kDefaultPathBufferSize];
    MultiByteToWideChar(CP_UTF8, 0, (driverNames + ".dll").c_str(), -1, widePath, kDefaultPathBufferSize);
    FileInformation fileInfo(widePath);
    FileInformation::Version fileVersion = fileInfo.GetFileVersion();
    if (!fileInfo.ValidFileInformation() || fileVersion.v1 == 0 || fileVersion.v2 == 0)
        return false;

    version = VersionInfo(fileVersion.v1, fileVersion.v2, fileVersion.v3, fileVersion.v4);
    return true;
}

bool windriverutils::GetFileVersionOfFile(const wchar_t* widePath, VersionInfo& outVersion)
{
    FileInformation fileInfo(widePath);
    bool ok = fileInfo.ValidFileInformation();
    if (ok)
    {
        FileInformation::Version fileVersion = fileInfo.GetFileVersion();
        outVersion = VersionInfo(fileVersion.v1, fileVersion.v2, fileVersion.v3, fileVersion.v4);
    }
    return ok;
}

static unsigned int GetVideoMemorySizeRegistry()
{
    // MSDN KB "How to Check the Video Driver": http://support.microsoft.com/Default.aspx?kbid=200435
    // only seems to work on Win 2000/XP; on Vista the Video0 entry does not point to the correct driver.
    // So instead we go through all entries and pick the one that seems suitable.

    const int kMaxDriverEntryCount = 8;
    core::string videoPathName = "\\Device\\Video0";
    for (int i = 0; i < kMaxDriverEntryCount; ++i)
    {
        videoPathName[videoPathName.size() - 1] = (char)('0' + i); // change last digit
        core::string videoPath = registry::getString("HARDWARE\\DEVICEMAP\\VIDEO", videoPathName, "");
        if (videoPath.empty())
            continue;

        // Should point to CurrentControlSet
        core::string::size_type pos = videoPath.find("CurrentControlSet");
        if (pos == core::string::npos)
            continue;

        core::string realVideoPath = "SYSTEM\\" + videoPath.substr(pos, videoPath.size() - pos);

        unsigned int size = registry::getUInt32(realVideoPath, "HardwareInformation.MemorySize", 0);
        // if does not have info - continue on searching
        if (size == 0u)
            continue;

        // check if it's built-in vga one
        core::string drivers = registry::getString(realVideoPath, "InstalledDisplayDrivers", "");
        if (drivers == "vga")
            continue;

        // size is in bytes; we need megabytes
        return size / 1024 / 1024;
    }

    // all failed - return zero
    return 0u;
}

static bool GetDisplayAdapterPropertyWMI(const DXGI_ADAPTER_DESC& adapterDesc, LPCWSTR attribName, VARIANT* retValue)
{
    bool success = false;
    CoInitialize(0);
    IWbemLocator* locator = NULL;
    if (SUCCEEDED(CoCreateInstance(CLSID_WbemLocator, NULL, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (void**)&locator)))
    {
        BSTR server = SysAllocString(L"\\\\.\\root\\cimv2");
        if (server)
        {
            IWbemServices* services = NULL;
            // Note: ConnectServer fails on IE7 with UAC. In that case we'll try to read from registry.
            if (SUCCEEDED(locator->ConnectServer(server, NULL, NULL, 0, 0, NULL, NULL, &services)))
            {
                CoSetProxyBlanket(services, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
                BSTR classname = SysAllocString(L"Win32_VideoController");
                if (classname)
                {
                    IEnumWbemClassObject* enumerator = NULL;
                    if (SUCCEEDED(services->CreateInstanceEnum(classname, 0, NULL, &enumerator)))
                    {
                        IWbemClassObject* obj = NULL;
                        DWORD count;
                        while (SUCCEEDED(enumerator->Next(-1, 1, &obj, &count)) && count)
                        {
                            VARIANT deviceIdVar;
                            obj->Get(L"PNPDeviceID", 0, &deviceIdVar, NULL, NULL);
                            core::wstring devDesc(deviceIdVar.bstrVal);
                            VariantClear(&deviceIdVar);

                            const wchar_t* szVendor = wcsstr(devDesc.c_str(), L"VEN_");
                            if (szVendor != nullptr)
                            {
                                const UINT vendor = wcstoul(szVendor + 4, NULL, 0x10);

                                const wchar_t* deviceStrPtr = wcsstr(devDesc.c_str(), L"DEV_");
                                const wchar_t* subsysStrPtr = wcsstr(devDesc.c_str(), L"SUBSYS_");
                                const UINT device = deviceStrPtr != nullptr ? wcstoul(deviceStrPtr + 4, NULL, 0x10) : 0;
                                const UINT subsys = subsysStrPtr != nullptr ? wcstoul(subsysStrPtr + 7, NULL, 0x10) : 0;
                                if (vendor == adapterDesc.VendorId && device == adapterDesc.DeviceId && subsys == adapterDesc.SubSysId)
                                    break;
                            }

                            obj->Release();
                            obj = nullptr;
                        }

                        if (obj != nullptr)
                        {
                            success = SUCCEEDED(obj->Get(attribName, 0, retValue, NULL, NULL));
                            obj->Release();
                        }

                        enumerator->Release();
                    }
                    SysFreeString(classname);
                }
                services->Release();
            }
            locator->Release();
        }
        SysFreeString(server);
    }

    CoUninitialize();
    return success;
}

typedef HRESULT(WINAPI* LPCREATEDXGIFACTORY)(REFIID, void**);
static DXGI_ADAPTER_DESC GetDXGIAdapterDesc(int adapterIndex)
{
    DXGI_ADAPTER_DESC desc = {};
    HINSTANCE hDXGI = LoadLibraryW(L"dxgi.dll");
    if (hDXGI)
    {
        LPCREATEDXGIFACTORY pCreateDXGIFactory = NULL;
        IDXGIFactory* pDXGIFactory = NULL;

        pCreateDXGIFactory = (LPCREATEDXGIFACTORY)GetProcAddress(hDXGI, "CreateDXGIFactory");
        pCreateDXGIFactory(__uuidof(IDXGIFactory), (LPVOID*)&pDXGIFactory);

        if (pDXGIFactory != nullptr)
        {
            IDXGIAdapter* pAdapter = NULL;
            if (S_OK == pDXGIFactory->EnumAdapters(adapterIndex, &pAdapter))
            {
                pAdapter->GetDesc(&desc);
                pAdapter->Release();
            }

            pDXGIFactory->Release();
        }
        FreeLibrary(hDXGI);
    }

    return desc;
}

bool windriverutils::GetDisplayDriverVersionString(int adapter, core::string& driverVersion)
{
    VARIANT ret;
    if (!GetDisplayAdapterPropertyWMI(GetDXGIAdapterDesc(adapter), L"DriverVersion", &ret))
        return false;

    char buf[128];
    WideCharToMultiByte(CP_UTF8, 0, ret.bstrVal, -1, buf, _countof(buf), NULL, NULL);
    driverVersion = buf;

    VariantClear(&ret);
    return !driverVersion.empty();
}

//
// Video memory detection via DXGI on Vista and later, adapted from VideoMemory sample in DX SDK.
//

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=NULL; } }
#endif

static HRESULT GetVideoMemoryViaDXGI(HMONITOR hMonitor, SIZE_T* pDedicatedVideoMemory, SIZE_T* pDedicatedSystemMemory, SIZE_T* pSharedSystemMemory)
{
    HRESULT hr;
    bool bGotMemory = false;
    *pDedicatedVideoMemory = 0;
    *pDedicatedSystemMemory = 0;
    *pSharedSystemMemory = 0;

    HINSTANCE hDXGI = LoadLibraryW(L"dxgi.dll");
    if (hDXGI)
    {
        LPCREATEDXGIFACTORY pCreateDXGIFactory = NULL;
        IDXGIFactory* pDXGIFactory = NULL;

        pCreateDXGIFactory = (LPCREATEDXGIFACTORY)GetProcAddress(hDXGI, "CreateDXGIFactory");
        pCreateDXGIFactory(__uuidof(IDXGIFactory), (LPVOID*)&pDXGIFactory);

        for (int index = 0;; ++index)
        {
            bool bFoundMatchingAdapter = false;
            IDXGIAdapter* pAdapter = NULL;
            hr = pDXGIFactory->EnumAdapters(index, &pAdapter);
            if (FAILED(hr))    // DXGIERR_NOT_FOUND is expected when the end of the list is hit
                break;

            for (int iOutput = 0;; ++iOutput)
            {
                IDXGIOutput* pOutput = NULL;
                hr = pAdapter->EnumOutputs(iOutput, &pOutput);
                if (FAILED(hr))    // DXGIERR_NOT_FOUND is expected when the end of the list is hit
                    break;

                DXGI_OUTPUT_DESC outputDesc;
                ZeroMemory(&outputDesc, sizeof(DXGI_OUTPUT_DESC));
                if (SUCCEEDED(pOutput->GetDesc(&outputDesc)))
                {
                    if (hMonitor == outputDesc.Monitor)
                        bFoundMatchingAdapter = true;
                }
                SAFE_RELEASE(pOutput);
            }

            if (bFoundMatchingAdapter)
            {
                DXGI_ADAPTER_DESC desc;
                ZeroMemory(&desc, sizeof(DXGI_ADAPTER_DESC));
                if (SUCCEEDED(pAdapter->GetDesc(&desc)))
                {
                    bGotMemory = true;
                    *pDedicatedVideoMemory = desc.DedicatedVideoMemory;
                    *pDedicatedSystemMemory = desc.DedicatedSystemMemory;
                    *pSharedSystemMemory = desc.SharedSystemMemory;
                }
                SAFE_RELEASE(pAdapter);
                break;
            }
            SAFE_RELEASE(pAdapter);
        }

        SAFE_RELEASE(pDXGIFactory);
        FreeLibrary(hDXGI);
    }

    if (bGotMemory)
        return S_OK;
    else
        return E_FAIL;
}

unsigned int windriverutils::GetVideoMemorySizeMB(HMONITOR monitor, const TCHAR** outMethod)
{
    SIZE_T dedicatedVideo, dedicatedSystem, sharedSystem;
    if (SUCCEEDED(GetVideoMemoryViaDXGI(monitor, &dedicatedVideo, &dedicatedSystem, &sharedSystem)))
    {
        size_t mb = GetApproximateVideoMemorySizeMB_DXGI(dedicatedVideo, dedicatedSystem, sharedSystem);
        if (mb > 0)
        {
            if (outMethod)
                *outMethod = _T("DXGI");
            return static_cast<unsigned int>(mb);
        }
    }

    unsigned int vram = 0u;
    vram = GetVideoMemorySizeRegistry();
    if (vram != 0)
    {
        if (outMethod)
            *outMethod = _T("registry");
        return vram;
    }
    // Still no info, assume 64 megabytes
    vram = 64;
    if (outMethod)
        *outMethod = _T("fallback");
    return vram;
}

#endif // PLATFORM_STANDALONE || UNITY_EDITOR || UNITY_EXTERNAL_TOOL
