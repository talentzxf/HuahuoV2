#include "UnityPrefix.h"
#include "JSBridge.h"
#include "Runtime/Misc/SystemInfo.h"
#include "Runtime/Logging/LogAssert.h"
#include "Runtime/Utilities/RegisterRuntimeInitializeAndCleanup.h"
#include "Runtime/Utilities/File.h"
#include "Runtime/Utilities/HashFunctions.h"
#include "Runtime/Utilities/Word.h"
#include <string>

struct SystemInfoStringCache
{
    static void StaticInitialize(void*);
    static void StaticDestroy(void*);
    core::string deviceModel;
    core::string osVer;
};

SystemInfoStringCache* s_SystemInfoStringCache;

core::string g_WebGLPersistentDataPath;

void md5(unsigned char *input, int ilen, unsigned char output[16]);

void InitializeWebGLPersistentDataPath(const core::string& url)
{
    core::string dataPath = url.substr(0, url.find('?'));
    dataPath = dataPath.substr(0, dataPath.rfind(kPathNameSeparator));
    // Can't use hashing functions from TLS module here, since it has no (=Dummy) implementation for WebGL
    UInt8 dataPathHash[16];
    md5((unsigned char*)&dataPath[0], dataPath.size(), dataPathHash);
    g_WebGLPersistentDataPath = core::string("/idbfs/") + MD5ToString(dataPathHash);
    CreateDirectoryRecursive(g_WebGLPersistentDataPath);
}

void SystemInfoStringCache::StaticInitialize(void*)
{
    s_SystemInfoStringCache = UNITY_NEW(SystemInfoStringCache, kMemManager);
}

void SystemInfoStringCache::StaticDestroy(void*)
{
    UNITY_DELETE(s_SystemInfoStringCache, kMemManager);
}

static RegisterRuntimeInitializeAndCleanup s_SystemInfoStringCacheCallbacks(SystemInfoStringCache::StaticInitialize, SystemInfoStringCache::StaticDestroy);

core::string systeminfo::GetOperatingSystem()
{
    if (!s_SystemInfoStringCache->osVer.empty())
        return s_SystemInfoStringCache->osVer;

    s_SystemInfoStringCache->osVer = GetStringFromJS(JS_SystemInfo_GetOS);
    return s_SystemInfoStringCache->osVer;
}

core::string systeminfo::GetProcessorType()
{
    return kUnsupportedIdentifier;
}

int systeminfo::GetProcessorFrequencyMHz()
{
    return 0;
}

char const* systeminfo::GetDeviceModel()
{
    if (!s_SystemInfoStringCache->deviceModel.empty())
        return s_SystemInfoStringCache->deviceModel.c_str();

    core::string browser = GetStringFromJS(JS_SystemInfo_GetBrowserName);
    core::string version = GetStringFromJS(JS_SystemInfo_GetBrowserVersionString);

    s_SystemInfoStringCache->deviceModel = Format("%s %s", browser.c_str(), version.c_str());
    return s_SystemInfoStringCache->deviceModel.c_str();
}

int systeminfo::GetProcessorCount()
{
    return emscripten_has_threading_support() ? emscripten_num_logical_cores() : 1;
}

int systeminfo::GetPhysicalProcessorCount()
{
    return GetProcessorCount();
}

int systeminfo::GetPhysicalMemoryMB()
{
    return JS_SystemInfo_GetMemory();
}

int systeminfo::GetUsedVirtualMemoryMB()
{
    // TODO: implement
    return 0;
}

int systeminfo::GetExecutableSizeMB()
{
    // TODO: implement
    return 0;
}

char const* systeminfo::GetDeviceUniqueIdentifier()
{
    return kUnsupportedIdentifier;
}

char const* systeminfo::GetDeviceName()
{
    return kUnsupportedIdentifier;
}

char const* systeminfo::GetDeviceSystemName()
{
    return kUnsupportedIdentifier;
}

char const* systeminfo::GetDeviceSystemVersion()
{
    return kUnsupportedIdentifier;
}

int systeminfo::GetSystemLanguage()
{
    core::string language = GetStringFromJS(JS_SystemInfo_GetLanguage);
    return ISOToSystemLanguage(language);
}

core::string systeminfo::GetPersistentDataPath()
{
    return g_WebGLPersistentDataPath;
}

core::string systeminfo::GetTemporaryCachePath()
{
    return "/tmp";
}
