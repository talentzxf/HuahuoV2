#pragma once

// Read PlatformDependent\Win\etw\ReadMe.txt for more information

#if ENABLE_EVENT_TRACING_FOR_WINDOWS
#   if !PLATFORM_WIN && !PLATFORM_METRO
#   error "Unsupported platform"
#   endif

#include <windef.h>
#include <timezoneapi.h>

// don't include whole winerror.h just for this
#ifndef ERROR_SUCCESS
#define ERROR_SUCCESS 0L
#endif

#include "artifacts/UnityETWProvider/UnityETWProvider.gen.h"


template<UInt32 enterType, UInt32 leaveType>
class EtwEntry
{
private:
    char const* m_Name;

public:
    inline EtwEntry(char const* name) : m_Name(name) { EventWriteScriptingEvent(m_Name, enterType); }
    inline ~EtwEntry() { EventWriteScriptingEvent(m_Name, leaveType); }
};


#define ETW_CHK(x) {auto const res = x; AssertMsg(ERROR_SUCCESS == res, #x" failed.");}
#else
#define ETW_CHK(x) {}

#endif
