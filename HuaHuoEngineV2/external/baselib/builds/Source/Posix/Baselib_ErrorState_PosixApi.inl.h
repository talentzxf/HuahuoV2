#pragma once

#include "Include/Baselib.h"
#include "Include/C/Baselib_ErrorState.h"
#include "Source/Baselib_ErrorState_Utils.h"
#include "Source/Baselib_InlineImplementation.h"
#include "Source/omemstream.h"

#include <string.h>

namespace PosixApi
{
    BASELIB_INLINE_IMPL void Baselib_ErrorState_Explain_errno(uint64_t nativeErrorCode, omemstream& out, Baselib_ErrorState_ExplainVerbosity verbosity)
    {
        int errorCode = (int)nativeErrorCode;

        switch (verbosity)
        {
            case Baselib_ErrorState_ExplainVerbosity_ErrorType:
                break;

            case Baselib_ErrorState_ExplainVerbosity_ErrorType_SourceLocation_Explanation:
            {
                char errorMessage[1024];

                // http://man7.org/linux/man-pages/man3/strerror.3.html
                // https://stackoverflow.com/questions/32629222/how-to-use-strerror-r-properly-for-android
            #if !defined(__GLIBC__)
                // using XSI-compliant strerror_r
                if (strerror_r(errorCode, errorMessage, sizeof(errorMessage)) == 0)
                    out << " - " << errorMessage;
            #else
                // using GNU-specific strerror_r
                char* errorMessageBuffer = strerror_r(errorCode, errorMessage, sizeof(errorMessage));
                if (errorMessageBuffer)
                {
                    snprintf(errorMessage, sizeof(errorMessage), "%s", errorMessageBuffer);
                    out << " - " << errorMessage;
                }
            #endif
                break;
            }
        }
        out << " (errno:0x" << std::setfill('0') << std::setw(8) << std::hex << errorCode << ")";
    }

    BASELIB_INLINE_IMPL size_t Baselib_ErrorState_Explain(const Baselib_ErrorState* errorState, char* buffer, size_t bufferLen, Baselib_ErrorState_ExplainVerbosity verbosity)
    {
        omemstream out(buffer, bufferLen);
        if (!Baselib_ErrorState_Explain_BaselibErrorCode(errorState, out, verbosity))
            return (uint32_t)out.totalBytes() + 1;

        switch (errorState->nativeErrorCodeType)
        {
            case Baselib_ErrorState_NativeErrorCodeType_errno:
                Baselib_ErrorState_Explain_errno(errorState->nativeErrorCode, out, verbosity);
                break;
        }
        return (uint32_t)out.totalBytes() + 1;
    }
}
