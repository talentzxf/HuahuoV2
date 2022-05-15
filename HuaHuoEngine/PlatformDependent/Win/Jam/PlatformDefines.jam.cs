using Configuration;

namespace PlatformDependent.Win.Jam
{
    // ReSharper disable once UnusedMember.Global - used via reflection
    class PlatformDefines
    {
        // ReSharper disable once UnusedMember.Global - used via reflection
        internal static void TopLevel()
        {
            GlobalDefines.RegisterPlatform("PLATFORM_STANDALONE_WIN");
            GlobalDefines.Platform_Add(
                "PLATFORM_STANDALONE_WIN",
                "PLATFORM_STANDALONE_WIN",
                "PLATFORM_STANDALONE",
                "UNITY_STANDALONE_WIN",
                "UNITY_STANDALONE",
                "ENABLE_RUNTIME_GI",
                "ENABLE_MOVIES",
                "ENABLE_NETWORK",
                "ENABLE_CRUNCH_TEXTURE_COMPRESSION",
                "ENABLE_UNITYWEBREQUEST",
                "ENABLE_CLOUD_SERVICES_ANALYTICS",
                "ENABLE_CLOUD_SERVICES_PURCHASING",
                "ENABLE_OUT_OF_PROCESS_CRASH_HANDLER",
                "ENABLE_EVENT_QUEUE",
                "ENABLE_CLUSTER_SYNC",
                "ENABLE_CLUSTERINPUT",
                "ENABLE_VR",
                "ENABLE_AR",
                "ENABLE_VIRTUALTEXTURING",
                "PLATFORM_UPDATES_TIME_OUTSIDE_OF_PLAYER_LOOP",
                "GFXDEVICE_WAITFOREVENT_MESSAGEPUMP");

            if (GlobalDefines.IsGlobalEnabled("ENABLE_UNET")) GlobalDefines.Platform_Add("PLATFORM_STANDALONE_WIN", "ENABLE_WEBSOCKET_HOST");

            if (ConvertedJamFile.Vars.TargetPlatformIsWindows) GlobalDefines.Platform_Add("UNITY_EDITOR", "ENABLE_HOLOLENS_MODULE");
        }
    }
}
