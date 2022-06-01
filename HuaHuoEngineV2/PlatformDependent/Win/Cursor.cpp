#include "UnityPrefix.h"
#include "Runtime/Input/Cursor.h"
#include "Runtime/Input/CursorImpl.h"
#include "Runtime/GfxDevice/GfxDevice.h"
#include "Runtime/Graphics/ScreenManager.h"
#include "Runtime/Graphics/Image.h"
#include "Runtime/Input/InputManager.h"
#include "Runtime/Misc/PlayerSettings.h"
#include "Runtime/Misc/SystemInfo.h"
#include "Runtime/Utilities/File.h"

#include "ComPtr.h"
#include "CoTaskMemHolder.h"
#include "WinUnicode.h"
#include "WinUtils.h"

#include <windows.h>
#include <Knownfolders.h>
#include <Shlobj.h>
#include <Shobjidl.h>


#if PLATFORM_SUPPORTS_HARDWARE_CURSORS

namespace Cursors
{
    // Max cursors to cache. Windows has a limitation of around 3300 of icons
    // a number is quite random here: something bug enough for user and small enough to not cause problems
    const int kMaxCursorCacheSize = 500;

    typedef UnityCursor<HCURSOR> WinCursor;
    typedef CursorManager<WinCursor> WinCursorManager;

    template<> WinCursorManager * WinCursorManager::s_CursorManager = NULL;

    // Save cursor to *.cur file, it's expected imageData format to be RGBA32
    bool SaveCursorToFile(const core::string& path, const UINT8* imageData, int width, int height, const Vector2f& hotspot)
    {
        // More info //https://en.wikipedia.org/wiki/ICO_(file_format)
        typedef struct
        {
            WORD idReserved; // must be 0
            WORD idType; // 1 = ICON, 2 = CURSOR
            WORD idCount; // number of images (and CURSORDIR)
        } CURSORHEADER;


        typedef struct
        {
            BYTE bWidth;
            BYTE bHeight;
            BYTE bColorCount;
            BYTE bReserved;
            WORD hotSpotX;
            WORD hotSpotY;
            DWORD dwBytesInRes;
            DWORD dwImageOffset; // file-offset to the start of IMAGE
        } CURSORDIR;

        File file;
        if (!file.Open(path, kWritePermission))
            return false;

        // Write cursor header
        CURSORHEADER iconheader;
        iconheader.idReserved = 0;
        iconheader.idType = 2;
        iconheader.idCount = 1;
        if (!file.Write(&iconheader, sizeof(iconheader)))
            return false;

        // Write cursor image header data
        int imageSize = width * height * 4 * 2; // Color + Mono
        CURSORDIR cursorDir;
        cursorDir.bWidth = (BYTE)width;
        cursorDir.bHeight = (BYTE)height;
        cursorDir.bColorCount = 0;
        cursorDir.bReserved = 0;
        cursorDir.hotSpotX = hotspot.x;
        cursorDir.hotSpotY = hotspot.y;
        cursorDir.dwBytesInRes = sizeof(BITMAPINFOHEADER) + imageSize;
        cursorDir.dwImageOffset = sizeof(CURSORHEADER) + sizeof(CURSORDIR);
        if (!file.Write(&cursorDir, sizeof(cursorDir)))
            return false;

        // Write the actual image
        BITMAPINFOHEADER biHeader;
        ZeroMemory(&biHeader, sizeof(biHeader));
        biHeader.biSize = sizeof(biHeader);
        biHeader.biWidth = width;
        biHeader.biHeight = height * 2; // height of color+mono
        biHeader.biPlanes = 1;
        biHeader.biBitCount = 32;
        biHeader.biSizeImage = imageSize;
        if (!file.Write(&biHeader, sizeof(biHeader)))
            return false;

        for (size_t y = 0; y < height; y++)
        {
            for (size_t x = 0; x < width; x++)
            {
                const UINT8* pixel = &imageData[(y * width + x) * 4];
                DWORD colorARGB =
                    pixel[3] << 24 |
                    pixel[0] << 16 |
                    pixel[1] << 8 |
                    pixel[2];

                file.Write(&colorARGB, sizeof(colorARGB));
            }
        }

        dynamic_array<UINT8> mono(kMemTempAlloc);
        mono.resize_initialized(imageSize, 255);
        if (!file.Write(&mono[0], imageSize))
            return false;

        return file.Close();
    }

#if UNITY_EDITOR
    bool SaveCursorToFile(const core::string& path, Texture2D* texture, const Vector2f& hotSpot)
    {
        ImageReference image;
        if (!texture->GetWriteImageReference(&image, 0, 0))
            return false;

        Image dstImage(image.GetWidth(), image.GetHeight(), kFormatR8G8B8A8_UNorm);
        dstImage.BlitImage(image, kImageBlitBilinearScale);

        return SaveCursorToFile(path, dstImage.GetImageData(), dstImage.GetWidth(), dstImage.GetHeight(), hotSpot);
    }

    bool SaveCursorToInMemoryResource(Texture2D* texture, const Vector2f& hotspot, UInt16 cursorDataResourceId, void* cursorDirectoryBuffer, UInt32 cursorDirectoryBufferSize, void* cursorDataBuffer, UInt32 cursorDataBufferSize)
    {
#pragma pack(push, 1)
        struct CURSORHEADER
        {
            WORD idReserved; // must be 0
            WORD idType; // 1 = ICON, 2 = CURSOR
            WORD idCount; // number of images (and CURSORDIR)
        };

        // I wrote the following two by gazing long enough at cursor resource bytes
        // extracted from an executable built by link.exe and manipulating the source
        // cursor image file. Eventually, it gazed back at me.
        //
        // This struct is similar to GRPICONDIRENTRY (described here: https://blogs.msdn.microsoft.com/oldnewthing/20120720-00/?p=7083/),
        // but it's for cursors instead and has a little bit different members
        // It also is described here, however, the types are mangled in their docs: https://docs.microsoft.com/en-us/windows/desktop/menurc/resdir
        // From remarks:
        //      One or more RESDIR structures immediately follow the NEWHEADER structure in the.res file.
        //      The ResCount member of the NEWHEADER structure specifies the number of RESDIR structures.
        //      Note that the RESDIR structure consists of either an ICONRESDIR structure or a CURSORDIR
        //      structure followed by the Planes, BitCount, BytesInRes, and IconCursorId members.
        struct CursorDirectoryEntry
        {
            WORD width;
            WORD height;
            WORD planeCount;
            WORD bitsPerPixel;
            DWORD sizeOfCursorResource;
            WORD cursorDataResourceId;
        };

        struct CursorDirectory
        {
            CURSORHEADER cursorHeader;
            CursorDirectoryEntry cursorEntries[1];
        };

        struct CursorData
        {
            WORD hotspotX;
            WORD hotspotY;
            BITMAPINFOHEADER bitmapHeader;
            BYTE data[1];
        };
#pragma pack(pop)

        // Basic buffer size checks
        if (cursorDirectoryBufferSize != sizeof(CursorDirectory))
            return false;

        ImageReference image;
        if (!texture->GetWriteImageReference(&image, 0, 0))
            return false;

        Image rgbaImage(image.GetWidth(), image.GetHeight(), kFormatR8G8B8A8_UNorm);
        rgbaImage.BlitImage(image, kImageBlitBilinearScale);

        const UInt32 width = rgbaImage.GetWidth();
        const UInt32 height = rgbaImage.GetHeight();
        const UInt32 imageSize = 4 /* bytes per pixel */ * width * height;
        const UInt32 cursorDataSize = 2 * sizeof(WORD) + sizeof(BITMAPINFOHEADER) + imageSize;
        if (cursorDataBufferSize != cursorDataSize)
            return false;

        auto cursorDirectory = reinterpret_cast<CursorDirectory*>(cursorDirectoryBuffer);
        cursorDirectory->cursorHeader.idReserved = 0;
        cursorDirectory->cursorHeader.idType = 2;
        cursorDirectory->cursorHeader.idCount = 1;
        cursorDirectory->cursorEntries[0].width = width;
        cursorDirectory->cursorEntries[0].height = 2 * height; // Note: the header actually contains double icon height (case 1162612)
        cursorDirectory->cursorEntries[0].planeCount = 1;
        cursorDirectory->cursorEntries[0].bitsPerPixel = 32;
        cursorDirectory->cursorEntries[0].sizeOfCursorResource = cursorDataSize;
        cursorDirectory->cursorEntries[0].cursorDataResourceId = cursorDataResourceId;

        auto cursorData = reinterpret_cast<CursorData*>(cursorDataBuffer);
        cursorData->hotspotX = static_cast<UInt16>(hotspot.x);
        cursorData->hotspotY = static_cast<UInt16>(hotspot.y);

        ZeroMemory(&cursorData->bitmapHeader, sizeof(cursorData->bitmapHeader));
        cursorData->bitmapHeader.biSize = sizeof(cursorData->bitmapHeader);
        cursorData->bitmapHeader.biWidth = width;
        cursorData->bitmapHeader.biHeight = 2 * height;
        cursorData->bitmapHeader.biPlanes = 1;
        cursorData->bitmapHeader.biBitCount = 32;
        cursorData->bitmapHeader.biSizeImage = imageSize;

        memcpy(cursorData->data, rgbaImage.GetImageData(), imageSize);

        // Convert from RGBA to BGRA
        UInt32 pixelCount = width * height;
        for (UInt32 i = 0; i < pixelCount; i++)
        {
            UInt32& pixel = reinterpret_cast<UInt32&>(cursorData->data[i * 4]);
            pixel = (pixel & 0xFF00FF00) | ((pixel & 0x00FF0000) >> 16) | ((pixel & 0xFF) << 16);
        }

        return true;
    }

#endif

    static bool TrySaveCursorToTempFile(const wchar_t* folder, const UINT8* imageData, int width, int height, Vector2f hotSpot, core::wstring& cursorPath)
    {
        auto folderAttributes = GetFileAttributesW(folder);
        if (folderAttributes == INVALID_FILE_ATTRIBUTES || (folderAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
        {
            ErrorStringMsg("Failed to save a temporary cursor file to '%s' because this directory does not exist.", WideToUtf8(folder).c_str());
            return false;
        }

        cursorPath.resize(MAX_PATH);
        if (GetTempFileNameW(folder, L"cur", 0, &cursorPath[0]) == 0)
        {
            auto error = GetLastError();
            ErrorStringMsg("Failed to save a temporary cursor file to directory at '%s': %s", WideToUtf8(folder).c_str(), winutils::ErrorCodeToMsg(error).c_str());
            return false;
        }

        auto nulCharIndex = cursorPath.find_first_of(L'\0');
        if (nulCharIndex != core::wstring::npos)
            cursorPath.resize(nulCharIndex);

        core::string cursorUtf8Path = WideToUtf8(cursorPath);
        if (!SaveCursorToFile(cursorUtf8Path, imageData, width, height, hotSpot))
        {
            ErrorStringMsg("Failed to save a temporary cursor file to '%s'.", cursorUtf8Path.c_str());
            return false;
        }

        return true;
    }

    static WinCursor LoadAndDeleteCursor(const core::wstring& path, Vector2f hotSpot)
    {
        WinCursor c;
        c.hotspot = hotSpot;
        c.sCursor = nullptr;
        c.hCursor = (HCURSOR)LoadImageW(nullptr, path.c_str(), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);

        if (c.hCursor == nullptr)
        {
            auto error = GetLastError();
            ErrorStringMsg("Failed to load cursor from '%s': %s", WideToUtf8(path).c_str(), winutils::ErrorCodeToMsg(error).c_str());

            // If we for whatever reason failed to load the cursor, fall back to the basic arrow.
            // That is better than the cursor going "poof, gone".
            c.hCursor = LoadCursor(nullptr, IDC_ARROW);
        }

        DeleteFileW(path.c_str());
        return c;
    }

    static WinCursor GenerateCursor(Texture2D* texture, Vector2f hotSpot, CursorMode cursorMode)
    {
        // if this is null you are doing it wrong
        Assert(texture);

        // looks like we have a software cursor :(
        if (cursorMode == kHardwareCursorOff)
        {
            // looks like we have a software cursor :(
            WinCursor c;
            c.sCursor = texture;
            c.hCursor = NULL;
            c.hotspot = hotSpot;
            return c;
        }

        ImageReference image;
        if (!texture->GetWriteImageReference(&image, 0, 0))
        {
            ErrorStringMsg("Failed to set the cursor because the specified texture ('%s') was not CPU accessible.", texture->GetName());

            WinCursor c;
            c.hotspot = hotSpot;
            c.sCursor = nullptr;
            c.hCursor = LoadCursor(nullptr, IDC_ARROW);
            return c;
        }

        Image cursorImage(image.GetWidth(), image.GetHeight(), kFormatR8G8B8A8_UNorm);
        cursorImage.BlitImage(image, kImageBlitBilinearScale);

        // Cursor loading on Windows is messed up. While you can load a cursor from memory via CreateCursor or LoadCursorIndirect,
        // neither support using the default cursor size for the system (scaled by DPI and accessibility settings). Instead,
        // you have to use LoadImageW with the LR_DEFAULTSIZE flag. LoadImageW can either load the cursor from a file or from a
        // loaded module resource directory. LR_DEFAULTSIZE flag is also supported by the CreateIconFromResourceEx function, however,
        // it doesn't go through the same path as LoadImageW and ends up loading the cursor at 32x32, ignoring any DPI or accessibility
        // settings.
        //
        // I stepped through LoadImageW to see how it determines the proper size in hopes that I could do the same and then load
        // it from memory but it uses undocumented functionality. On Windows 7, it looks inside the global variable user32.dll!gpsi.
        // On a recent Windows 10 version, it calls ntdll.dll!NtUserGetRequiredCursorSizes. I couldn't find a supported way to
        // determine the default cursor size for the system.
        //
        // Since we need to be able to load arbitrary cursors at runtime, we have no choice but to save the cursor image to a file
        // and point LoadImageW at it. Unfortunately, this can fail for variety of reasons (the process might be launched with weird
        // permissions, the disk is full, %TMP% environment variable is misconfigured and pointing at a non-existant folder, etc.
        // Due to that, we try to save the cursor to a variety of folders before giving up.
        //
        // Another approach to making this work would be to have UnityPlayer.dll linker produce a special read-write section in the
        // binary that would contain an empty resource. We would then write the cursor bytes there and have LoadImageW load it
        // from that. However, this approach seems complicated so I decided not to pursue it.
        {
            wchar_t tempPath[MAX_PATH + 1]; // From MSDN: The maximum possible return value is MAX_PATH+1
            if (GetTempPathW(ARRAYSIZE(tempPath), tempPath) != 0)
            {
                core::wstring cursorPath(kMemTempAlloc);
                if (TrySaveCursorToTempFile(tempPath, cursorImage.GetImageData(), cursorImage.GetWidth(), cursorImage.GetHeight(), hotSpot, cursorPath))
                    return LoadAndDeleteCursor(cursorPath, hotSpot);
            }
            else
            {
                auto error = GetLastError();
                ErrorStringMsg("Failed to retrieve TEMP path: ", winutils::ErrorCodeToMsg(error).c_str());
            }
        }

        // TEMP path is either invalid or we don't have write access. Try creating the cursor in a few other places.
        win::ComPtr<IKnownFolderManager> folderManager;
        auto hr = CoCreateInstance(CLSID_KnownFolderManager, NULL, CLSCTX_INPROC_SERVER, __uuidof(folderManager), &folderManager);
        if (SUCCEEDED(hr))
        {
            const KNOWNFOLDERID* kFolderIds[] =
            {
                &FOLDERID_LocalAppData,
                &FOLDERID_LocalAppDataLow, // Perhaps we're running as a low integrity process?

                // Some random last resort fallbacks
                &FOLDERID_Public,
                &FOLDERID_Documents,
                &FOLDERID_SavedGames,
            };

            for (auto folderId : kFolderIds)
            {
                win::ComPtr<IKnownFolder> folder;
                hr = folderManager->GetFolder(*folderId, &folder);
                if (SUCCEEDED(hr))
                {
                    CoTaskMemHolder<wchar_t> folderPath;
                    hr = folder->GetPath(KF_FLAG_CREATE | KF_FLAG_DONT_UNEXPAND, &folderPath);
                    if (SUCCEEDED(hr))
                    {
                        core::wstring cursorPath(kMemTempAlloc);
                        if (TrySaveCursorToTempFile(folderPath, cursorImage.GetImageData(), cursorImage.GetWidth(), cursorImage.GetHeight(), hotSpot, cursorPath))
                            return LoadAndDeleteCursor(cursorPath, hotSpot);
                    }
                }
            }
        }

        // Saving cursor to a file failed. Ugh! Fallback to default arrow cursor to avoid the cursor from becoming invisible
        WinCursor c;
        c.hotspot = hotSpot;
        c.sCursor = nullptr;
        c.hCursor = LoadCursor(nullptr, IDC_ARROW);
        return c;
    }

    Texture2D* GetSoftwareCursor()
    {
        return WinCursorManager::Instance().GetSoftwareCursor();
    }

    Vector2f GetCursorHotspot()
    {
        return WinCursorManager::Instance().GetCursorHotspot();
    }

    HCURSOR GetHardwareCursor()
    {
        return WinCursorManager::Instance().GetHardwareCursor();
    }

    void ResetCursor()
    {
        if (!GetScreenManager().GetCursorInsideWindow())
            return;

        // If we're currently using a software cursor, then hCursor will be NULL which will hide it
        ::SetCursor(WinCursorManager::Instance().GetHardwareCursor());
    }

    void TrackMouseEvent()
    {
        TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT) };
        tme.dwFlags = TME_LEAVE;
        tme.hwndTrack = ::GetScreenManager().GetWindow();
        TrackMouseEvent(&tme);
    }

    bool HandleMouseCursor(UINT message, LPARAM lParam)
    {
        if (!GetScreenManager().GetCursorInsideWindow())
            return false;

        if (message == WM_SETCURSOR || message == WM_MOUSEMOVE)
            ResetCursor();

        if (message == WM_SETCURSOR)
            return true;

        return false;
    }

    void InitializeCursors(Texture2D* defaultCursorTexture, Vector2f defaultCursorHotSpot)
    {
        WinCursorManager& manager = WinCursorManager::Instance();
        if (!defaultCursorTexture)
        {
            manager.m_DefaultCursor.hCursor = ::LoadCursor(NULL, IDC_ARROW);
            manager.m_DefaultCursor.sCursor = NULL;
            manager.m_UsingBuiltinDefaultCursor = true;
        }
        else
        {
            manager.m_DefaultCursor = GenerateCursor(defaultCursorTexture, defaultCursorHotSpot, kAutoHardwareCursor);
            manager.m_UsingBuiltinDefaultCursor = false;
        }
        manager.m_CurrentCursor = manager.m_DefaultCursor;
    }

    void CleanupCursors()
    {
        // go through and destroy all the generated cursors
        WinCursorManager& manager = WinCursorManager::Instance();
        if (!manager.m_UsingBuiltinDefaultCursor && manager.m_DefaultCursor.hCursor != NULL)
            ::DestroyCursor(manager.m_DefaultCursor.hCursor);

        for (WinCursorManager::CursorCache::iterator itr = manager.m_CursorCache.begin(); itr != manager.m_CursorCache.end(); ++itr)
        {
            HCURSOR hcur = (*itr).second.hCursor;
            if (hcur)
            {
                ::DestroyCursor(hcur);
            }
        }

        WinCursorManager::Cleanup();
    }

    void SetCursor(Texture2D* texture, Vector2f hotSpot, CursorMode cursorMode)
    {
        WinCursorManager& manager = WinCursorManager::Instance();
        if (!texture)
        {
            manager.m_CurrentCursor = manager.m_DefaultCursor;
            ResetCursor();
            return;
        }

        // try and find the cursor in the cache
        WinCursorManager::CursorCache::const_iterator found = manager.m_CursorCache.find(texture->GetTextureID());
        WinCursor cursorToSet;
        bool shouldGenerateCursor = true;
        if (manager.m_CursorCache.end() != found)
        {
            // if we found a cursor we need to see if the desired mode (hardware / software)
            // is the same as the one requested now...
            // if it's not then delete the old cursor and recreate it!
            cursorToSet = found->second;

            if ((cursorMode == kAutoHardwareCursor && cursorToSet.hCursor == NULL)
                || (cursorMode == kHardwareCursorOff && cursorToSet.sCursor.IsNull())
                || !CompareApproximately(hotSpot.x, cursorToSet.hotspot.x)
                || !CompareApproximately(hotSpot.y, cursorToSet.hotspot.y))
            {
                if (found->second.hCursor)
                    ::DestroyCursor(found->second.hCursor);
                manager.m_CursorCache.erase(found);
            }
            else
            {
                shouldGenerateCursor = false;
            }
        }

        if (shouldGenerateCursor)
        {
            // control the cursor cache size, as if we create somewhere around 3300 cursors, it starts to fail creating new ones and breaks Windows dialogs etc.
            while (manager.m_CursorCache.size() >= kMaxCursorCacheSize)
            {
                Cursors::WinCursorManager::CursorCache::iterator iter = manager.m_CursorCache.begin();
                DestroyIcon(iter->second.hCursor);
                manager.m_CursorCache.erase(iter);
            }
            cursorToSet = GenerateCursor(texture, hotSpot, cursorMode);
            manager.m_CursorCache[texture->GetTextureID()] = cursorToSet;
        }

        manager.m_CurrentCursor = cursorToSet;
        ResetCursor();
    }
} //namespace
#endif
