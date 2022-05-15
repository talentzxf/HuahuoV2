#include "UnityPrefix.h"
#include "Runtime/Misc/Plugins.h"

extern "C" void UnityRegisterRenderingPlugin(PluginLoadFunc loadPlugin, PluginUnloadFunc unloadPlugin)
{
    UnityPluginFunctions func =
    {
        0,
        0,
        0,
        loadPlugin,
        unloadPlugin
#if GFX_SUPPORTS_RENDERING_EXT_PLUGIN
        , 0,
        0
#endif
#if GFX_SUPPORTS_SHADERCOMPILER_EXT_PLUGIN
        , 0
#endif
    };
    RegisterPlugin(0, func);
}
