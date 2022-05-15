#if ENABLE_PROFILER && PLATFORM_ARCH_64

#include "UnityPrefix.h"

#include "Runtime/PluginInterface/PluginInterface.h"
#include "Runtime/PluginInterface/Headers/IUnityProfilerCallbacks.h"
#include "Runtime/Utilities/dynamic_block_array.h"
#include "Runtime/Utilities/RuntimeStatic.h"

#include "Windows.h"
#include "evntprov.h"

#ifndef USE_PIX
#define USE_PIX
#endif

#include "External/WinPIXEventRuntime/builds/include/WinPIXEventRuntime/pix3.h"
#include "PixHelpers.h"

class PixPlatformProfiler
{
public:
    PixPlatformProfiler(MemLabelId memLabel)
        : m_DefaultMarkerDesc(NULL)
        , m_CategoryColors(memLabel)
        , m_MemLabel(memLabel)
        , m_PixEventHandle(0)
        , m_IsPixCapturing(false)
    {
        m_UnityProfilerCallbacks = GetUnityInterfaces().Get<IUnityProfilerCallbacks>();
        m_UnityProfilerCallbacks->RegisterCreateCategoryCallback(&CreateCategoryCallback, this);

        m_PixEventHandle = PixCaptureStateCallbackRegister(&PixCaptureStateCallback, this);
    }

    ~PixPlatformProfiler()
    {
        m_UnityProfilerCallbacks->UnregisterCreateMarkerCallback(CreateMarkerCallback, this);
        m_UnityProfilerCallbacks->UnregisterCreateCategoryCallback(CreateCategoryCallback, this);
        m_UnityProfilerCallbacks->UnregisterMarkerEventCallback(NULL, EventCallback, this);

        PixCaptureStateCallbackUnregister(m_PixEventHandle);
    }

    friend static void PixCaptureStateCallback(
        LPCGUID SourceId,
        ULONG IsEnabled,
        UCHAR Level,
        ULONGLONG MatchAnyKeyword,
        ULONGLONG MatchAllKeyword,
        PEVENT_FILTER_DESCRIPTOR FilterData,
        PVOID CallbackContext
    );

private:
    static void UNITY_INTERFACE_API CreateCategoryCallback(const UnityProfilerCategoryDesc* categoryDesc, void* userData)
    {
        PixPlatformProfiler* this__ = static_cast<PixPlatformProfiler*>(userData);
        this__->m_CategoryColors.emplace_back(categoryDesc->rgbaColor);
    }

    static void UNITY_INTERFACE_API CreateMarkerCallback(const UnityProfilerMarkerDesc* eventDesc, void* userData)
    {
        PixPlatformProfiler* this__ = static_cast<PixPlatformProfiler*>(userData);

        if (this__->m_DefaultMarkerDesc == NULL)
        {
            if (strcmp(eventDesc->name, "Profiler.Default") == 0)
                this__->m_DefaultMarkerDesc = eventDesc;
        }

        this__->m_UnityProfilerCallbacks->RegisterMarkerEventCallback(eventDesc, EventCallback, this__);
    }

    static void UNITY_INTERFACE_API EventCallback(const UnityProfilerMarkerDesc* eventDesc, UnityProfilerMarkerEventType eventType, unsigned short eventDataCount, const UnityProfilerMarkerData* eventData, void* userData)
    {
        PixPlatformProfiler* this__ = static_cast<PixPlatformProfiler*>(userData);

        switch (eventType)
        {
            case kUnityProfilerMarkerEventTypeBegin:
            {
                if (eventDataCount > 2 && eventDesc != NULL && eventDesc == this__->m_DefaultMarkerDesc)
                {
                    // Default marker emits UTF16 string as the second metadata parameter.
                    const PCWSTR name = static_cast<const PCWSTR>(eventData[1].ptr);
                    UnityProfilerCategoryId categoryId = static_cast<UnityProfilerCategoryId>(*(static_cast<const uint32_t*>(eventData[2].ptr)));

                    // PIX can write wide string natively
                    PIXBeginEvent(this__->m_CategoryColors[categoryId], name);
                }
                else
                {
                    // Use UTF-8 markers
                    PIXBeginEvent(this__->m_CategoryColors[eventDesc->categoryId], eventDesc->name);
                }

                break;
            }
            case kUnityProfilerMarkerEventTypeEnd:
            {
                PIXEndEvent();
                break;
            }
        }
    }

    IUnityProfilerCallbacks* m_UnityProfilerCallbacks;
    const UnityProfilerMarkerDesc* m_DefaultMarkerDesc;
    dynamic_block_array<UInt32, 32> m_CategoryColors;
    MemLabelId m_MemLabel;

    REGHANDLE m_PixEventHandle;
    bool m_IsPixCapturing;
};

// Callback function for PIX to register/unregister platform hooks when it begins or ends a capture.
static void PixCaptureStateCallback(
    LPCGUID SourceId,
    ULONG IsEnabled,
    UCHAR Level,
    ULONGLONG MatchAnyKeyword,
    ULONGLONG MatchAllKeyword,
    PEVENT_FILTER_DESCRIPTOR FilterData,
    PVOID CallbackContext
)
{
    PixPlatformProfiler* this__ = static_cast<PixPlatformProfiler*>(CallbackContext);
    const bool currentCapturing = IsEnabled > 0;

    if (this__->m_IsPixCapturing != currentCapturing)
    {
        this__->m_IsPixCapturing = currentCapturing;

        if (currentCapturing)
        {
            this__->m_UnityProfilerCallbacks->RegisterCreateMarkerCallback(&PixPlatformProfiler::CreateMarkerCallback, this__);
        }
        else
        {
            this__->m_UnityProfilerCallbacks->UnregisterCreateMarkerCallback(PixPlatformProfiler::CreateMarkerCallback, this__);
            this__->m_UnityProfilerCallbacks->UnregisterMarkerEventCallback(NULL, PixPlatformProfiler::EventCallback, this__);
        }
    }
}

// Should be initialized after profiler plugin API.
// The priority should be after profiler native plugin API is initialized and (if we want to profiler early) before other subsystems.
static RuntimeStatic<PixPlatformProfiler, true> s_PixPlatformProfiler(kMemProfiler, "Profiling", "PixWrapper", RuntimeStatic<PixPlatformProfiler, true>::kStatic, -8);
#endif // ENABLE_PROFILER && PLATFORM_ARCH_64
