#pragma once

#include "Include/C/Baselib_Process.h"

BASELIB_C_INTERFACE
{
    void Baselib_Process_Abort(Baselib_ErrorCode error)
    {
        platform::Baselib_Process_Abort(error);
    }
}
