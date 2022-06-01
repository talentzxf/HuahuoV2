#include "UnityPrefix.h"
#include "ExternalCrashHandler.h"
#include "SharedCrashData.h"

#if ENABLE_OUT_OF_PROCESS_CRASH_HANDLER
namespace winutils
{
    bool CreateSharedCrashNotificationEvents(DWORD processID, SharedCrashNotificationEvents& events)
    {
        wchar_t crashEventName[256] = { 0 };
        wchar_t responseEventName[256] = { 0 };
        wchar_t termEventName[256] = { 0 };
        swprintf_s(crashEventName, _countof(crashEventName), L"UnityCrashHandler_%u", processID);
        swprintf_s(responseEventName, _countof(responseEventName), L"UnityCrashHandlerResp_%u", processID);
        swprintf_s(termEventName, _countof(termEventName), L"UnityCrashHandlerTerm_%u", processID);

        AutoHandle crashEvent, respEvent, termEvent;
        crashEvent.Reset(CreateEventW(nullptr, false, false, crashEventName));
        respEvent.Reset(CreateEventW(nullptr, false, false, responseEventName));
        termEvent.Reset(CreateEventW(nullptr, false, false, termEventName));
        if (!crashEvent.IsValid() ||
            !respEvent.IsValid() ||
            !termEvent.IsValid())
            return false;

        events.HostDataReady = std::move(crashEvent);
        events.HostCanContinue = std::move(respEvent);
        events.RequestTerminate = std::move(termEvent);
        return true;
    }
}
#endif // ENABLE_OUT_OF_PROCESS_CRASH_HANDLER
