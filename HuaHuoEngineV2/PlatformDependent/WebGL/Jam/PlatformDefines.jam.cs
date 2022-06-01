using Configuration;

namespace PlatformDependent.WebGL.Jam
{
    // ReSharper disable once UnusedMember.Global - used via reflection
    class PlatformDefines
    {
        // ReSharper disable once UnusedMember.Global - used via reflection
        internal static void TopLevel()
        {
            GlobalDefines.RegisterPlatform("PLATFORM_WEBGL");
            GlobalDefines.Platform_Add(
                "PLATFORM_WEBGL",
                "PLATFORM_WEBGL",
                "UNITY_WEBGL",
                "UNITY_WEBGL_API",
                "UNITY_DISABLE_WEB_VERIFICATION",
                "UNITY_GFX_USE_PLATFORM_VSYNC",
                "UNITY_PLUGINS_ARE_IN_EXTERNAL_EXECUTABLE=0",
                "ENABLE_CRUNCH_TEXTURE_COMPRESSION",
                "ENABLE_UNITYWEBREQUEST",
                "ENABLE_CLOUD_SERVICES_ANALYTICS",
                "ENABLE_CLOUD_SERVICES_PURCHASING",
                "ENABLE_ENGINE_CODE_STRIPPING",
                "ENABLE_VR",
                "ENABLE_SPATIALTRACKING");
            GlobalDefines.Platform_Remove(
                "PLATFORM_WEBGL",
                "ENABLE_LZMA",
                "ENABLE_MICROPHONE",
                "INCLUDE_DYNAMIC_GI",
                "ENABLE_SCRIPTING_GC_WBARRIERS",
                "PLATFORM_SUPPORTS_MONO");
            GlobalDefines.Platform_Add("UNITY_EDITOR", "ENABLE_WEBSOCKET_HOST");
        }
    }
}
