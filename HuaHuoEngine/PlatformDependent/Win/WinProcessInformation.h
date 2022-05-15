#pragma once

#include <windef.h>

namespace winutils
{
    //
    //   FUNCTION: GetProcessIntegrityLevel()
    //
    //   PURPOSE: The function gets the integrity level of the current process.
    //   Integrity level is only available on Windows Vista and newer operating
    //   systems, thus GetProcessIntegrityLevel throws a C++ exception if it is
    //   called on systems prior to Windows Vista.
    //
    //   RETURN VALUE: Returns the integrity level of the current process. It is
    //   usually one of these values:
    //
    //     SECURITY_MANDATORY_UNTRUSTED_RID (SID: S-1-16-0x0)
    //     Means untrusted level. It is used by processes started by the
    //     Anonymous group. Blocks most write access.
    //
    //     SECURITY_MANDATORY_LOW_RID (SID: S-1-16-0x1000)
    //     Means low integrity level. It is used by Protected Mode Internet
    //     Explorer. Blocks write acess to most objects (such as files and
    //     registry keys) on the system.
    //
    //     SECURITY_MANDATORY_MEDIUM_RID (SID: S-1-16-0x2000)
    //     Means medium integrity level. It is used by normal applications
    //     being launched while UAC is enabled.
    //
    //     SECURITY_MANDATORY_HIGH_RID (SID: S-1-16-0x3000)
    //     Means high integrity level. It is used by administrative applications
    //     launched through elevation when UAC is enabled, or normal
    //     applications if UAC is disabled and the user is an administrator.
    //
    //     SECURITY_MANDATORY_SYSTEM_RID (SID: S-1-16-0x4000)
    //     Means system integrity level. It is used by services and other
    //     system-level applications (such as Wininit, Winlogon, Smss, etc.)
    //
    //   EXCEPTION: If this function fails, it throws a C++ DWORD exception
    //   which contains the Win32 error code of the failure. For example, if
    //   GetProcessIntegrityLevel is called on systems prior to Windows Vista,
    //   the error code will be ERROR_INVALID_PARAMETER.
    //
    //   EXAMPLE CALL:
    //     DWORD dwIntegrityLevel;
    //     DWORD error = GetProcessIntegrityLevel(dwIntegrityLevel);
    //     if (error == ERROR_SUCCESS)
    //     {
    //            <Correctly acquired integrity level
    //     }
    //
    DWORD GetProcessIntegrityLevel(DWORD& outIntegrityLevel);

    bool  IsRunningInLowIntegrityLevel();
}  // namespace winutils
