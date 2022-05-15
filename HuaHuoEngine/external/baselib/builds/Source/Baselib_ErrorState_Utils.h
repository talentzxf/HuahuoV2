#pragma once

#include "Include/C/Baselib_ErrorState.h"

#include <ostream>
#include <stdarg.h>

BASELIB_CPP_INTERFACE
{
    // Formats and stores extra information in internal static buffer.
    // Beware that it's implemented via global lock, so will stall on contention.
    // Needs to be exported because it's used by tests.
    BASELIB_API uint64_t Baselib_ErrorState_FormatAndStoreExtraInformation(const char* format, va_list args);

    // Needs to be exported because it's used by tests.
    BASELIB_API std::ostream& operator<<(std::ostream& os, const Baselib_ErrorCode& errorCode);

    // Baselib error codes. Return true if native error code needs to be explained.
    bool Baselib_ErrorState_Explain_BaselibErrorCode(const Baselib_ErrorState* errorState, std::ostream& out, Baselib_ErrorState_ExplainVerbosity verbosity);
}
