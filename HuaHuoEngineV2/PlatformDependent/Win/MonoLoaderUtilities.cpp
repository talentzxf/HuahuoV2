#include "UnityPrefix.h"
#include "MonoLoaderUtilities.h"
#include "Runtime/Utilities/FileUtilities.h"
#include "Runtime/Utilities/PathNameUtility.h"
#include "Runtime/Bootstrap/BootConfig.h"
#include "Runtime/Misc/Player.h"
#include "Runtime/Mono/MonoManager.h"

#if ENABLE_MONO
static const core::string kMonoLibraryName = "mono.dll";
#if ENABLE_MONO_SGEN
static const core::string kNewMonoLibraryName = "mono-2.0-sgen.dll";
#elif ENABLE_MONO_BDWGC
static const core::string kNewMonoLibraryName = "mono-2.0-bdwgc.dll";
#else
static const core::string kNewMonoLibraryName = "mono-2.0-boehm.dll";
#endif

void GetMonoPaths(core::string& dllPath, core::string& distributionBasePath)
{
    const char* kMonoRuntimeVersion = GetMonoFrameworkDirectory();

#if UNITY_EDITOR
    distributionBasePath = AppendPathName(GetApplicationContentsPath(), kMonoRuntimeVersion);
#else
    distributionBasePath = AppendPathName(GetApplicationFolder(), kMonoRuntimeVersion);
#endif
    dllPath = AppendPathName(distributionBasePath, core::Join("/EmbedRuntime/", kNewMonoLibraryName));

#if !UNITY_EDITOR
    if (BootConfig::CheckKeyValuePairExists("mono-codegen", "il2cpp"))
        dllPath = AppendPathName(SelectDataFolder(), "Native/GameAssembly.dll");
#endif
}

#endif
