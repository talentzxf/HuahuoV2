#pragma once

#include "Source/Baselib_InlineImplementation.h"
#include "Include/C/Baselib_Process.h"

#include <stdlib.h>

namespace C99Api
{
    BASELIB_INLINE_IMPL COMPILER_NORETURN void Baselib_Process_Abort(Baselib_ErrorCode error)
    {
        abort();
    }
}
