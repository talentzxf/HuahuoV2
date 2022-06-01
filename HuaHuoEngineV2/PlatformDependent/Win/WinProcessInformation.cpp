#include "UnityPrefix.h"
#include "WinProcessInformation.h"
#include <windows.h>


// Based on http://blogs.msdn.com/b/msdnforum/archive/2010/03/30/a-quick-start-guide-of-process-mandatory-level-checking-and-self-elevation-under-uac.aspx
DWORD winutils::GetProcessIntegrityLevel(DWORD& outIntegrityLevel)
{
    outIntegrityLevel = 0;
    DWORD dwError = ERROR_SUCCESS;
    HANDLE hToken = NULL;
    DWORD cbTokenIL = 0;
    PTOKEN_MANDATORY_LABEL pTokenIL = NULL;

    // Open the primary access token of the process with TOKEN_QUERY.
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

    // Query the size of the token integrity level information. Note that
    // we expect a FALSE result and the last error ERROR_INSUFFICIENT_BUFFER
    // from GetTokenInformation because we have given it a NULL buffer. On
    // exit cbTokenIL will tell the size of the integrity level information.
    if (!GetTokenInformation(hToken, TokenIntegrityLevel, NULL, 0, &cbTokenIL))
    {
        if (ERROR_INSUFFICIENT_BUFFER != GetLastError())
        {
            // When the process is run on operating systems prior to Windows
            // Vista, GetTokenInformation returns FALSE with the
            // ERROR_INVALID_PARAMETER error code because TokenElevation
            // is not supported on those operating systems.
            dwError = GetLastError();
            goto Cleanup;
        }
    }

    // Now we allocate a buffer for the integrity level information.
    pTokenIL = (TOKEN_MANDATORY_LABEL*)LocalAlloc(LPTR, cbTokenIL);
    if (pTokenIL == NULL)
    {
        dwError = GetLastError();
        goto Cleanup;
    }

    // Retrieve token integrity level information.
    if (!GetTokenInformation(hToken, TokenIntegrityLevel, pTokenIL, cbTokenIL, &cbTokenIL))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

    // Integrity Level SIDs are in the form of S-1-16-0xXXXX. (e.g.
    // S-1-16-0x1000 stands for low integrity level SID). There is one and
    // only one subauthority.
    outIntegrityLevel = *GetSidSubAuthority(pTokenIL->Label.Sid, 0);

Cleanup:
    // Centralized cleanup for all allocated resources.
    if (hToken)
    {
        CloseHandle(hToken);
        hToken = NULL;
    }
    if (pTokenIL)
    {
        LocalFree(pTokenIL);
        pTokenIL = NULL;
        cbTokenIL = 0;
    }

    return dwError;
}

bool winutils::IsRunningInLowIntegrityLevel()
{
    static DWORD s_IntegrityLevel = -1;
    if (s_IntegrityLevel == -1)
    {
        DWORD error = GetProcessIntegrityLevel(s_IntegrityLevel);
        if (error != ERROR_SUCCESS)
        {
            s_IntegrityLevel = -1;
            return false;
        }
    }
    // Note: This is not a bitwise operation, as per example in https://msdn.microsoft.com/en-us/library/bb625966.aspx
    return s_IntegrityLevel == SECURITY_MANDATORY_LOW_RID;
}
