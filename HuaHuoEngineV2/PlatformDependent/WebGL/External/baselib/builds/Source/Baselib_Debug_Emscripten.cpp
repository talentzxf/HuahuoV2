#include "Include/Baselib.h"
#include "Include/C/Baselib_Debug.h"

BASELIB_C_INTERFACE
{
    bool Baselib_Debug_IsDebuggerAttached()
    {
        // it seems like there is no reasonable way to do it properly
        return false;
    }
}
