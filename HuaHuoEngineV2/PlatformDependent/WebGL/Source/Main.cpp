#include <stdlib.h>
#include <stdio.h>

#include "UnityPrefix.h"

#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/threading.h>

#include "InputWebGL.h"
#include "JSBridge.h"
#include "SystemInfo.h"

#include "Runtime/Allocator/MemoryManager.h"
#include "Runtime/Bootstrap/BootConfig.h"
#include "Runtime/Input/Cursor.h"
#include "Runtime/Input/GetInput.h" // InitInput(), InputShutdown()
#include "Runtime/Input/InputManager.h" // GetInputManager()
#include "Runtime/Input/TargetFrameRate.h"
#include "Runtime/Input/TimeManager.h"
#include "Runtime/Misc/Player.h"
#include "Runtime/Graphics/DrawSplashScreenAndWatermarks.h"
#include "Runtime/Graphics/ScreenManager.h"
#include "Runtime/Graphics/Texture2D.h"
#include "Runtime/Misc/PlayerSettings.h"
#include "Runtime/Misc/SystemInfo.h"
#include "Modules/EntryPoint/EntryPoint.h"
#include "Runtime/Scripting/GarbageCollector.h"
#include "Runtime/Scripting/Il2CppLoader.h"
#include "Runtime/Scripting/MonoManager_Il2Cpp.h"
#include "Runtime/ScriptingBackend/ScriptingApi.h"
#include "Runtime/Scripting/ScriptingManager.h"
#include "Runtime/Shaders/GraphicsCaps.h"
#include "Runtime/Testing/Testing.h"
#include "Runtime/Utilities/Argv.h"
#include "Runtime/Core/Callbacks/GlobalCallbacks.h"
#include "Runtime/Profiler/Profiler.h"

PROFILER_INFORMATION(gWebGLPrep, "WebGL Pre-Run Initializations", kProfilerLoading);

static void PostprocessWebGLGraphicsCaps()
{
    // disable deferred rendering on webgl1.0:
    // - it requires WEBGL_draw_buffers extension (not always available)
    // - it does not work properly on Windows if browser rendering back-end is DirectX (via Angle)
    GetGraphicsCaps().hasDeferredRenderLoop = GetGraphicsCaps().hasDeferredRenderLoop && IsGfxLevelES3(GetGraphicsCaps().gles.featureLevel);
    GetGraphicsCaps().rendererString = GetStringFromJS(JS_SystemInfo_GetGPUInfo);
}

static void MainLoop(void);
static void DoQuit(void);

static UInt32 gMainLoopUpdateIntervalId = 0;
// Keep track of last wallclock time when an engine frame was updated
static double gTimeOfLastMainLoopUpdate = 0;

// If RunInBackground is set, then we want MainLoop to be called even if requestAnimationFrame isn't fired,
// which would happen if the browser tab isn't visible.
void MainLoopUpdateFromBackground(void*)
{
    double timeNow = emscripten_get_now();
    // slightly below 1000 msecs for safe granularity
    if (timeNow - gTimeOfLastMainLoopUpdate > 900)
    {
        MainLoop();
    }
}

static void MainLoop(void)
{
    gTimeOfLastMainLoopUpdate  = emscripten_get_now();

    if (GetInputManager().ShouldQuit() || EM_ASM_INT(return typeof Module.shouldQuit != "undefined"))
    {
        DoQuit();
        return;
    }

    GetScreenManager().Update();

    if (GetShouldShowSplashScreen() && !IsSplashScreenFadeComplete())
    {
        DrawSplashScreen(true);
        return;
    }

    InputProcess();

    double width, height;
    GetCanvasClientSize(&width, &height);

    if (kPlayerPaused != GetPlayerPause() && width > 0 && height > 0 && !emscripten_is_webgl_context_lost(NULL))
    {
        if (kPlayerPausing == GetPlayerPause())
            SetPlayerPause(kPlayerPaused);

        ::SetIsBatchmode(false);
        PlayerLoop();

        // we keep GC disabled during player loop except now when we give GC a chance to run
        // Incremental GC is performed during sync, dynamically adjusted to player loop time.
        GetTimeManager().Sync(TimeManager::AfterPlayerLoop);

        // if incremental GC is not used, call scripting_gc_collect_a_little, which will initiate
        // a full GC if needed.
        if (!GarbageCollector::GetIncrementalEnabled())
        {
            scripting_gc_set_mode(kEnabled);
            scripting_gc_collect_a_little();
            scripting_gc_set_mode(kDisabled);
        }
    }
    else
    {
        INVOKE_GLOBAL_CALLBACK(whilePaused);
    }

    // Adjust target frame rate
    const int targetFramerate = GetTargetFrameRate();

    int targetMode, targetValue;
    if (targetFramerate <= 0)
    {
        // When we don't have a throttled target frame rate use requestAnimationFrame timing for smoothest animation.
        targetMode = EM_TIMING_RAF;
        targetValue = 1;
    }
    else
    {
        targetMode = EM_TIMING_SETTIMEOUT;
        targetValue = 1000 / targetFramerate;
    }

    int curMode, curValue;
    emscripten_get_main_loop_timing(&curMode, &curValue);
    if (targetMode != curMode || targetValue != curValue)
        emscripten_set_main_loop_timing(targetMode, targetValue);
}

int InitWebGLPlayer(int argc, char **argv)
{
    if (!BootConfig::InitFromFile((const char**)&argv[1], argc - 1, BootConfig::kFilename))
    {
        WarningString("no boot config - using default values");
        BootConfig::Init((const char**)&argv[1], argc - 1);
    }
    SetupArgv(argc, (const char**)argv);

    // Initialize MemoryManager here so we can initialize the profiler early.
    MemoryManager::StaticInitialize();

#if ENABLE_PROFILER
    JS_Profiler_InjectJobs();
    PROFILER_END(gWebGLPrep);
#endif

    InputInit();

    // Disable GC. Only enabled at fixed time in mainloop above
    scripting_gc_set_mode(kDisabled);

    RuntimeInitialize();

    InitializeIl2CppFromMain(core::string("Managed"), core::string("Il2CppData"), argc, (const char**)argv);

    RunNativeTestsIfRequiredAndExit();

    core::string url = GetStringFromJS(JS_SystemInfo_GetDocumentURL);

    InitializeWebGLPersistentDataPath(url);

    GlobalCallbacks::Get().initializedGraphicsCaps.Register(PostprocessWebGLGraphicsCaps);

    if (!PlayerInitEngineNoGraphics("", ""))
    {
        printf_console("Failed to initialize player\n");
        return 1;
    }

    if (!PlayerInitEngineGraphics())
    {
        printf_console("Failed to initialize player\n");
        return 1;
    }

    GetScriptingManager().RebuildNativeTypeToScriptingClass();

    GetPlayerSettings().absoluteURL = url;

    JS_FileSystem_Initialize();

    // Workaround to bug in asm2wasm which makes it fail on gLogToConsoleFunc function pointer invocation
    // if gLogToConsoleFunc is never assigned, causing the invocation to be partially optimized away by llvm.
    RegisterLogToConsole(NULL);

    PlayerLoadFirstScene();
    PlayerInitState();
    Cursors::InitializeCursors(GetPlayerSettings().GetDefaultCursor(), GetPlayerSettings().GetCursorHotspot());

    if (GetPlayerSettings().GetRunInBackground())
    {
        // The current version of Emscripten does not have the emscripten_set_interval. When emscripten is updated,
        // that function can be used instead of JS_Eval_SetInterval.
        gMainLoopUpdateIntervalId = JS_Eval_SetInterval(MainLoopUpdateFromBackground, NULL, 1000);
    }

    return 0;
}

void DoQuit()
{
    printf_console("Quitting...");

    // engine shutdown
    PlayerCleanup(true);
    // remove event listeners from window, document, canvas, etc..
    emscripten_html5_remove_all_event_listeners();
    // prevent main loop from running again
    emscripten_cancel_main_loop();
    // remaining JS cleanup
    EM_ASM(
        for (var id in Module.intervals)
        {
            window.clearInterval(id);
        }
        Module.intervals = {};

        for (var i = 0; i < Module.deinitializers.length; i++)
        {
            Module.deinitializers[i]();
        }
        Module.deinitializers = [];

        if (typeof Module.onQuit == "function")
            Module.onQuit();
    );

    JS_Eval_ClearInterval(gMainLoopUpdateIntervalId);

    printf_console("done!\n");
}

void RunMainLoop()
{
    emscripten_set_main_loop(MainLoop, 0, 1);
}

#if !ENABLE_ENTRYPOINT_MODULE
int EMSCRIPTEN_KEEPALIVE main(int argc, char **argv)
{
#if SUPPORT_THREADS
    Thread::mainThreadId = pthread_self();
#endif

    int retval = InitWebGLPlayer(argc, argv);
    if (retval == 0)
        RunMainLoop();
    return retval;
}

#else

void InitCoreModule()
{
    InitWebGLPlayer(PackedPlayerGetArgc(), PackedPlayerGetArgv());
    PackedPlayerSetMainLoop(RunMainLoop);
}

#endif

#include <math.h>
extern "C" {
#if ENABLE_PROFILER
// This is called from JS by JS_Profiler_InjectJobs to inject the pre-load jobs into the profiler.
    void EMSCRIPTEN_KEEPALIVE InjectProfilerSample(const char* name, double starttime, double endtime)
    {
        // patch performance.now to return start time
        EM_ASM(Module['emscripten_get_now_backup'] = performance.now; );
        EM_ASM_({performance.now = function() {
                     return $0;
                 }; }, starttime);

        if (!profiler_is_available())
        {
            profiler_initialize();
            PROFILER_BEGIN(gWebGLPrep);
        }

        profiling::Marker& profilerInfo = *profiler_get_info_for_name(name, profiling::kProfilerLoading);
        PROFILER_BEGIN(profilerInfo);

        // patch performance.now to return end time
        EM_ASM_({performance.now = function() {
                     return $0;
                 }; }, endtime);

        PROFILER_END(profilerInfo);

        // restore performance.now
        EM_ASM(performance.now = Module['emscripten_get_now_backup']; );
    }

#endif

    int isFinf(float x)
    {
        return isfinite(x);
    }
}
