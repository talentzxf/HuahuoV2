#pragma once

#include "Include/C/Baselib_ErrorState.h"

BASELIB_C_INTERFACE
{
    uint32_t Baselib_ErrorState_Explain(const Baselib_ErrorState* errorState, char* buffer, uint32_t bufferLen, Baselib_ErrorState_ExplainVerbosity verbose)
    {
        return platform::Baselib_ErrorState_Explain(errorState, buffer, bufferLen, verbose);
    }
}
