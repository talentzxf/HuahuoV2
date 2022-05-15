#include "UnityPrefix.h"
#include "wow64.h"
#include "WinUtils.h"
#include "Winlib.h"
#include "Runtime/Misc/SystemInfo.h"
#include "Runtime/Logging/LogAssert.h"
#include "Runtime/Utilities/Argv.h"

using namespace winlib;

// Check for wow64 seh hotfix http://support.microsoft.com/kb/976038
static bool HasWoW64Hotfix()
{
    return (GetKernel32().IsFunctionLoaded<Kernel32Dll::Fn::GetProcessUserModeExceptionPolicy>() &&
        GetKernel32().IsFunctionLoaded<Kernel32Dll::Fn::SetProcessUserModeExceptionPolicy>());
}

// Check if we're running in 32bit mode on win64
// We have to do this dynamically since there's no guarantee the api will exist.
static bool IsWow64()
{
    if (GetKernel32().IsFunctionLoaded<Kernel32Dll::Fn::IsWow64Process>())
    {
        BOOL isWow64 = FALSE;
        if (GetKernel32().Invoke<Kernel32Dll::Fn::IsWow64Process>(GetCurrentProcess(), &isWow64))
        {
            return (TRUE == isWow64);
        }
    }

    return false;
}

void CheckWow64()
{
#if UNITY_EDITOR
    const int winVersion = systeminfo::GetOperatingSystemNumeric();
    if (winVersion < winutils::kWindows8)
    {
        GetKernel32().Load();

        // The following warning applies for these OSes:
        // * Windows Vista Service Pack 2 (SP2)
        // * Windows Server 2008 Service Pack 2 (SP2)
        // * Windows 7
        // * Windows Server 2008 R2
        if (IsWow64() && !HasWoW64Hotfix() && IsHumanControllingUs())
        {
            WarningString("Your 64 bit Windows installation is missing an important service pack patch. Please apply http://support.microsoft.com/kb/976038 to ensure stability.");
        }
    }
#endif
}
