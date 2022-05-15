#include "AbortShim.h"
#include "Include/C/Baselib_Process.h"

BASELIB_CPP_INTERFACE
{
    AbortShim AbortShim::Empty()
    {
        return AbortShim(nullptr, nullptr);
    }

    AbortShim::AbortShim(FuncPtr setFunc, void* setCtx)
        : func(setFunc), ctx(setCtx)
    {
    }

    bool AbortShim::Call(Baselib_ErrorCode errorCode)
    {
        return func ? (*func)(errorCode, ctx) : false;
    }

    static AbortShim shim = AbortShim::Empty();

    AbortShim SetAbortShim(AbortShim newShim)
    {
        AbortShim oldShim = shim;
        shim = newShim;
        return oldShim;
    }

    void ShimmableAbort(Baselib_ErrorCode errorCode)
    {
        if (shim.Call(errorCode))
            return;
        Baselib_Process_Abort(errorCode);
    }
}
