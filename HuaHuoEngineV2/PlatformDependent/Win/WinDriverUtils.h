#pragma once

#if PLATFORM_STANDALONE || UNITY_EDITOR || UNITY_EXTERNAL_TOOL

#include <windef.h>
#include "Runtime/Core/Containers/String.h"

namespace windriverutils
{
    struct VersionInfo
    {
        VersionInfo() : hipart(0), lopart(0) {}
        VersionInfo(unsigned short v1, unsigned short v2, unsigned short v3, unsigned short v4)
            : hipart((v1 << 16) | v2), lopart((v3 << 16) | v4) {}

        bool operator<(const VersionInfo& rhs) const
        {
            if (hipart == rhs.hipart)
                return lopart < rhs.lopart;
            return hipart < rhs.hipart;
        }

        bool operator<=(const VersionInfo& rhs) const
        {
            if (hipart == rhs.hipart)
                return lopart <= rhs.lopart;
            return hipart <= rhs.hipart;
        }

        bool operator==(const VersionInfo& rhs) const
        {
            return hipart == rhs.hipart && lopart == rhs.lopart;
        }

        bool operator!=(const VersionInfo& rhs) const
        {
            return hipart != rhs.hipart || lopart != rhs.lopart;
        }

        DWORD hipart;
        DWORD lopart;
    };

    bool GetDisplayDriverInfoRegistry(core::string* registryLoc, core::string* name, VersionInfo& version);

    unsigned int GetVideoMemorySizeMB(HMONITOR monitor, const TCHAR** outMethod);

    bool GetFileVersionOfFile(const wchar_t* widePath, VersionInfo& outVersion);

    bool GetDisplayDriverVersionString(int adapter, core::string& driverVersion);
}  // namespace windriverutils

#endif // PLATFORM_STANDALONE || UNITY_EDITOR || UNITY_EXTERNAL_TOOL

// Given DXGI reported adapter memory sizes, try to come up with
// a single rough "this is VRAM size" in megabytes.
inline size_t GetApproximateVideoMemorySizeMB_DXGI(size_t dedicatedVideo, size_t dedicatedSystem, size_t sharedSystem)
{
    size_t mb = dedicatedVideo / (1024 * 1024);

    // For integrated GPUs: they often report something like 64MB of dedicated memory, even for
    // a relatively good GPU (e.g. Intel Sandy Bridge), but they have quite a lot of usable shared
    // system memory. So add half of that as a ballpark if GPU does not have "decently enough"
    // dedicated VRAM. 512 was chosen somewhat arbitrarily as "decently enough".
    if (mb != 0 && mb < 512)
        mb += sharedSystem / (1024 * 1024) / 2;

    if (mb == 0)
        mb = dedicatedSystem / (1024 * 1024);
    if (mb == 0)
        mb = sharedSystem / (1024 * 1024);
    return mb;
}
