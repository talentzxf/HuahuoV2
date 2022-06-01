#include "UnityPrefix.h"
#include "WinLib.h"
#include "Runtime/Utilities/RuntimeStatic.h"

namespace winlib
{
    RuntimeStatic<LibUser32> User32{kMemDefault};
    LibUser32& GetUser32() { return *User32; }

    RuntimeStatic<LibShcore> Shcore{kMemDefault};
    LibShcore& GetShcore() { return *Shcore; }

    RuntimeStatic<LibKernel32> Kernel32{kMemDefault};
    LibKernel32& GetKernel32() { return *Kernel32; }

    RuntimeStatic<LibDbghelp> Dbghelp{kMemDefault};
    LibDbghelp& GetDbghelp() { return *Dbghelp; }
}
