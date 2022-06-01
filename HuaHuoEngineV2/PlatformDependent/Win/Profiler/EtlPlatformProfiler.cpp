#include "UnityPrefix.h"

#include "Runtime/PluginInterface/PluginInterface.h"
#include "Runtime/PluginInterface/Headers/IUnityProfilerCallbacks.h"
#include "Runtime/Utilities/dynamic_block_array.h"
#include "Runtime/Utilities/RuntimeStatic.h"

#include "Windows.h"

#if ENABLE_EVENT_TRACING_FOR_WINDOWS

#include "PlatformDependent/Win/etw/ETW.h"

class EtlPlatformProfiler
{
public:
    EtlPlatformProfiler(MemLabelId memLabel)
        : m_DefaultMarkerDesc(NULL)
        , m_CategoryColors(memLabel)
        , m_MemLabel(memLabel)
        , m_IsEtlCapturing(GetEtlCapturingState())
    {
        m_UnityProfilerCallbacks = GetUnityInterfaces().Get<IUnityProfilerCallbacks>();

        m_UnityProfilerCallbacks->RegisterCreateCategoryCallback(&CreateCategoryCallback, this);
        m_UnityProfilerCallbacks->RegisterFrameCallback(&FrameCallback, this);

        if (m_IsEtlCapturing)
            m_UnityProfilerCallbacks->RegisterCreateMarkerCallback(&CreateMarkerCallback, this);
    }

    ~EtlPlatformProfiler()
    {
        m_UnityProfilerCallbacks->UnregisterFrameCallback(FrameCallback, this);
        m_UnityProfilerCallbacks->UnregisterCreateMarkerCallback(CreateMarkerCallback, this);
        m_UnityProfilerCallbacks->UnregisterCreateCategoryCallback(CreateCategoryCallback, this);
        m_UnityProfilerCallbacks->UnregisterMarkerEventCallback(NULL, EventCallback, this);
    }

    static bool GetEtlCapturingState()
    {
        for (auto element : UnityEnableBits)
        {
            if (element > 0) return true;
        }

        return false;
    }

private:
    static void UNITY_INTERFACE_API CreateCategoryCallback(const UnityProfilerCategoryDesc* categoryDesc, void* userData)
    {
        EtlPlatformProfiler* this__ = static_cast<EtlPlatformProfiler*>(userData);
        this__->m_CategoryColors.emplace_back(categoryDesc->rgbaColor);
    }

    static void UNITY_INTERFACE_API CreateMarkerCallback(const UnityProfilerMarkerDesc* eventDesc, void* userData)
    {
        EtlPlatformProfiler* this__ = static_cast<EtlPlatformProfiler*>(userData);

        if (this__->m_DefaultMarkerDesc == NULL)
        {
            if (strcmp(eventDesc->name, "Profiler.Default") == 0)
                this__->m_DefaultMarkerDesc = eventDesc;
        }

        this__->m_UnityProfilerCallbacks->RegisterMarkerEventCallback(eventDesc, EventCallback, this__);
    }

    static void UNITY_INTERFACE_API EventCallback(const UnityProfilerMarkerDesc* eventDesc, UnityProfilerMarkerEventType eventType, unsigned short eventDataCount, const UnityProfilerMarkerData* eventData, void* userData)
    {
        EtlPlatformProfiler* this__ = static_cast<EtlPlatformProfiler*>(userData);

        switch (eventType)
        {
            case kUnityProfilerMarkerEventTypeBegin:
            {
                if (eventDataCount > 2 && eventDesc != NULL && eventDesc == this__->m_DefaultMarkerDesc)
                {
                    // Default marker emits UTF16 string as the second metadata parameter.
                    // ETW doesn't write wide string
                    // For simplicity we slice UTF16 data to char.
                    const UInt16* first = static_cast<const UInt16*>(eventData[1].ptr);
                    const UInt16* last = reinterpret_cast<const UInt16*>(static_cast<const UInt8*>(eventData[1].ptr) + eventData[1].size);
                    dynamic_array<char> str(kMemTempAlloc);
                    str.assign_range(first, last);

                    EventWriteProfilerEvent(str.data(), 1);
                }
                else
                {
                    EventWriteProfilerEvent(eventDesc->name, 1);
                }

                break;
            }
            case kUnityProfilerMarkerEventTypeEnd:
            {
                EventWriteProfilerEvent("", 2);

                break;
            }
        }
    }

    static void UNITY_INTERFACE_API FrameCallback(void* userData)
    {
        EtlPlatformProfiler* this__ = static_cast<EtlPlatformProfiler*>(userData);
        const bool currentCapturing = EtlPlatformProfiler::GetEtlCapturingState();

        if (this__->m_IsEtlCapturing != currentCapturing)
        {
            this__->m_IsEtlCapturing = currentCapturing;

            if (currentCapturing)
            {
                this__->m_UnityProfilerCallbacks->RegisterCreateMarkerCallback(&CreateMarkerCallback, this__);
            }
            else
            {
                this__->m_UnityProfilerCallbacks->UnregisterCreateMarkerCallback(CreateMarkerCallback, this__);
                this__->m_UnityProfilerCallbacks->UnregisterMarkerEventCallback(NULL, EventCallback, this__);
            }
        }
    }

    IUnityProfilerCallbacks* m_UnityProfilerCallbacks;
    const UnityProfilerMarkerDesc* m_DefaultMarkerDesc;
    dynamic_block_array<UInt32, 32> m_CategoryColors;
    MemLabelId m_MemLabel;
    bool m_IsEtlCapturing;
};

static RuntimeStatic<EtlPlatformProfiler, true> s_EtlPlatformProfiler(kMemProfiler, "Profiling", "EtlWrapper", RuntimeStatic<EtlPlatformProfiler, true>::kStatic, -8);
#endif // ENABLE_EVENT_TRACING_FOR_WINDOWS
