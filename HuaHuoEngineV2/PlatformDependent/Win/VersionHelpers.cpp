#include "UnityPrefix.h"
#include "VersionHelpers.h"


bool GetWindowsVersionImpl(OSVERSIONINFOW* osinfo)
{
    // Querying Windows version is a costly operation, that's why we're caching results
    // This is an official method of acquiring Windows version on Windows 8 or higher.
    // This also respects data located in manifest files PlatformDependent\Win\res\editor-xpstyle.manifest (Editor) or UnityPlayer.manifest (Standalone)
    static int osIdentified = -1;
    static OSVERSIONINFOW osVersion;
    switch (osIdentified)
    {
        case -1:
        {
            memset(&osVersion, 0, sizeof(OSVERSIONINFOW));
            osVersion.dwPlatformId = VER_PLATFORM_WIN32_NT;
            osVersion.dwBuildNumber = 0;

            osIdentified = 1;
            // ToDo: add future Windows version here
            // dwMajorVersion & dwMinorVersion can be acquired here - https://msdn.microsoft.com/en-us/library/windows/desktop/ms724834%28v=vs.85%29.aspx?f=255&MSPPError=-2147217396
            if (IsWindows10OrGreater())
            {
                osVersion.dwMajorVersion = 10;
                osVersion.dwMinorVersion = 0;
            }
            else if (IsWindows8Point1OrGreater())
            {
                osVersion.dwMajorVersion = 6;
                osVersion.dwMinorVersion = 3;
            }
            else if (IsWindows8OrGreater())
            {
                osVersion.dwMajorVersion = 6;
                osVersion.dwMinorVersion = 2;
            }
            else if (IsWindows7OrGreater())
            {
                osVersion.dwMajorVersion = 6;
                osVersion.dwMinorVersion = 1;
            }
            else if (IsWindowsVistaOrGreater())
            {
                osVersion.dwMajorVersion = 6;
                osVersion.dwMinorVersion = 0;
            }
            else if (IsWindowsXPOrGreater())
            {
                osVersion.dwMajorVersion = 5;
                osVersion.dwMinorVersion = 1;
            }
            else
            {
                osIdentified = 0;
            }

            if (osIdentified == 0)
                return false;
        }
        case 1:
            memcpy(osinfo, &osVersion, sizeof(OSVERSIONINFOW));
            return true;
        case 0:
            return false;
    }
    return false;
}
