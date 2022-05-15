#include "UnityPrefix.h"
#include "Runtime/Misc/Plugins.h"
#include <string>
#include "Runtime/Utilities/PathNameUtility.h"
#include "Runtime/Utilities/FileUtilities.h"
#include "Runtime/Utilities/File.h"

#if !UNITY_EDITOR
#include "Runtime/Misc/Player.h"


static const char* kPluginDirectories[] =
{
    // Look in CPU specific directories first
#if defined(_M_X64)
    "Plugins\\x86_64",
#elif defined(_M_IX86)
    "Plugins\\x86",
#else
#error "Unknown CPU architecture!"
#endif

    // Then fallback to base plugins directory
    "Plugins",
};

core::string FindPluginExecutable(const char* pluginName)
{
    core::string dataPath = SelectDataFolder();
    for (auto pluginDir : kPluginDirectories)
    {
        core::string pluginPath = AppendPathName(dataPath, pluginDir);
        pluginPath = AppendPathName(pluginPath, pluginName);
        pluginPath = AppendPathNameExtension(pluginPath, "dll");

        if (IsFileCreated(pluginPath))
            return pluginPath;
    }

    return pluginName;
}

#endif
