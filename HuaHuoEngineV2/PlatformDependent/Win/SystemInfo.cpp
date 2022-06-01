#include "UnityPrefix.h"
#include <winerror.h>

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
#include "PlatformDependent/Win/DetectProcessorCores.h"
#include "PlatformDependent/Win/PathUnicodeConversion.h"
#include "PlatformDependent/Win/Registry.h"
#include "Runtime/Misc/SystemInfo.h"
#include "Runtime/Logging/LogAssert.h"
#include "Runtime/Utilities/vector_map.h"

// system includes
#include <WinIoCtl.h>
 #include <Shlobj.h>
#include <WinSock2.h>
#include <Iphlpapi.h>
#include <Psapi.h>
#include <tlhelp32.h>

#include <cstdlib> // for atoi
#include <memory> // unique_ptr

#include "PlatformDependent/Win/Registry.h"
#include "PlatformDependent/Win/VersionHelpers.h"

#include "Modules/TLS/Public/TLS.h"

// from winbase.h
#ifndef VER_PLATFORM_WIN32_CE
#define VER_PLATFORM_WIN32_CE 3
#endif


static bool Is64BitWindows()
{
    #if defined(_WIN64)

    return true;  // 64-bit programs run only on Win64

    #else

    // 32-bit programs run on both 32-bit and 64-bit Windows so must sniff
    BOOL isWow64 = FALSE;

    typedef BOOL (WINAPI *FuncIsWow64Process) (HANDLE, PBOOL);
    HMODULE kernel = GetModuleHandleW(L"kernel32");
    if (kernel)
    {
        FuncIsWow64Process funcWow64 = (FuncIsWow64Process)GetProcAddress(kernel, "IsWow64Process");
        if (NULL != funcWow64)
            funcWow64(GetCurrentProcess(), &isWow64);
    }
    return isWow64 ? true : false;
    #endif
}

#if !PLATFORM_WINRT && !PLATFORM_FAMILY_WINDOWSGAMES
float systeminfo::GetBatteryLevel()
{
    float level = systeminfo::kBatteryLevelUnknown;
    SYSTEM_POWER_STATUS powerStatus;
    if (GetSystemPowerStatus(&powerStatus))
    {
        if (powerStatus.BatteryLifePercent != 255)
        {
            level = powerStatus.BatteryLifePercent / 100.0f;
        }
    }
    return level;
}

BatteryStatus systeminfo::GetBatteryStatus()
{
    BatteryStatus status = kBatteryStatusUnknown;
    bool pluggedIn = false;
    float level = 0;

    SYSTEM_POWER_STATUS powerStatus;
    if (GetSystemPowerStatus(&powerStatus))
    {
        if (powerStatus.ACLineStatus != 255)
        {
            pluggedIn = powerStatus.ACLineStatus;
        }

        if (powerStatus.BatteryLifePercent != 255)
        {
            level = powerStatus.BatteryLifePercent / 100.0f;
        }

        if (pluggedIn && level >= 1.0)
        {
            status = kBatteryStatusFull;
        }
        else if (pluggedIn)
        {
            status = kBatteryStatusCharging;
        }
        else
        {
            status = kBatteryStatusDischarging;
        }
    }

    return status;
}

#endif

#if !PLATFORM_GAMECORE
int systeminfo::GetOperatingSystemNumeric()
{
    static int OSVersion = 0;
    if (OSVersion != 0)
        return OSVersion;

    OSVERSIONINFOW osinfo;
    osinfo.dwOSVersionInfoSize = sizeof(osinfo);
    if (!GetWindowsVersionImpl(&osinfo))
    {
        OSVersion = -1;
        return OSVersion;
    }

    OSVersion = osinfo.dwMajorVersion * 100 + osinfo.dwMinorVersion * 10;
    return OSVersion;
}

core::string systeminfo::GetOperatingSystem()
{
    OSVERSIONINFOW osinfo;
    osinfo.dwOSVersionInfoSize = sizeof(osinfo);
    if (!GetWindowsVersionImpl(&osinfo))
        return "Unknown Windows version";
    DWORD dwPlatformId   = osinfo.dwPlatformId;
    DWORD dwMinorVersion = osinfo.dwMinorVersion;
    DWORD dwMajorVersion = osinfo.dwMajorVersion;
    DWORD dwBuildNumber  = osinfo.dwBuildNumber & 0xFFFF;

    core::string result(kMemTempAlloc);

    if ((dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) && (dwMajorVersion == 4))
    {
        result = "Windows 9x";
    }
    else if (dwPlatformId == VER_PLATFORM_WIN32_NT)
    {
        // Numbers can be found here https://msdn.microsoft.com/en-us/library/windows/desktop/ms724834
        if (dwMajorVersion <= 4)
            result = "Windows NT";
        else if ((dwMajorVersion == 5) && (dwMinorVersion == 0))
            result = "Windows 2000";
        else if ((dwMajorVersion == 5) && (dwMinorVersion == 1))
            result = "Windows XP";
        else if ((dwMajorVersion == 5) && (dwMinorVersion == 2))
            result = "Windows 2003 Server";
        else if ((dwMajorVersion == 6) && (dwMinorVersion == 0))
            result = "Windows Vista";
        else if ((dwMajorVersion == 6) && (dwMinorVersion == 1))
            result = "Windows 7";
        else if ((dwMajorVersion == 6) && (dwMinorVersion == 2))
            result = "Windows 8";
        else if ((dwMajorVersion == 6) && (dwMinorVersion == 3))
            result = "Windows 8.1";
        else if ((dwMajorVersion == 10) && (dwMinorVersion == 0))
            result = "Windows 10";
        // Add future Windows versions here
    }
    else if (dwPlatformId == VER_PLATFORM_WIN32_CE)
    {
        result = "Windows CE";
    }
    else
    {
        result = "Unknown Windows version";
    }

    core::string buildNumber = dwMajorVersion >= 10
        ? registry::getString("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "CurrentBuild", "0")
        : UnsignedIntToString(dwBuildNumber);

    core::string extra = Is64BitWindows() ? " 64bit" : "";

    // Note: we used to includeosinfo.szCSDVersion, but the new implementation has been returning empty string for a long time.
    // I kept 2 spaces in case some unlucky person is parsing this string expecting 2 spaces.
    result += FormatString("  (%u.%u.%s)%s", dwMajorVersion, dwMinorVersion, buildNumber.c_str(), extra.c_str());
    return result;
}

// Gets information like what is displayed in Settings->About->Windows specifications
core::string systeminfo::GetOperatingSystemEx()
{
    OSVERSIONINFOW osinfo;
    osinfo.dwOSVersionInfoSize = sizeof(osinfo);
    if (!GetWindowsVersionImpl(&osinfo))
        return "Unknown Windows version";
    else if (osinfo.dwMajorVersion < 10)
        return systeminfo::GetOperatingSystem();

    core::string root = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion";

    core::string productName = registry::getString(root, "ProductName", "");
    core::string releaseId = registry::getString(root, "ReleaseId", "");
    core::string currentBuild = registry::getString(root, "CurrentBuild", "0");

    UInt32 ubr = registry::getUInt32(root, "UBR", 0);
    UInt32 bitWidth = Is64BitWindows() ? 64 : 32;

    // Sample output: 'Windows 10 Enterprise; OS build 19041.508; Version 2004; 64bit'
    core::string out = Format("%s; OS build %s.%u; Version %s; %ubit", productName.c_str(), currentBuild.c_str(), ubr, releaseId.c_str(), bitWidth);
    return out;
}

core::string systeminfo::GetProcessorType()
{
    core::string rawType = registry::getString("HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", "ProcessorNameString", "Unknown");

    // The returned name often has long series of consecutive spaces. So skip
    // space characters that are followed by space characters in the result.
    TempString result;
    size_t rawlen = rawType.size();
    result.reserve(rawlen);
    for (size_t i = 0; i < rawlen; ++i)
    {
        if (isspace(rawType[i]))
            if (result.empty() || (i != rawlen - 1 && isspace(rawType[i + 1])))
                continue;
        result += rawType[i];
    }
    return result;
}

int systeminfo::GetProcessorFrequencyMHz()
{
    unsigned int mhz = registry::getUInt32("HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", "~MHz", 0);
    return mhz;
}

#endif // !PLATFORM_GAMECORE

static int g_ProcessorCoreCount = 0;
static int g_LogicalProcessorsCount = 0;

int systeminfo::GetProcessorCount()
{
    if (g_LogicalProcessorsCount != 0)
        return g_LogicalProcessorsCount;

    g_LogicalProcessorsCount = DetectNumberOfLogicalProcessors();
    return g_LogicalProcessorsCount;
}

int systeminfo::GetPhysicalProcessorCount()
{
    if (g_ProcessorCoreCount != 0)
        return g_ProcessorCoreCount;

    g_ProcessorCoreCount = DetectNumberOfProcessorCores();
    return g_ProcessorCoreCount;
}

int systeminfo::GetPhysicalMemoryMB()
{
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(memInfo);
    memInfo.ullTotalPhys = 0;
    GlobalMemoryStatusEx(&memInfo);
    DWORDLONG mb = memInfo.ullTotalPhys / 1048576;
    return int(mb);
}

#if UNITY_EDITOR || UNITY_EXTERNAL_TOOL
int systeminfo::GetSystemPagedMemoryMB()
{
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(memInfo);
    memInfo.ullTotalPageFile = 0;
    GlobalMemoryStatusEx(&memInfo);
    DWORDLONG mb = memInfo.ullTotalPageFile / 1048576;
    return int(mb);
}

int systeminfo::GetUsedSystemPagedMemoryMB()
{
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(memInfo);
    memInfo.ullAvailPageFile = 0;
    GlobalMemoryStatusEx(&memInfo);
    DWORDLONG mb = (memInfo.ullTotalPageFile - memInfo.ullAvailPageFile) / 1048576;
    return int(mb);
}

systeminfo::MemoryPressure systeminfo::GetMemoryPressure()
{
    // Experiments show that Windows grows its swap file when total usage crosses 90%.
    // Therefore, a usage of ~90% does not necessarily mean that the system is anywhere
    // near running out of virtual memory. By setting the limit to slightly above 90%,
    // we give Windows a short while to grow its swap memory.
    // If the usage rises above this limit, this suggests that Windows cannot
    // grow its swap memory anymore and then we report a non-normal memory pressure.

    float usage = GetSystemPagedMemoryUsage();
    if (usage > 0.95f)
    {
        return MemoryPressure::Critical;
    }
    else if (usage > 0.93f)
    {
        return MemoryPressure::Warning;
    }
    else
    {
        return MemoryPressure::Normal;
    }
}

#endif

int systeminfo::GetUsedVirtualMemoryMB()
{
#if !UNITY_EXTERNAL_TOOL
    PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
    DWORDLONG mb = (pmc.PrivateUsage + 1024 * 1024 - 1) / (1024 * 1024);
    return int(mb);
#else
    return 0;
#endif
}

#if !PLATFORM_FAMILY_WINDOWSGAMES
int systeminfo::GetExecutableSizeMB()
{
    // take a snapshot of all loaded modules
    HANDLE handle = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, 0);
    int memoryUsage = 0;
    if (handle != INVALID_HANDLE_VALUE)
    {
        MODULEENTRY32 module_entry;
        module_entry.dwSize = sizeof(module_entry);
        if (Module32First(handle, &module_entry))
        {
            do
            {
                memoryUsage += module_entry.modBaseSize;
            }
            while (Module32Next(handle, &module_entry));
        }
        CloseHandle(handle);
    }
    return (memoryUsage + 1024 * 1024 - 1) / (1024 * 1024);
}

#define UNITY_DEFINE_KNOWN_FOLDER(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        EXTERN_C const GUID DECLSPEC_SELECTANY name \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
UNITY_DEFINE_KNOWN_FOLDER(FOLDERID_LocalAppDataLow,     0xA520A1A4, 0x1780, 0x4FF6, 0xBD, 0x18, 0x16, 0x73, 0x43, 0xC5, 0xAF, 0x16);
static core::string gAppDataPath;
core::string systeminfo::GetPersistentDataPath()
{
    if (!gAppDataPath.empty())
        return core::string(gAppDataPath, kMemTempAlloc);
    wchar_t *widePath = NULL;

    HRESULT gotPath{ E_FAIL };

    typedef HRESULT WINAPI SHGetKnownFolderPathFn (REFGUID rfid, DWORD dwFlags, HANDLE hToken, PWSTR *ppszPath);
    HMODULE hShell32 = LoadLibraryW(L"shell32.dll");
    if (hShell32)
    {
        SHGetKnownFolderPathFn* pfnSHGetKnownFolderPath = reinterpret_cast<SHGetKnownFolderPathFn*>(GetProcAddress(hShell32, "SHGetKnownFolderPath"));
        if (pfnSHGetKnownFolderPath)
        {
            gotPath = pfnSHGetKnownFolderPath(FOLDERID_LocalAppDataLow, NULL, NULL, &widePath);
        }
        else
        {
            widePath = reinterpret_cast<wchar_t*>(CoTaskMemAlloc(MAX_PATH * sizeof(wchar_t)));
            gotPath = SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, widePath);
        }
        FreeLibrary(hShell32);
    }

    if (SUCCEEDED(gotPath))
    {
        ConvertWindowsPathName(widePath, gAppDataPath);
    }

    if (widePath)
        CoTaskMemFree(widePath);

    return core::string(gAppDataPath, kMemTempAlloc);
}

static core::string gTempPath;
core::string systeminfo::GetTemporaryCachePath()
{
    if (gTempPath.empty())
    {
        wchar_t widePath[MAX_PATH];
        DWORD ret = GetTempPathW(MAX_PATH, widePath);
        if (ret > 0)
            ConvertWindowsPathName(widePath, gTempPath);
    }

    return core::string(gTempPath, kMemTempAlloc);
}

static core::string gCommonAppDataPath;
core::string systeminfo::GetCommonPersistentDataPath()
{
    if (gCommonAppDataPath.empty())
    {
        wchar_t _path[kDefaultPathBufferSize];
        HRESULT result = SHGetFolderPathW(NULL, CSIDL_COMMON_APPDATA, NULL, SHGFP_TYPE_CURRENT, _path);
        if (result == S_OK)
            ConvertWindowsPathName(_path, gCommonAppDataPath);
    }

    return core::string(gCommonAppDataPath, kMemTempAlloc);
}

#endif// !PLATFORM_FAMILY_WINDOWSGAMES

#if PLATFORM_FAMILY_WINDOWSGAMES
core::string systeminfo::GetSystemLanguageISO()
{
    wchar_t wCharPrefix[LOCALE_NAME_MAX_LENGTH];
    GetUserDefaultLocaleName(wCharPrefix, LOCALE_NAME_MAX_LENGTH);
    core::string prefix;
    ConvertWideToUTF8String(core::wstring(wCharPrefix, 0, 2), prefix);

    return prefix;
}

#else
core::string systeminfo::GetSystemLanguageISO()
{
    LANGID lid = GetUserDefaultUILanguage();
    switch (lid & 0x3ff)
    {
        case LANG_AFRIKAANS:
            return TempString("af");
        case LANG_ARABIC:
            return TempString("ar");
        case LANG_BASQUE:
            return TempString("eu");
        case LANG_BELARUSIAN:
            return TempString("be");
        case LANG_BULGARIAN:
            return TempString("bg");
        case LANG_CATALAN:
            return TempString("ca");
        case LANG_CHINESE:
            return TempString("zh");
        case LANG_CZECH:
            return TempString("cs");
        case LANG_DANISH:
            return TempString("da");
        case LANG_DUTCH:
            return TempString("nl");
        case LANG_ENGLISH:
            return TempString("en");
        case LANG_ESTONIAN:
            return TempString("et");
        case LANG_FAEROESE:
            return TempString("fo");
        case LANG_FINNISH:
            return TempString("fi");
        case LANG_FRENCH:
            return TempString("fr");
        case LANG_GERMAN:
            return TempString("de");
        case LANG_GREEK:
            return TempString("el");
        case LANG_HEBREW:
            return TempString("he");
        case LANG_HUNGARIAN:
            return TempString("hu");
        case LANG_ICELANDIC:
            return TempString("is");
        case LANG_INDONESIAN:
            return TempString("id");
        case LANG_ITALIAN:
            return TempString("it");
        case LANG_JAPANESE:
            return TempString("ja");
        case LANG_KOREAN:
            return TempString("ko");
        case LANG_LATVIAN:
            return TempString("lv");
        case LANG_LITHUANIAN:
            return TempString("lt");
        case LANG_NORWEGIAN:
            return TempString("no");
        case LANG_POLISH:
            return TempString("pl");
        case LANG_PORTUGUESE:
            return TempString("pt");
        case LANG_ROMANIAN:
            return TempString("ro");
        case LANG_RUSSIAN:
            return TempString("ru");
        case LANG_SERBIAN: //same as LANG_CROATIAN "hr"
            return TempString("sr");
        case LANG_SLOVAK:
            return TempString("sk");
        case LANG_SLOVENIAN:
            return TempString("sl");
        case LANG_SPANISH:
            return TempString("es");
        case LANG_SWEDISH:
            return TempString("sv");
        case LANG_THAI:
            return TempString("th");
        case LANG_TURKISH:
            return TempString("tr");
        case LANG_UKRAINIAN:
            return TempString("uk");
        case LANG_VIETNAMESE:
            return TempString("vi");
        default:
            return TempString("unknown");
    }
}

#endif// !PLATFORM_FAMILY_WINDOWSGAMES

int systeminfo::GetSystemLanguage()
{
    static int lang = -1;
    if (lang < 0)
    {
        lang = SystemLanguageUnknown;

        wchar_t wCharPrefix[LOCALE_NAME_MAX_LENGTH];
        GetUserDefaultLocaleName(wCharPrefix, LOCALE_NAME_MAX_LENGTH);

        const core::wstring prefix = core::wstring(wCharPrefix, 0, 2);

        if (prefix == L"af")
            lang = SystemLanguageAfrikaans;
        else if (prefix == L"ar")
            lang = SystemLanguageArabic;
        else if (prefix == L"eu")
            lang = SystemLanguageBasque;
        else if (prefix == L"be")
            lang = SystemLanguageBelarusian;
        else if (prefix == L"bg")
            lang = SystemLanguageBulgarian;
        else if (prefix == L"ca")
            lang = SystemLanguageCatalan;
        else if (prefix == L"zh")
        {
            const core::wstring fullStr = wCharPrefix;
            if (fullStr == L"zh-TW" || fullStr == L"zh-HK" || fullStr == L"zh-MO")
                lang = SystemLanguageChineseTraditional;
            else if (fullStr == L"zh-CN" || fullStr == L"zh-SG" || fullStr == L"zh-MY")
                lang = SystemLanguageChineseSimplified;
            else
                lang = SystemLanguageChinese;
        }
        else if (prefix == L"hr")
            lang = SystemLanguageSerboCroatian;
        else if (prefix == L"cs")
            lang = SystemLanguageCzech;
        else if (prefix == L"da")
            lang = SystemLanguageDanish;
        else if (prefix == L"nl")
            lang = SystemLanguageDutch;
        else if (prefix == L"en")
            lang = SystemLanguageEnglish;
        else if (prefix == L"et")
            lang = SystemLanguageEstonian;
        else if (prefix == L"fo")
            lang = SystemLanguageFaroese;
        else if (prefix == L"fi")
            lang = SystemLanguageFinnish;
        else if (prefix == L"fr")
            lang = SystemLanguageFrench;
        else if (prefix == L"de")
            lang = SystemLanguageGerman;
        else if (prefix == L"el")
            lang = SystemLanguageGreek;
        else if (prefix == L"he")
            lang = SystemLanguageHebrew;
        else if (prefix == L"iw")
            lang = SystemLanguageHebrew;
        else if (prefix == L"hu")
            lang = SystemLanguageHungarian;
        else if (prefix == L"is")
            lang = SystemLanguageIcelandic;
        else if (prefix == L"id")
            lang = SystemLanguageIndonesian;
        else if (prefix == L"in")
            lang = SystemLanguageIndonesian;
        else if (prefix == L"it")
            lang = SystemLanguageItalian;
        else if (prefix == L"ja")
            lang = SystemLanguageJapanese;
        else if (prefix == L"ko")
            lang = SystemLanguageKorean;
        else if (prefix == L"lv")
            lang = SystemLanguageLatvian;
        else if (prefix == L"lt")
            lang = SystemLanguageLithuanian;
        else if (prefix == L"no" || prefix == L"nb" || prefix == L"nn")
            lang = SystemLanguageNorwegian;
        else if (prefix == L"pl")
            lang = SystemLanguagePolish;
        else if (prefix == L"pt")
            lang = SystemLanguagePortuguese;
        else if (prefix == L"ro")
            lang = SystemLanguageRomanian;
        else if (prefix == L"ru")
            lang = SystemLanguageRussian;
        else if (prefix == L"sr")
            lang = SystemLanguageSerboCroatian;
        else if (prefix == L"sk")
            lang = SystemLanguageSlovak;
        else if (prefix == L"sl")
            lang = SystemLanguageSlovenian;
        else if (prefix == L"es")
            lang = SystemLanguageSpanish;
        else if (prefix == L"sv")
            lang = SystemLanguageSwedish;
        else if (prefix == L"th")
            lang = SystemLanguageThai;
        else if (prefix == L"tr")
            lang = SystemLanguageTurkish;
        else if (prefix == L"uk")
            lang = SystemLanguageUkrainian;
        else if (prefix == L"vi")
            lang = SystemLanguageVietnamese;
    }
    return lang;
}

namespace systeminfo
{
    core::string FormatMacAddress(BYTE* addr, ULONG length)
    {
        core::string result(kMemTempAlloc);
        for (unsigned int i = 0; i < length && i < MAX_ADAPTER_ADDRESS_LENGTH; i++)
        {
            char buff[10];
            snprintf(buff, 10, "%.2x", (int)addr[i]);
            result += buff;
            if (i < (length - 1) && i < (MAX_ADAPTER_ADDRESS_LENGTH - 1))
                result += ':';
        }
        return result;
    }

    typedef UNITY_VECTOR (kMemTempAlloc, core::string) MacAddressVector_t;
    MacAddressVector_t GetMacAddresses();
#if !PLATFORM_FAMILY_WINDOWSGAMES
    MacAddressVector_t GetMacAddressesWmi(bool connectorPresent, bool returnOnFirstAddress);
#endif// !PLATFORM_FAMILY_WINDOWSGAMES
}

core::string systeminfo::GetMacAddress()
{
#if PLATFORM_FAMILY_WINDOWSGAMES
    MacAddressVector_t mac_addresses = GetMacAddresses();
#else
    MacAddressVector_t mac_addresses = GetMacAddressesWmi(true, true);
    if (mac_addresses.size() > 0)
        return *mac_addresses.begin();
    // Fallback to unfiltered list (unsorted)
    mac_addresses = GetMacAddressesWmi(false, true);
    if (mac_addresses.size() > 0)
        return *mac_addresses.begin();
    // In case WMI fails, fallback to Win32 function (unsorted)
    mac_addresses = GetMacAddresses();
#endif// !PLATFORM_FAMILY_WINDOWSGAMES

    if (mac_addresses.size() > 0)
        return *mac_addresses.begin();
    return "";
}

systeminfo::MacAddressVector_t systeminfo::GetMacAddresses()
{
    MacAddressVector_t mac_addresses;
    int iteration = 0;
    ULONG ret = 0;
    ULONG size = 15 * 1024;
    IP_ADAPTER_ADDRESSES* addresses = NULL;
    auto buf = std::unique_ptr<char[], decltype(free)*>((char*)0, free);
    do
    {
        buf.reset((char*)malloc(size));
        addresses = (IP_ADAPTER_ADDRESSES*)buf.get();
        ULONG flags = 0;
        //flags |= GAA_FLAG_INCLUDE_PREFIX;
        flags |= GAA_FLAG_SKIP_UNICAST;
        flags |= GAA_FLAG_SKIP_ANYCAST;
        flags |= GAA_FLAG_SKIP_MULTICAST;
        flags |= GAA_FLAG_SKIP_DNS_SERVER;
        //flags |= GAA_FLAG_SKIP_DNS_INFO;
        ret = GetAdaptersAddresses(AF_UNSPEC, flags, NULL, addresses, &size);
        if (ret == ERROR_BUFFER_OVERFLOW)
        {
            buf.reset();
            addresses = NULL;
        }
    }
    while (ret == ERROR_BUFFER_OVERFLOW && iteration++ < 2);

    if (ret != NO_ERROR || !addresses)
    {
        return mac_addresses;
    }

    IP_ADAPTER_ADDRESSES* address = addresses;
    for (; address; address = address->Next)
    {
        if (address->PhysicalAddressLength == 0)
            continue;

        core::string addr = FormatMacAddress(address->PhysicalAddress, address->PhysicalAddressLength);
        mac_addresses.push_back(addr);
    }

    return mac_addresses;
}

bool systeminfo::MacAddressExists(const core::string& macaddr)
{
#if PLATFORM_FAMILY_WINDOWSGAMES
    MacAddressVector_t mac_addresses = GetMacAddresses();
    MacAddressVector_t::const_iterator it, it2;
#else
    // WMI is only available outside the WindowsGames partition
    MacAddressVector_t mac_addresses = GetMacAddressesWmi(false, false);
    MacAddressVector_t::const_iterator it = mac_addresses.begin(), it2 = mac_addresses.end();
    for (; it != it2; it++)
    {
        const core::string& addr = *it;

        if (addr.compare(macaddr, kComparisonIgnoreCase) == 0)
            return true;
    }
    mac_addresses = GetMacAddresses();
#endif // PLATFORM_FAMILY_WINDOWSGAMES

    it = mac_addresses.begin(), it2 = mac_addresses.end();
    for (; it != it2; it++)
    {
        const core::string& addr = *it;

        if (addr.compare(macaddr, kComparisonIgnoreCase) == 0)
            return true;
    }

    return false;
}

#if defined(UNITY_EXTERNAL_TOOL) && !UNITY_EXTERNAL_TOOL
core::string systeminfo::GetCurrentOsVersionString()
{
    HKEY hkResult = NULL;
    REGSAM samDesired = KEY_READ | KEY_WOW64_64KEY;
    ::RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", NULL, samDesired, &hkResult);

    WCHAR szBufferW[512];
    DWORD dwBufferWSize = sizeof(szBufferW);
    ULONG nError = RegQueryValueExW(hkResult, L"ProductId", 0, NULL, (LPBYTE)szBufferW, &dwBufferWSize);

    if (ERROR_SUCCESS == nError)
    {
        core::string uf8String(kMemTempAlloc);
        ConvertWideToUTF8String(szBufferW, uf8String);
        return uf8String;
    }
    else
    {
        ErrorStringMsg("Failed to parse machine binding 1\n");
        return core::string(kMemTempAlloc);
    }
}

//  Function to decode the serial numbers of IDE hard drives
//  using the IOCTL_STORAGE_QUERY_PROPERTY command
char* FlipAndCodeBytes(const char* str, int pos, int flip, char* buf)
{
    int i;
    int j = 0;
    int k = 0;

    buf[0] = '\0';
    if (pos <= 0)
        return buf;

    if (!j)
    {
        char p = 0;

        // First try to gather all characters representing hex digits only.
        j = 1;
        k = 0;
        buf[k] = 0;
        for (i = pos; j && str[i] != '\0'; ++i)
        {
            char c = tolower(str[i]);

            if (isspace(c))
                c = '0';

            ++p;
            buf[k] <<= 4;

            if (c >= '0' && c <= '9')
                buf[k] |= (unsigned char)(c - '0');
            else if (c >= 'a' && c <= 'f')
                buf[k] |= (unsigned char)(c - 'a' + 10);
            else
            {
                j = 0;
                break;
            }

            if (p == 2)
            {
                if (buf[k] != '\0' && !isprint((unsigned char)buf[k]))
                {
                    j = 0;
                    break;
                }
                ++k;
                p = 0;
                buf[k] = 0;
            }
        }
    }

    if (!j)
    {
        // There are non-digit characters, gather them as is.
        j = 1;
        k = 0;
        for (i = pos; j && str[i] != '\0'; ++i)
        {
            char c = str[i];

            if (!isprint((unsigned char)c))
            {
                j = 0;
                break;
            }

            buf[k++] = c;
        }
    }

    if (!j)
    {
        // The characters are not there or are not printable.
        k = 0;
    }

    buf[k] = '\0';

    if (flip)
        // Flip adjacent characters
        for (j = 0; j < k; j += 2)
        {
            char t = buf[j];
            buf[j] = buf[j + 1];
            buf[j + 1] = t;
        }

    // Trim any beginning and end space
    i = j = -1;
    for (k = 0; buf[k] != '\0'; ++k)
    {
        if (!isspace(buf[k]))
        {
            if (i < 0)
                i = k;
            j = k;
        }
    }

    if ((i >= 0) && (j >= 0))
    {
        for (k = i; (k <= j) && (buf[k] != '\0'); ++k)
            buf[k - i] = buf[k];
        buf[k - i] = '\0';
    }

    return buf;
}

#if !PLATFORM_FAMILY_WINDOWSGAMES
char const* systeminfo::GetBootDriveSerialNumber()
{
    static core::string serialNumber;

    TCHAR  infoBuf[MAX_PATH];
    GetSystemDirectory(infoBuf, MAX_PATH);

    LPCSTR pcszDrive = "\\\\.\\";
    TCHAR drive[8];
    memset(drive, 0x00, 8);
    strcpy(drive, pcszDrive);
    strncat(drive, infoBuf, 1);
    strncat(drive, ":", 1);

    HANDLE hPhysicalDriveIOCTL = CreateFileA(drive, 0,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
        OPEN_EXISTING, 0, NULL);
    if (hPhysicalDriveIOCTL == INVALID_HANDLE_VALUE)
    {
        ErrorStringMsg("Failed to parse machine binding 2\n");
        return serialNumber.c_str();
    }

    STORAGE_PROPERTY_QUERY query;
    DWORD cbBytesReturned = 0;
    char buffer[10000];

    memset((void*)&query, 0, sizeof(query));
    query.PropertyId = StorageDeviceProperty;
    query.QueryType = PropertyStandardQuery;

    memset(buffer, 0, sizeof(buffer));

    if (DeviceIoControl(hPhysicalDriveIOCTL, IOCTL_STORAGE_QUERY_PROPERTY,
        &query, sizeof(query),
        &buffer, sizeof(buffer),
        &cbBytesReturned, NULL))
    {
        STORAGE_DEVICE_DESCRIPTOR * descrip = (STORAGE_DEVICE_DESCRIPTOR*)&buffer;
        char buf[1000];
        FlipAndCodeBytes(buffer, descrip->SerialNumberOffset, 1, buf);
        serialNumber = buf;
    }
    return serialNumber.c_str();
}

#endif PLATFORM_FAMILY_WINDOWSGAMES

char const* systeminfo::GetDeviceName()
{
    // return cached value

    static core::string deviceName;

    if (!deviceName.empty())
    {
        return deviceName.c_str();
    }

    // get computer name

    WCHAR name[1024];
    DWORD size = ARRAYSIZE(name);

    BOOL const result = GetComputerNameW(name, &size);
    Assert(FALSE != result);

    if (FALSE == result)
    {
        goto error;
    }

    // convert wide value to utf8

    ConvertWideToUTF8String(name, deviceName);
    if (!deviceName.empty())
    {
        return deviceName.c_str();
    }

    // failed

error:

    deviceName = ".";
    return deviceName.c_str();
}

#if !PLATFORM_FAMILY_WINDOWSGAMES
char const* systeminfo::GetDeviceSystemName()
{
    Assert(false);  // unused on windows
    return "Windows";
}

char const* systeminfo::GetDeviceSystemVersion()
{
    Assert(false);  // unused on windows
    return GetOperatingSystem().c_str();
}

#endif// !PLATFORM_FAMILY_WINDOWSGAMES

#define STATUS_SUCCESS (0x00000000)

// The RtlGetVersion routine returns version information about the currently running operating system
// Unlike GetVersion, it is not shimmed for application manifests and it is available on all
// versions of Windows starting with Windows 2000.
bool GetOSVersionViaRtlFunction(RTL_OSVERSIONINFOW* osVersionInfo)
{
    HMODULE hMod = ::GetModuleHandleW(L"ntdll.dll");
    if (hMod)
    {
        typedef LONG NTSTATUS, *PNTSTATUS;
        typedef NTSTATUS(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);
        RtlGetVersionPtr fxPtr = (RtlGetVersionPtr)::GetProcAddress(hMod, "RtlGetVersion");
        if (fxPtr != nullptr)
        {
            osVersionInfo->dwOSVersionInfoSize = sizeof(*osVersionInfo);
            if (STATUS_SUCCESS == fxPtr(osVersionInfo))
                return true;
        }
    }
    return false;
}

int systeminfo::GetOperatingSystemBuildNumber()
{
    RTL_OSVERSIONINFOW rovi = {0};
    if (GetOSVersionViaRtlFunction(&rovi))
    {
        return rovi.dwBuildNumber;
    }
    return 0;
}

#endif
