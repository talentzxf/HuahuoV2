#pragma once

// Abort shim is enabling intercepting calls to Baselib_Process_Abort.
// This is useful for testing purposes.

#include "Include/Baselib.h"
#include "Include/C/Baselib_ErrorCode.h"

BASELIB_CPP_INTERFACE
{
    class BASELIB_API AbortShim
    {
    public:
        // Return true if abort is handled inside the shim.
        // Otherwise return false to continue to Baselib_Process_Abort.
        using FuncPtr = bool (*)(Baselib_ErrorCode errorCode, void* ctx);

        static AbortShim Empty();
        AbortShim(FuncPtr func, void* ctx);
        bool Call(Baselib_ErrorCode errorCode);

    private:
        FuncPtr func;
        void* ctx;
    };

    // Sets abort shim. Pass AbortShim::Empty to clear the shim.
    // Returns last shim set, so you can daisy chain them.
    BASELIB_API AbortShim SetAbortShim(AbortShim shim);

    // Calls the shim if any, and/or Baselib_Process_Abort.
    // Beware, unlike Baselib_Process_Abort, this ShimmableAbort can return,
    // so please handle this case gracefully.
    BASELIB_API void ShimmableAbort(Baselib_ErrorCode errorCode);
}
