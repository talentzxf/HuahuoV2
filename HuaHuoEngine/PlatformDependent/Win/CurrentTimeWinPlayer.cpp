#include "UnityPrefix.h"
#include "Runtime/Utilities/DateTime.h"
#include "Runtime/Testing/Fakeable.h"
#include <sysinfoapi.h>

DateTime GetCurrentTimeAsDateTime()
{
    __FAKEABLE_FUNCTION__(GetCurrentTimeAsDateTime, ());

    // Use GetSystemTimeAsFileTime instead of just GetSystemTime because it has better precision (100ns, rather than 1ms)

    FILETIME fileTime;
    GetSystemTimeAsFileTime(reinterpret_cast<FILETIME*>(&fileTime));

    // FILETIME is 100ns-ticks since Jan 1 1601
    ULARGE_INTEGER timeValue;
    timeValue.LowPart = fileTime.dwLowDateTime;
    timeValue.HighPart = fileTime.dwHighDateTime;

    DateTime result(1601, 1, 1, 0, 0, 0);
    result.ticks += timeValue.QuadPart;
    return result;
}
