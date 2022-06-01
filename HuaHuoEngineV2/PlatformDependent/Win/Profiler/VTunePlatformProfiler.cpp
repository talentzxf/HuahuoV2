#include "UnityPrefix.h"

#if ENABLE_PROFILER && PLATFORM_ARCH_64

#include "Runtime/PluginInterface/PluginInterface.h"
#include "Runtime/PluginInterface/Headers/IUnityProfilerCallbacks.h"
#include "Runtime/Utilities/RuntimeStatic.h"
#include "Runtime/Bootstrap/BootConfig.h"


// VTune support for player

// Note: Attach with VTune captures Unity markers only if INTEL_LIBITTNOTIFY64 envvar is set.
// VTune has several HW and SW - based collectors and a few agents to collect itt tasks and jit info.
// The agents (the shared libraries) are loaded into a profiling application (if correspondend envvars are set).
#include "External/VTune/builds/include/ittnotify.h"
// Note: Dynamic code profiling works only if INTEL_JIT_PROFILER64 envvar is set.
#include "External/VTune/builds/include/jitprofiling.h"

#if ENABLE_MONO && LOAD_MONO_DYNAMICALLY
#include "Runtime/Core/Callbacks/GlobalCallbacks.h"
#include "Runtime/Core/Containers/String.h"
#include "Runtime/Scripting/ScriptingManager.h"
#include "Runtime/Mono/MonoIncludes.h"
#endif // ENABLE_MONO && LOAD_MONO_DYNAMICALLY

// Public flag for enabling VTune profiling
static BootConfig::Parameter<bool> s_EnableVTuneMarkers("profiler-enable-vtune-markers", false);

class VTunePlatformProfiler
{
#if UNITY_EDITOR
    static void OnEditorTerminateDone();
#endif

public:
    VTunePlatformProfiler(MemLabelId memLabel)
        : m_VTuneCapturing(s_EnableVTuneMarkers)
        // If an application has integrated VTune JIT Aps to report out jit data that the VTune’s JIT agent is loaded into the application at start up.
        // In this case, iJIT_IsProfilingActive() returns iJIT_SAMPLING_ON if the agent is loaded and initialized successfully.
        // In other words iJIT_SAMPLING_ON means the agent is ON but not a profiling session is launched.
        // It’s an indicator for the JIT engine to start reporting the jit data with NotifyEvent.
        , m_VTuneJITCapturing(iJIT_IsProfilingActive() == iJIT_SAMPLING_ON)
        , m_MemLabel(memLabel)
    {
        m_UnityProfilerCallbacks = GetUnityInterfaces().Get<IUnityProfilerCallbacks>();
        m_UnityProfilerCallbacks->RegisterFrameCallback(&FrameCallback, this);

        if (m_VTuneCapturing)
        {
            m_UnityProfilerCallbacks->RegisterCreateMarkerCallback(&CreateMarkerCallback, this);
            m_UnityProfilerCallbacks->RegisterCreateThreadCallback(&CreateThreadCallback, this);
        }

#if ENABLE_MONO && LOAD_MONO_DYNAMICALLY
        if (m_VTuneJITCapturing)
        {
            REGISTER_GLOBAL_CALLBACK(loadedScriptingRuntime, "VTunePlatformProfiler.InitializeJIT", VTunePlatformProfiler::InitializeJIT());
        }
#endif // ENABLE_MONO && LOAD_MONO_DYNAMICALLY

#if UNITY_EDITOR
        GlobalCallbacks::Get().editorTerminateDone.Register(OnEditorTerminateDone);
#endif
    }

    ~VTunePlatformProfiler()
    {
#if UNITY_EDITOR
        GlobalCallbacks::Get().editorTerminateDone.Unregister(OnEditorTerminateDone);
#endif

        m_UnityProfilerCallbacks->UnregisterCreateMarkerCallback(CreateMarkerCallback, this);
        m_UnityProfilerCallbacks->UnregisterMarkerEventCallback(NULL, EventCallback, this);
        m_UnityProfilerCallbacks->UnregisterCreateThreadCallback(&CreateThreadCallback, this);


        if (s_ShouldEndFrame)
        {
            __itt_frame_end_v3(s_UnityDomain, NULL);
        }

        __itt_detach();

        s_DefaultMarkerDesc = NULL;


#if ENABLE_MONO && LOAD_MONO_DYNAMICALLY
        if (m_VTuneJITCapturing)
            iJIT_NotifyEvent(iJVM_EVENT_TYPE_SHUTDOWN, NULL);
#endif // ENABLE_MONO && LOAD_MONO_DYNAMICALLY
    }

private:

    static void UNITY_INTERFACE_API CreateMarkerCallback(const UnityProfilerMarkerDesc* eventDesc, void* userData)
    {
        VTunePlatformProfiler* this__ = static_cast<VTunePlatformProfiler*>(userData);

        if (s_DefaultMarkerDesc == NULL)
        {
            if (strcmp(eventDesc->name, "Profiler.Default") == 0)
                s_DefaultMarkerDesc = eventDesc;
        }

        __itt_string_handle* handle = __itt_string_handle_create(eventDesc->name);
        this__->m_UnityProfilerCallbacks->RegisterMarkerEventCallback(eventDesc, &EventCallback, handle);
    }

    static void UNITY_INTERFACE_API EventCallback(const UnityProfilerMarkerDesc* eventDesc, UnityProfilerMarkerEventType eventType, unsigned short eventDataCount, const UnityProfilerMarkerData* eventData, void* userData)
    {
        __itt_string_handle* handle = static_cast<__itt_string_handle*>(userData);
        switch (eventType)
        {
            case kUnityProfilerMarkerEventTypeBegin:
                if (eventDataCount > 2 && eventDesc != NULL && eventDesc == s_DefaultMarkerDesc)
                {
#if defined(WINDOWS)
                    // Default marker emits UTF16 string as the second metadata parameter.
                    const PCWSTR name = static_cast<const PCWSTR>(eventData[1].ptr);

                    // Override handle
                    handle = __itt_string_handle_createW(name);
#else
                    // convert wide to narrow for Linux names
                    const uint16_t* nameWide = static_cast<const uint16_t*>(eventData[1].ptr);
                    char name[256];
                    for (int i = 0; i < 255; ++i)
                    {
                        name[i] = (char)nameWide[i];
                        if (nameWide[i] == 0)
                            break;
                    }

                    // Override handle
                    handle = __itt_string_handle_create(name);
#endif
                }

                __itt_task_begin(s_UnityDomain, __itt_null, __itt_null, handle);
                break;
            case kUnityProfilerMarkerEventTypeEnd:
                __itt_task_end(s_UnityDomain);
                break;
        }
    }

    static void UNITY_INTERFACE_API FrameCallback(void* userData)
    {
        VTunePlatformProfiler* this__ = static_cast<VTunePlatformProfiler*>(userData);

        // Handle callbacks on capture state change
        const bool currentCapturing = s_EnableVTuneMarkers;
        if (this__->m_VTuneCapturing != currentCapturing)
        {
            this__->m_VTuneCapturing = currentCapturing;
            if (currentCapturing)
            {
                this__->m_UnityProfilerCallbacks->RegisterCreateMarkerCallback(&CreateMarkerCallback, this__);
            }
            else
            {
                this__->m_UnityProfilerCallbacks->UnregisterCreateMarkerCallback(CreateMarkerCallback, this__);
                this__->m_UnityProfilerCallbacks->UnregisterMarkerEventCallback(NULL, EventCallback, this__);

                // force a frame end if we switched of capturing
                if (s_ShouldEndFrame)
                {
                    __itt_frame_end_v3(s_UnityDomain, NULL);
                    s_ShouldEndFrame = false;
                }
            }
        }

        // Handle VTune frame markers
        if (currentCapturing)
        {
            if (s_ShouldEndFrame)
            {
                __itt_frame_end_v3(s_UnityDomain, NULL);
            }

            __itt_frame_begin_v3(s_UnityDomain, NULL);
            s_ShouldEndFrame = true;
        }
    }

    static void UNITY_INTERFACE_API CreateThreadCallback(const UnityProfilerThreadDesc* threadDesc, void* userData)
    {
        const ThreadId currentThreadId = CurrentThread::GetID();
        if (threadDesc->threadId == currentThreadId)
            __itt_thread_set_name(threadDesc->name);
    }

#if ENABLE_MONO && LOAD_MONO_DYNAMICALLY
    static void InitializeJIT()
    {
        if (s_EnableVTuneMarkers && mono_profiler_create != NULL)
        {
            s_MonoProfilerHandle = mono_profiler_create(NULL);
            if (mono_profiler_set_jit_done_callback != NULL)
                mono_profiler_set_jit_done_callback(s_MonoProfilerHandle, jit_done);
            if (mono_profiler_set_jit_code_buffer_callback != NULL)
                mono_profiler_set_jit_code_buffer_callback(s_MonoProfilerHandle, code_buffer);
        }
    }

    static void jit_done(void *prof, MonoMethod *method, MonoJitInfo *jinfo)
    {
        core::string methodStr(kMemTempAlloc);
        methodStr.reserve(200);
        const char* methodName = scripting_method_get_name(method);
        methodStr.append(methodName);
        methodStr.append("()"); // Do we need signature? That is expensive to obtain.

        core::string classStr(kMemTempAlloc);
        classStr.reserve(200);
        MonoClass* klass = mono_method_get_class(method);
        const char* namespaceName = mono_class_get_namespace(klass);
        const char* klassName = mono_class_get_name(klass);
        if (namespaceName[0] != '\0')
        {
            classStr.append(namespaceName);
            classStr.append(".");
        }
        classStr.append(klassName);

        gpointer code_start = jinfo->code_start;
        int code_size = jinfo->code_size;

        iJIT_Method_Load vtuneMethod;
        memset(&vtuneMethod, 0, sizeof(vtuneMethod));
        vtuneMethod.method_id = iJIT_GetNewMethodID();
        vtuneMethod.method_name = &methodStr[0];
        vtuneMethod.method_load_address = code_start;
        vtuneMethod.method_size = code_size;
        vtuneMethod.class_file_name = &classStr[0];

        core::string filenameStr(kMemTempAlloc);
        if (mono_debug_find_method != NULL)
        {
            MonoDomain* currentDomain = mono_domain_get();
            MonoDebugMethodJitInfo* dmji = mono_debug_find_method(method, currentDomain);
            if (dmji != NULL)
            {
                vtuneMethod.line_number_size = dmji->num_line_numbers;
                vtuneMethod.line_number_table = (vtuneMethod.line_number_size != 0) ?
                    (LineNumberInfo*)ALLOC_TEMP_MANUAL(LineNumberInfo, vtuneMethod.line_number_size) : NULL;

                for (uint32_t i = 0; i < dmji->num_line_numbers; ++i)
                {
                    MonoDebugSourceLocation* sourceLoc = mono_debug_lookup_source_location(method, dmji->line_numbers[i].native_offset, currentDomain);
                    if (sourceLoc == NULL)
                    {
                        FREE_TEMP_MANUAL(vtuneMethod.line_number_table);
                        vtuneMethod.line_number_table = NULL;
                        vtuneMethod.line_number_size = 0;
                        break;
                    }
                    if (i == 0)
                    {
                        filenameStr = sourceLoc->source_file;
                        vtuneMethod.source_file_name = &filenameStr[0];
                    }
                    vtuneMethod.line_number_table[i].Offset = dmji->line_numbers[i].native_offset;
                    vtuneMethod.line_number_table[i].LineNumber = sourceLoc->row;
                    mono_debug_free_source_location(sourceLoc);
                }
                mono_debug_free_method_jit_info(dmji);
            }
        }

        iJIT_NotifyEvent(iJVM_EVENT_TYPE_METHOD_LOAD_FINISHED, &vtuneMethod);

        if (vtuneMethod.line_number_table != NULL)
            FREE_TEMP_MANUAL(vtuneMethod.line_number_table);
    }

    static void code_buffer(void *prof, void *buffer, uint64_t size, MonoProfilerCodeBufferType type, const void *data)
    {
        core::string methodStr(kMemTempAlloc);
        methodStr.reserve(200);

        iJIT_Method_Load vtuneMethod;
        memset(&vtuneMethod, 0, sizeof(vtuneMethod));

        if (type == MONO_PROFILER_CODE_BUFFER_SPECIFIC_TRAMPOLINE)
        {
            methodStr = "code_buffer_specific_trampoline_";
            methodStr.append((char*)data);
            vtuneMethod.method_name = &methodStr[0];
        }
        else
        {
            vtuneMethod.method_name = (char*)code_buffer_desc(type);
        }

        vtuneMethod.method_id = iJIT_GetNewMethodID();
        vtuneMethod.method_load_address = buffer;
        vtuneMethod.method_size = size;

        iJIT_NotifyEvent(iJVM_EVENT_TYPE_METHOD_LOAD_FINISHED, &vtuneMethod);
    }

    static const char* code_buffer_desc(MonoProfilerCodeBufferType type)
    {
        switch (type)
        {
            case MONO_PROFILER_CODE_BUFFER_METHOD:
                return "code_buffer_method";
            case MONO_PROFILER_CODE_BUFFER_METHOD_TRAMPOLINE:
                return "code_buffer_method_trampoline";
            case MONO_PROFILER_CODE_BUFFER_UNBOX_TRAMPOLINE:
                return "code_buffer_unbox_trampoline";
            case MONO_PROFILER_CODE_BUFFER_IMT_TRAMPOLINE:
                return "code_buffer_imt_trampoline";
            case MONO_PROFILER_CODE_BUFFER_GENERICS_TRAMPOLINE:
                return "code_buffer_generics_trampoline";
            case MONO_PROFILER_CODE_BUFFER_SPECIFIC_TRAMPOLINE:
                return "code_buffer_specific_trampoline";
            case MONO_PROFILER_CODE_BUFFER_HELPER:
                return "code_buffer_misc_helper";
            case MONO_PROFILER_CODE_BUFFER_MONITOR:
                return "code_buffer_monitor";
            case MONO_PROFILER_CODE_BUFFER_DELEGATE_INVOKE:
                return "code_buffer_delegate_invoke";
            case MONO_PROFILER_CODE_BUFFER_EXCEPTION_HANDLING:
                return "code_buffer_exception_handling";
            default:
                return "unspecified";
        }
    }

#endif // ENABLE_MONO && LOAD_MONO_DYNAMICALLY

    IUnityProfilerCallbacks* m_UnityProfilerCallbacks;
    MemLabelId m_MemLabel;
    bool m_VTuneCapturing;
    bool m_VTuneJITCapturing;

    static __itt_domain* s_UnityDomain;
    static bool s_ShouldEndFrame;
    static const UnityProfilerMarkerDesc* s_DefaultMarkerDesc;
#if ENABLE_MONO && LOAD_MONO_DYNAMICALLY
    static void* s_MonoProfilerHandle;
#endif
};

__itt_domain*  VTunePlatformProfiler::s_UnityDomain = __itt_domain_create("com.unity.vtune");
bool  VTunePlatformProfiler::s_ShouldEndFrame = false;
const UnityProfilerMarkerDesc* VTunePlatformProfiler::s_DefaultMarkerDesc = NULL;
#if ENABLE_MONO && LOAD_MONO_DYNAMICALLY
void* VTunePlatformProfiler::s_MonoProfilerHandle = NULL;
#endif

// Should be initialized after profiler plugin API.
// The priority should be after profiler native plugin API is initialized and (if we want to profiler early) before other subsystems.
static RuntimeStatic<VTunePlatformProfiler, true> s_VTunePlatformProfiler(kMemProfiler, "Profiling", "VTuneWrapper", RuntimeStatic<VTunePlatformProfiler, true>::kStatic, -8);

#if UNITY_EDITOR
void VTunePlatformProfiler::OnEditorTerminateDone()
{
    if (static_cast<VTunePlatformProfiler*>(s_VTunePlatformProfiler))
        s_VTunePlatformProfiler.Destroy();
}

#endif  // UNITY_EDITOR

#endif // ENABLE_PROFILER && PLATFORM_ARCH_64
