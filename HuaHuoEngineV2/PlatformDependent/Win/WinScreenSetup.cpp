#include "UnityPrefix.h"
#include "WinScreenSetup.h"
#include "ErrorMessages.h"
#include "WinUtils.h"
#include "WinLib.h"
#include "Runtime/GfxDevice/GfxDevice.h"
#include "Runtime/Graphics/DisplayManager.h"
#include "Runtime/Graphics/ScreenManager.h"
#include "Runtime/Misc/PlayerSettings.h"
#include "Runtime/Utilities/PlayerPrefs.h"
#include "Runtime/GfxDevice/dxgi/HDRDisplayUtils.h"
#if GFX_SUPPORTS_D3D11
#include "Runtime/GfxDevice/d3d11/D3D11Context.h"
#endif
#if GFX_SUPPORTS_D3D12
#include "Runtime/GfxDevice/d3d12/D3D12Context.h"
#endif
#if GFX_SUPPORTS_VULKAN
#include "Runtime/GfxDevice/vulkan/VKContext.h"
#endif
#include "Runtime/Graphics/QualitySettings.h"
#if GFX_SUPPORTS_OPENGL_UNIFIED
#include "Runtime/GfxDevice/opengles/ApiGLES.h"
#include "Runtime/GfxDevice/opengles/ApiConstantsGLES.h"
#include "Runtime/GfxDevice/opengles/FramebufferGLES.h"
#include "Runtime/Graphics/GraphicsHelper.h"
#include "Runtime/Graphics/RenderTexture.h"

// In GfxDeviceGLES.cpp
namespace gles
{
    GfxFramebufferGLES &GetFramebufferGLES();
}

#endif

#include <limits>
#if !UNITY_EDITOR
#include "Runtime/Utilities/Argv.h"
#endif


#define ENABLE_DWM_API \
    !UNITY_EDITOR // We are enabling the DWM API for the standalone app

#if ENABLE_DWM_API
#include <Dwmapi.h> //  Desktop Window Manager (DWM) API
#endif

#ifndef DPI_ENUMS_DECLARED

typedef enum PROCESS_DPI_AWARENESS
{
    PROCESS_DPI_UNAWARE = 0,
    PROCESS_SYSTEM_DPI_AWARE = 1,
    PROCESS_PER_MONITOR_DPI_AWARE = 2
} PROCESS_DPI_AWARENESS;

typedef enum MONITOR_DPI_TYPE
{
    MDT_EFFECTIVE_DPI = 0,
    MDT_ANGULAR_DPI = 1,
    MDT_RAW_DPI = 2,
    MDT_DEFAULT = MDT_EFFECTIVE_DPI
} MONITOR_DPI_TYPE;

#define DPI_ENUMS_DECLARED
#endif // (DPI_ENUMS_DECLARED)

const int kMinResolutionW = 512;
const int kMinResolutionH = 384;
const int kMinResolutionBpp = 15;

bool g_PopUpWindow = false;
bool g_DontRepositionWindow = false;


#if ENABLE_DWM_API
typedef HRESULT (WINAPI *DwmGetWindowAttributeFuncPointer)(HWND, DWORD, PVOID, DWORD);
typedef HRESULT (WINAPI *DwmSetWindowAttributeFuncPointer)(HWND, DWORD, LPCVOID, DWORD);

static DwmGetWindowAttributeFuncPointer g_DwmGetWindowAttributeFunc = NULL;
static DwmSetWindowAttributeFuncPointer g_DwmSetWindowAttributeFunc = NULL;

inline bool IsDWMEnabled()
{
    return NULL != g_DwmGetWindowAttributeFunc && NULL != g_DwmSetWindowAttributeFunc;
}

#endif // ENABLE_DWM_API


// When Windows 7 starts a windows in the exclusive mode
// and we later switch to the windowed mode, the Aero theme is not enabled (case 431740).
// This checks if the Aero theme is enabled, and if it is not -- tries to enable the theme.
static void SetWindowTitlebarToAero(HWND window)
{
    #if ENABLE_DWM_API
    if (!IsDWMEnabled())
        return;

    BOOL enabled;
    HRESULT dwm_hresult = g_DwmGetWindowAttributeFunc(window, DWMWA_NCRENDERING_ENABLED, &enabled, sizeof(BOOL));

    if (SUCCEEDED(dwm_hresult))
    {
        // Use the retrieved information.
        if (!enabled)
        {
            // Enable non-client area rendering on the window.
            // (we don't check the result because, frankly, the result not important)
            DWMNCRENDERINGPOLICY ncrp = DWMNCRP_ENABLED;
            g_DwmSetWindowAttributeFunc(window, DWMWA_NCRENDERING_POLICY, &ncrp, sizeof(ncrp));
        }
    }
    #endif // ENABLE_DWM_API
}

class ResolutionSort : public std::binary_function<const WinScreenSetup::WinResolution&, const WinScreenSetup::WinResolution&, bool>
{
public:
    bool operator()(const WinScreenSetup::WinResolution& lhs, const WinScreenSetup::WinResolution& rhs) const
    {
        // always list non-rotated resolutions first
        if (!lhs.IsRotated() && rhs.IsRotated())
            return true;

        if (lhs.width != rhs.width)
            return lhs.width < rhs.width;
        return lhs.height < rhs.height;
    }
};

static int CrackDisplayFrequency(const DEVMODEW& dm)
{
    // return zero for "default" values
    if (!(dm.dmFields & DM_DISPLAYFREQUENCY))
        return 0;
    if ((dm.dmFields & DM_DISPLAYFREQUENCY) && dm.dmDisplayFrequency == 1)
        return 0;
    // else return the frequency
    return dm.dmDisplayFrequency;
}

WinScreenSetup::WinScreenSetup()
    :   m_Window(NULL)
    ,   m_VirtualDesktopX(0)
    ,   m_VirtualDesktopY(0)
    ,   m_VirtualDesktopWidth(0)
    ,   m_VirtualDesktopHeight(0)
    ,   m_Width(0)
    ,   m_RealWidth(0)
    ,   m_WindowWidth(0)
    ,   m_Height(0)
    ,   m_RealHeight(0)
    ,   m_WindowHeight(0)
    ,   m_FullscreenMode(kWindowed)
    ,   m_Stereoscopic(false)
    ,   m_MultiDisplays(false)
    ,   m_PendingModeChange(false)
    ,   m_FailedToApplyRequestedResolution(false)
    ,   m_LastRequestedExclusiveFullscreenRefreshRate(0)
    ,   m_CoordinateScale(1.0f, 1.0f)
    ,   m_DPI(USER_DEFAULT_SCREEN_DPI)
#if !UNITY_EDITOR && GFX_SUPPORTS_OPENGL_UNIFIED
    ,   m_UseFullscreenUpscalingGL(false)
    ,   m_TargetColor(0)
    ,   m_TargetDepth(0)
    ,   m_SystemColor(0)
    ,   m_SystemDepth(0)
#endif//!UNITY_EDITOR && GFX_SUPPORTS_OPENGL
{
    winlib::GetShcore().Load();
    UpdateVirtualDesktopBounds();

#if ENABLE_DWM_API
    if (!IsDWMEnabled())
    {
        HMODULE dwm_module = GetModuleHandleW(L"Dwmapi.dll");
        // if the handle is not NULL, then it means that we are running on at least Windows Vista.
        if (NULL != dwm_module)
        {
            g_DwmGetWindowAttributeFunc = (DwmGetWindowAttributeFuncPointer)GetProcAddress(dwm_module, "DwmGetWindowAttribute");
            g_DwmSetWindowAttributeFunc = (DwmSetWindowAttributeFuncPointer)GetProcAddress(dwm_module, "DwmSetWindowAttribute");
        }
    }
#endif // ENABLE_DWM_API
}

WinScreenSetup::~WinScreenSetup()
{
#   if !UNITY_EDITOR && GFX_SUPPORTS_OPENGL_UNIFIED
    if (IsGfxDevice() && (GetGfxDevice().GetRenderer() == kGfxRendererOpenGLCore))
        ReleaseUpscaledFramebufferGL(true);
#   endif//!UNITY_EDITOR && GFX_SUPPORTS_OPENGL
}

void WinScreenSetup::UpdateVirtualDesktopBounds()
{
    // get virtual desktop size
    m_VirtualDesktopX = ::GetSystemMetrics(SM_XVIRTUALSCREEN);
    m_VirtualDesktopY = ::GetSystemMetrics(SM_YVIRTUALSCREEN);
    m_VirtualDesktopWidth = ::GetSystemMetrics(SM_CXVIRTUALSCREEN);
    m_VirtualDesktopHeight = ::GetSystemMetrics(SM_CYVIRTUALSCREEN);
}

void WinScreenSetup::InvalidateResolutions()
{
    m_Resolutions.clear_dealloc();
}

typedef HRESULT (WINAPI* LPCREATEDXGIFACTORY)(REFIID, void**);
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=NULL; } }
#endif

#include "DXGI.h"

static int FindOrAddResolution(dynamic_array<WinScreenSetup::WinResolution>& resolutions, int width, int height);
static Rational FindBestRefresh(const WinScreenSetup::WinResolution& resolution, int preferredRefreshRate, int desktopRefreshRate);
static void FillResolutionsDXGI(dynamic_array<WinScreenSetup::WinResolution>& resolutions, HMONITOR monitor);

const dynamic_array<WinScreenSetup::WinResolution>& WinScreenSetup::GetResolutions(HMONITOR monitor)
{
    core::hash_map<HMONITOR, dynamic_array<WinResolution> >::const_iterator it = m_Resolutions.find(monitor);
    if (it != m_Resolutions.end())
        return it->second;

    DebugAssertMsg(IsGfxDevice(), "Cannot get resolutions before GfxDevice is initialized!");

    GfxDevice& device = GetGfxDevice();
    dynamic_array<WinScreenSetup::WinResolution>& resolutions = m_Resolutions[monitor];
    FillResolutionsDXGI(resolutions, monitor);

    // sort the modes
    std::sort(resolutions.begin(), resolutions.end(), ResolutionSort());
    return resolutions;
}

static win::ComPtr<IDXGIOutput> FindDxgiOutputFromMonitor(HINSTANCE hDXGI, HMONITOR monitor)
{
    DebugAssertMsg(hDXGI != NULL, "NULL hDXGI instance was passed to FindDxgiOutputFromMonitor");
    LPCREATEDXGIFACTORY pCreateDXGIFactory = (LPCREATEDXGIFACTORY)GetProcAddress(hDXGI, "CreateDXGIFactory");
    if (pCreateDXGIFactory == NULL)
        return NULL;

    win::ComPtr<IDXGIFactory> pDXGIFactory;
    HRESULT hr = pCreateDXGIFactory(__uuidof(IDXGIFactory), &pDXGIFactory);
    if (FAILED(hr))
        return NULL;

    for (int index = 0;; ++index)
    {
        win::ComPtr<IDXGIAdapter> pAdapter = NULL;
        hr = pDXGIFactory->EnumAdapters(index, &pAdapter);
        if (FAILED(hr)) // DXGIERR_NOT_FOUND is expected when the end of the list is hit
            break;

        for (int iOutput = 0;; ++iOutput)
        {
            win::ComPtr<IDXGIOutput> pOutput = NULL;
            hr = pAdapter->EnumOutputs(iOutput, &pOutput);
            if (FAILED(hr)) // DXGIERR_NOT_FOUND is expected when the end of the list is hit
                break;

            DXGI_OUTPUT_DESC outputDesc;
            ZeroMemory(&outputDesc, sizeof(DXGI_OUTPUT_DESC));
            if (SUCCEEDED(pOutput->GetDesc(&outputDesc)) && outputDesc.Monitor == monitor)
                return pOutput;
        }
    }

    return NULL;
}

struct MonitorEnumDataForFindingDisplayMode
{
    HMONITOR monitor;
    WinScreenSetup::WinResolution result;
    bool success;
};

BOOL CALLBACK MonitorEnumProcForFindingDisplayMode(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
    MonitorEnumDataForFindingDisplayMode* data = reinterpret_cast<MonitorEnumDataForFindingDisplayMode*>(dwData);

    if (hMonitor != data->monitor)
        return TRUE; // Continue enumeration

    data->result.width = GetDeviceCaps(hdcMonitor, HORZRES);
    data->result.height = GetDeviceCaps(hdcMonitor, VERTRES);
    data->result.refreshRates.push_back(Rational{ static_cast<uint32_t>(GetDeviceCaps(hdcMonitor, VREFRESH)), 1 });
    data->success = true;

    return FALSE; // Stop enumeration
}

static void AddDesktopResolutionFromGDI(dynamic_array<WinScreenSetup::WinResolution>& resolutions, HMONITOR monitor)
{
    HDC deskDC = ::GetDC(GetDesktopWindow());

    MonitorEnumDataForFindingDisplayMode enumerationData;
    enumerationData.monitor = monitor;
    enumerationData.success = false;

    EnumDisplayMonitors(deskDC, NULL, MonitorEnumProcForFindingDisplayMode, reinterpret_cast<LPARAM>(&enumerationData));

    if (enumerationData.success)
        resolutions.push_back(enumerationData.result);

    // done with desktop DC
    ReleaseDC(GetDesktopWindow(), deskDC);
}

static void FillResolutionsDXGI(dynamic_array<WinScreenSetup::WinResolution>& resolutions, HMONITOR monitor)
{
    // Use DXGI for proper enumeration
    HINSTANCE hDXGI = LoadLibraryW(L"dxgi.dll");
    if (hDXGI)
    {
        win::ComPtr<IDXGIOutput> dxgiOutput = FindDxgiOutputFromMonitor(hDXGI, monitor);
        if (dxgiOutput != NULL)
        {
            UINT numModes = 0;
            // We'll just assume the same resolutions are available for other color formats as well
            HRESULT hr = dxgiOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &numModes, NULL);

            if (SUCCEEDED(hr) && numModes != 0)
            {
                dynamic_array<DXGI_MODE_DESC> pDesc(kMemTempAlloc);
                pDesc.resize_uninitialized(numModes);

                hr = dxgiOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &numModes, pDesc.data());
                if (SUCCEEDED(hr))
                {
                    for (UINT i = 0; i < numModes; i++)
                    {
                        int resIdx = FindOrAddResolution(resolutions, pDesc[i].Width, pDesc[i].Height);
                        if (resIdx >= 0)
                        {
                            WinScreenSetup::WinResolution& res = resolutions[resIdx];
                            res.refreshRates.push_back(Rational{ pDesc[i].RefreshRate.Numerator, pDesc[i].RefreshRate.Denominator });
                        }
                    }
                }
            }
        }

        dxgiOutput = NULL; // Can't let destructor free it after we free dxgi.dll
        FreeLibrary(hDXGI);
    }

    // if no resolutions - get desktop one
    if (resolutions.empty())
        AddDesktopResolutionFromGDI(resolutions, monitor);
}

#if UNITY_EDITOR
void WinScreenSetup::BoundRectangleToDesktops(RECT& rc) const
{
    const int virtualDesktopRight = m_VirtualDesktopX + m_VirtualDesktopWidth;
    const int virtualDesktopBottom = m_VirtualDesktopY + m_VirtualDesktopHeight;

    // bound size
    if (rc.right - rc.left > m_VirtualDesktopWidth)
        rc.right = rc.left + m_VirtualDesktopWidth;
    if (rc.bottom - rc.top > m_VirtualDesktopHeight)
        rc.bottom = rc.top + m_VirtualDesktopHeight;
    // bound right/lower side
    if (rc.right > virtualDesktopRight)
        OffsetRect(&rc, virtualDesktopRight - rc.right, 0);
    if (rc.bottom > virtualDesktopBottom)
        OffsetRect(&rc, 0, virtualDesktopBottom - rc.bottom);
    // bound left/top side
    if (rc.left < m_VirtualDesktopX)
        OffsetRect(&rc, m_VirtualDesktopX - rc.left, 0);
    if (rc.top < m_VirtualDesktopY)
        OffsetRect(&rc, 0, m_VirtualDesktopY - rc.top);
}

#endif

static int FindOrAddResolution(dynamic_array<WinScreenSetup::WinResolution>& resolutions, int width, int height)
{
    size_t n = resolutions.size();
    for (size_t i = 0; i < n; ++i)
    {
        const WinScreenSetup::WinResolution& r = resolutions[i];
        if (r.width == width && r.height == height)
            return i; // found one, return index
    }

    // don't add resolutions that are excluded from player settings
    const PlayerSettings* settings = GetPlayerSettingsPtr();
    if (settings && !settings->DoesSupportResolution(width, height))
        return -1;

    // not found - add and return index
    WinScreenSetup::WinResolution res;
    res.width = width;
    res.height = height;
    resolutions.push_back(res);
    return resolutions.size() - 1;
}

static Rational FindBestRefresh(const WinScreenSetup::WinResolution& resolution, int preferredRefreshRate, int defaultRefreshRate)
{
    // Find closest to desktop if no preferred rate
    if (preferredRefreshRate == 0)
        preferredRefreshRate = defaultRefreshRate;

    Rational bestRate = { 0, 1 };
    float bestDist = std::numeric_limits<float>::max();

    for (const auto& refreshRate : resolution.refreshRates)
    {
        if (refreshRate.Denominator == 0)
            continue;

        float dist = fabs(preferredRefreshRate - static_cast<float>(refreshRate.Numerator) / static_cast<float>(refreshRate.Denominator));
        if (dist < bestDist)
        {
            bestRate = refreshRate;
            bestDist = dist;
        }
    }

    return bestRate;
}

static int FindClosestResolution(const dynamic_array<WinScreenSetup::WinResolution>& resolutions, const Resolution& desktopResolution, int width, int height, bool fullscreen)
{
    // Tricky parts: some drivers report "rotated" resolutions for use with rotated LCDs.
    // We penalize automatic selection of those by multiplying the score (so it would still
    // choose rotated one if that was requested exactly, e.g. 480x640).

    // Then some (MacBook Intel GMA950 on windows) report resolutions that are larger than the monitor
    // can handle, and the driver uses lower one and does magic panning of the view area.
    // So we penalize selection of resolutions larger than desktop by multiplying again.

    // For windowed, we penalize window sizes larger than desktop by adding a big number to
    // the score, so that window we create always fits into the view. Don't just skip those
    // resolutions; we need them for extreme cases where there are no other resolutions to
    // choose from.

    const int kTooBigForWindowedPenalty = 1024 * 1024;
    const int kTooBigForFullscreenMultiplier = 8;
    const int kRotatedPenaltyMultiplier = 16;

    int minDistance = std::numeric_limits<int>::max();
    int index = 0;
    size_t n = resolutions.size();
    for (size_t i = 0; i < n; ++i)
    {
        const WinScreenSetup::WinResolution& res = resolutions[i];
        int distance = abs(width - res.width) + abs(height - res.height);
        // Important: for windowed, "too big" is >= than desktop; for fullscreen "too big" is > than desktop.
        if (!fullscreen)
        {
            bool tooBig = res.width >= desktopResolution.width || res.height >= desktopResolution.height;
            if (tooBig)
                distance += kTooBigForWindowedPenalty;
        }
        else
        {
            bool tooBig = res.width > desktopResolution.width || res.height > desktopResolution.height;
            if (tooBig)
            {
                distance *= kTooBigForFullscreenMultiplier;
            }
        }
        if (res.IsRotated())
            distance *= kRotatedPenaltyMultiplier;
        if (distance < minDistance)
        {
            index = i;
            minDistance = distance;
        }
    }

    return index;
}

DWORD GetStandaloneWindowedStyle()
{
    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    PlayerSettings* playerSettings = GetPlayerSettingsPtr();
    if (playerSettings && playerSettings->GetResizableWindow())
    {
        style |= WS_MAXIMIZEBOX | WS_THICKFRAME;
    }
    return style;
}

#if !UNITY_EDITOR

void WinScreenSetup::AdjustWindowStyleForModeChange(FullscreenMode fullscreenMode)
{
    //NESTED_LOG("DX11 debug", "AdjustWindowStyleForModeChange: fs=%i window %p", fullscreen, m_Window);

    // Unity application is embedded into another application, don't change the window style
    if (GetParent(m_Window) != NULL)
        return;

    DWORD style = GetWindowLong(m_Window, GWL_STYLE);
    style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
    const DWORD directstyle = (WS_POPUP);
    const DWORD windowstyle = GetStandaloneWindowedStyle();
    // We remove chrome for fullscreen and exclusive full screen explicitly here, rather than use IsModeFullscreen
    // because other modes may or may not need chrome (kMaximized window should have chrome, for example)
    if (fullscreenMode == kFullscreenWindow || fullscreenMode == kExclusiveFullscreen || g_PopUpWindow)
    {
        style &= ~windowstyle;
        style |= directstyle;
    }
    else
    {
        style &= ~directstyle;
        style |= windowstyle;
    }
    // If Multiple Monitor Support enabled, switch to fake-full-screen. Only if a secondary screen is activated through Script, the App becomes Fake-Full-Screen.
    if (m_MultiDisplays)
        style = WS_POPUP | WS_VISIBLE;

    SetWindowLong(m_Window, GWL_STYLE, style);
}

void WinScreenSetup::ResizeAndCenterWindowOnScreen(int width, int height, FullscreenMode fullscreenMode)
{
    WINDOWPLACEMENT wndPlacement{};
    wndPlacement.length = sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(m_Window, &wndPlacement);

    // If window currently maximized, restore it to a "normal" window before resizing/centering (case 1085673 & 1137204)
    // - Works around an issue were UI elements are misaligned with the mouse cursor
    // - Fixes other quirky behavior caused by the window remaining in a maximized state but doesn't fill the entire screen
    // NOTE: The original fix for case 1085673 would change display to Fullscreen mode when maximized button clicked, but
    // this led to case 1137204. So that change was removed and replaced with this fix.
    if (!IsModeFullscreen(fullscreenMode) && wndPlacement.showCmd == SW_MAXIMIZE)
    {
        ShowWindow(m_Window, SW_RESTORE);
    }

    RectInt displayCoord = GetDisplayCoordinates(GetMonitorForMainWindow());

    RECT bounds;
    bounds.left = displayCoord.x;
    bounds.top = displayCoord.y;
    bounds.right = displayCoord.x + width;
    bounds.bottom = displayCoord.y + height;

    if (fullscreenMode == kFullscreenWindow)
    {
        bounds.right = displayCoord.x + displayCoord.width;
        bounds.bottom = displayCoord.y + displayCoord.height;
    }

    AdjustWindowRectEx(&bounds, GetWindowLong(m_Window, GWL_STYLE), FALSE, 0);

    int windowWidth = bounds.right - bounds.left;
    int windowHeight = bounds.bottom - bounds.top;
    int windowX, windowY;

    // Center on selected monitor by default
    windowX = (displayCoord.width - windowWidth) / 2 + displayCoord.x;
    windowY = (displayCoord.height - windowHeight) / 2 + displayCoord.y;

    // In windowed mode, if a window is larger than the width and/or height of the monitor, align the left and/or top of the window with the left and/or top of the monitor (case 760215)
    // Make sure to not realign it to the primary monitor, as we don't want the window to suddenly jump there (case 881735)
    if (!IsModeFullscreen(fullscreenMode))
    {
        if (windowWidth > displayCoord.width)
        {
            windowX = displayCoord.x;
        }
        if (windowHeight > displayCoord.height)
        {
            windowY = displayCoord.y;
        }
    }

    // We need to specify NOTOPMOST if we are not in fullscreen exclusive mode (cases 633049, 1157039)
    HWND wndAfter = fullscreenMode == kExclusiveFullscreen ? HWND_TOPMOST : HWND_NOTOPMOST;
    UINT flags = SWP_NOCOPYBITS | SWP_NOACTIVATE;

    bool hideWindow = false;
    hideWindow |= HasARGV("hideWindow");
    hideWindow |= HasARGV("parentHWND") && StrIEquals(GetFirstValueForARGV("parentHWND"), "delayed");
    hideWindow |= IsBatchmode(); // when in batchmode we should always keep the window hidden (case 1183530)

    flags |= hideWindow ? SWP_HIDEWINDOW : SWP_SHOWWINDOW;

    {
        //NESTED_LOG("DX11 debug", "ResizeAndCenterWindowOnScreen window %p %dx%d", m_Window, windowWidth, windowHeight);

        // If window is minimized we need to restore it (case 611897). Don't call ShowWindow otherwise (case 630493).
        if (IsModeFullscreen(fullscreenMode) && wndPlacement.showCmd != SW_SHOWNORMAL)
        {
            ShowWindow(m_Window, SW_SHOWNORMAL);
        }
        SetWindowPos(m_Window, wndAfter, windowX, windowY, windowWidth, windowHeight, flags);
    }
}

void WinScreenSetup::AdjustWindowForModeChange(const Resolution& desktopDisplayMode, FullscreenMode previousFullscreenMode, FullscreenMode fullscreenMode, int width, int height, AdjustWindowFlags flags)
{
    // Don't adjust window if all we changed was anti-aliasing or vsync count (case 561928)
    if (fullscreenMode == previousFullscreenMode && width == m_Width && height == m_Height && !HasFlag(flags, kAdjustWindowForced))
        return;

    //NESTED_LOG("DX11 debug", "AdjustWindowForModeChange: %ix%i, fs=%i, flags=%x", width, height, fullscreen, flags);

    if (!IsModeFullscreen(fullscreenMode))
    {
        // TODO: Isn't the size already clamped? This looks redundant.
        // in windowed mode, check for too big window (for all virtual desktop space)
        width = std::min(width, m_VirtualDesktopWidth);
        height = std::min(height, m_VirtualDesktopHeight);
    }

    if (HasFlag(flags, kAdjustWindowSetStyle))
    {
        AdjustWindowStyleForModeChange(fullscreenMode);
    }

    // Set window size and center it on screen if requested
    if (HasFlag(flags, kAdjustWindowSetSizeAndCenter))
    {
        bool setSizeAndCenter = !IsModeFullscreen(fullscreenMode);

        // We're in the middle of user-performed window resize, or otherwise invoked WM_SIZE. No need to perform automatic
        // placement of the window in the middle of screen; let WM_SIZE position it where it wants.
        if (g_DontRepositionWindow)
            setSizeAndCenter = false;

        if (IsModeFullscreen(fullscreenMode) && HasFlag(flags, kAdjustWindowSetSizeWhenFullscreen))
            setSizeAndCenter = true;

        if (setSizeAndCenter)
        {
            ResizeAndCenterWindowOnScreen(width, height, fullscreenMode);

            if (!IsModeFullscreen(fullscreenMode))
                SetWindowTitlebarToAero(m_Window);
        }
    }

    // Set to foreground
    if (HasARGV("hideWindow"))
        SetForegroundWindow(m_Window);
}

#endif // !UNITY_EDITOR

void WinScreenSetup::SetWindow(HWND window)
{
    if (m_Window == window)
        return;

    m_Window = window;
    RECT rc;
    GetClientRect(window, &rc);
    m_Width = m_WindowWidth = rc.right;
    m_Height = m_WindowHeight = rc.bottom;

    // Figure out our DPI
    using namespace winlib;
    if (GetShcore().IsFunctionLoaded<ShcoreDll::Fn::GetDpiForMonitor>())
    {
        UINT dpiX, dpiY;
        HRESULT hr = GetShcore().Invoke<ShcoreDll::Fn::GetDpiForMonitor>(GetMonitorForMainWindow(), MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
        DebugAssertFormatMsg(SUCCEEDED(hr), "GetDpiForMonitor(...) failed with error 0x%X.", hr);
        m_DPI = SUCCEEDED(hr) ? dpiX : USER_DEFAULT_SCREEN_DPI;
    }
    else
    {
        HDC screen = GetDC(0);
        m_DPI = GetDeviceCaps(screen, LOGPIXELSX);
        ReleaseDC(0, screen);
    }
}

bool WinScreenSetup::HasFocus() const
{
    return (GetFocus() == m_Window);
}

void WinScreenSetup::SetWindowSize(int width, int height)
{
    m_Width = m_WindowWidth = width;
    m_Height = m_WindowHeight = height;

    // In windowed mode, changing the window size means we need to change the reposition rect.
    // In fullscreen mode, don't worry about it as changing size should always mean changing resolution
    // too, and changing resolution will recalculate the rect (with appropriate black bars) at that point.
    if (!IsModeFullscreen(m_FullscreenMode))
        m_RepositionRect.Set(0, 0, m_Width, m_Height);
}

HMONITOR WinScreenSetup::GetMonitorForMainWindow() const
{
    HMONITOR monitor = MonitorFromWindow(m_Window, MONITOR_DEFAULTTONEAREST); // Note: it defaults to the default monitor if m_Window is NULL

    if (monitor == NULL)
    {
        // Our hwnd might not be valid in the editor at some points in time
        monitor = MonitorFromWindow(NULL, MONITOR_DEFAULTTOPRIMARY);
        DebugAssertMsg(monitor != NULL, "MonitorFromWindow returned NULL instead of the primary monitor!");
    }

    return monitor;
}

RectInt WinScreenSetup::GetDisplayCoordinates(HMONITOR monitor) const
{
    MONITORINFO monitorInfo = {};
    monitorInfo.cbSize = sizeof(monitorInfo);

    BOOL getMonitorInfoResult = GetMonitorInfoW(monitor, &monitorInfo);
    DebugAssertMsg(getMonitorInfoResult != FALSE, "GetMonitorInfoW failed.");

    const RECT& monitorBounds = monitorInfo.rcMonitor;
    return RectInt(monitorBounds.left, monitorBounds.top, monitorBounds.right - monitorBounds.left, monitorBounds.bottom - monitorBounds.top);
}

void WinScreenSetup::GetMonitorDisplayMode(HMONITOR monitor, Resolution& outMode)
{
    MONITORINFOEXW monitorInfo;
    const wchar_t* monitorName = NULL;

    monitorInfo.cbSize = sizeof(monitorInfo);
    BOOL getMonitorInfoResult = GetMonitorInfoW(monitor, &monitorInfo);
    DebugAssertMsg(getMonitorInfoResult != FALSE, "GetMonitorInfoW failed.");

    if (getMonitorInfoResult != NULL)
        monitorName = monitorInfo.szDevice;

    DEVMODEW displayMode;
    memset(&displayMode, 0, sizeof(displayMode));
    displayMode.dmSize = sizeof(displayMode);
    displayMode.dmDriverExtra = 0;
    EnumDisplaySettingsW(monitorName, ENUM_CURRENT_SETTINGS, &displayMode);

    outMode.width = displayMode.dmPelsWidth;
    outMode.height = displayMode.dmPelsHeight;
    outMode.refreshRate = CrackDisplayFrequency(displayMode);
}

void WinScreenSetup::UpdateHDRDisplaySupport()
{
#if !UNITY_EDITOR
    HDROutputSettings* HDRSettings = GetScreenManager().GetHDROutputSettings();
    CheckHDRDisplaySupport(GetGfxDevice().GetRenderer(), HDRSettings, m_Window);
#endif
}

#if !UNITY_EDITOR
#if GFX_SUPPORTS_OPENGL_UNIFIED

void WinScreenSetup::ReleaseUpscaledFramebufferGL(bool isShuttingDown /* = false*/)
{
    GfxDevice &gfx = GetGfxDevice();
    // set current context and the correct thread before any gl calls are made
    AutoGfxDeviceAcquireThreadOwnership autoOwn;

    if (m_SystemColor)
    {
        gfx.SetBackBufferColorDepthSurface(m_SystemColor, m_SystemDepth);
        gfx.DeallocRenderSurface(m_SystemColor);
        gfx.DeallocRenderSurface(m_SystemDepth);
        m_SystemColor = m_SystemDepth = NULL;
    }

    if (!isShuttingDown)
    {
        RenderTexture::SetActive(NULL, 0, kCubeFaceUnknown, 0, RenderTexture::kFlagForceSetRT);

        if (m_TargetColor.IsValid())
        {
            gfx.DestroyRenderSurface(m_TargetColor);
            m_TargetColor.Reset();
        }
        if (m_TargetDepth.IsValid())
        {
            gfx.DestroyRenderSurface(m_TargetDepth);
            m_TargetDepth.Reset();
        }
    }
}

void WinScreenSetup::BindDefaultFramebufferGL()
{
    if (m_UseFullscreenUpscalingGL)
    {
        gGL->BindFramebuffer(gl::kDrawFramebuffer, gl::FramebufferHandle::Default());
        gGL->BindFramebuffer(gl::kReadFramebuffer, gl::FramebufferHandle::Default());
    }

    m_UseFullscreenUpscalingGL = false;
}

void WinScreenSetup::BlitUpscaledFramebufferGL()
{
    if (!m_TargetColor.IsValid())
        return;

    AutoGfxDeviceAcquireThreadOwnership autoOwn;

    GfxFramebufferGLES &fb = gles::GetFramebufferGLES();

    gGL->BindFramebuffer(gl::kDrawFramebuffer, fb.GetDefaultFBO());
    GLint samples = 0;
    gGL->glGetIntegerv(GL_SAMPLES, &samples);
    bool needMSAAResolve = samples != 0;

    gGL->BlitFramebuffer(fb.GetDefaultFBO(), gl::kFramebufferReadDefault, gl::FramebufferHandle::Default(), gl::FramebufferHandle::Default(),
        0, 0, m_Width, m_Height, m_RepositionRect.x, m_RepositionRect.y, m_RepositionRect.width, m_RepositionRect.height, gl::kFramebufferTypeColor,
        needMSAAResolve);
    gGL->Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    gGL->BindFramebuffer(gl::kDrawFramebuffer, gl::FramebufferHandle::Default());
    gGL->BindFramebuffer(gl::kReadFramebuffer, gl::FramebufferHandle::Default());
}

void WinScreenSetup::BindUpscaledFramebufferGL(int width, int height)
{
    Assert(width > 0 && height > 0);

    // If we already created a downscale framebuffer, we delete its resources in case the resolution has changed
    this->ReleaseUpscaledFramebufferGL(false);

    SurfaceCreateFlags createFlags = kSurfaceRenderTextureAsBackBuffer;
    const bool linearColorSpace = GetActiveColorSpace() == kLinearColorSpace;
    if (linearColorSpace)
        createFlags |= kSurfaceCreateSRGB;
    GfxDevice &gfx = GetGfxDevice();

    m_TargetColor = gfx.CreateRenderColorSurface(TextureID(), width, height, 1, 0, kTexDim2D, GetGraphicsCaps().GetGraphicsFormat(kDefaultFormatLDR), createFlags);
    m_TargetDepth = gfx.CreateRenderDepthSurface(TextureID(), width, height, 1, 0, kTexDim2D, kDepthFormatMin24bits_Stencil, kSurfaceRenderTextureAsBackBuffer);

    m_SystemColor = gfx.CloneRenderSurface(gfx.GetBackBufferColorSurface().object);
    m_SystemDepth = gfx.CloneRenderSurface(gfx.GetBackBufferDepthSurface().object);

    gfx.SetBackBufferColorDepthSurface(m_TargetColor.object, m_TargetDepth.object);

    RenderTexture::SetActive(NULL, 0, kCubeFaceUnknown, 0, RenderTexture::kFlagForceSetRT);

    m_UseFullscreenUpscalingGL = true;
}

#endif//GFX_SUPPORTS_OPENGL_UNIFIED


bool WinScreenSetup::SetResolution(GfxDeviceRenderer rendererType, int width, int height, FullscreenMode fullscreenMode, bool stereo, int preferredRefreshRate, int& outBackbufferBPP, int& outFrontbufferBPP, int& outDepthBPP)
{
    bool validRendererType = (rendererType == kGfxRendererD3D11
        || rendererType == kGfxRendererD3D12
        || rendererType == kGfxRendererVulkan
        || IsUnifiedGLRenderer(rendererType));
    Assert(validRendererType);

    FullscreenMode previousFullscreenMode = m_FullscreenMode;
    bool wasFullscreen = IsModeFullscreen(m_FullscreenMode);
    bool fullscreen =  IsModeFullscreen(fullscreenMode);
    bool wasExclusiveFullscreen = (m_FullscreenMode == kExclusiveFullscreen);
    bool exclusiveFullscreen = (fullscreenMode == kExclusiveFullscreen);
    if (!wasFullscreen && fullscreen)
    {
        PlayerPrefs::SetInt(kSelectMonitor, GetCurrentlyUsedDisplayIndex());
    }

    if (wasExclusiveFullscreen && !exclusiveFullscreen || !wasExclusiveFullscreen && exclusiveFullscreen)
        m_PendingModeChange = true;

    HMONITOR monitor = GetMonitorForMainWindow();
    RectInt displayCoord = GetDisplayCoordinates(monitor);
    const dynamic_array<WinScreenSetup::WinResolution>& resolutions = GetResolutions(monitor);

    Resolution desktopDisplayMode;
    GetMonitorDisplayMode(GetMonitorForMainWindow(), desktopDisplayMode);

    // Find best matching size for exclusive mode.
    const int resIndex = exclusiveFullscreen ? FindClosestResolution(resolutions, desktopDisplayMode, width, height, fullscreen) : 0;

    // If we don't have any valid resolutions or the displayCoord is empty
    // then we should fallback to windowed mode as to allow this to progress
    // and not cause issues later with the swapchain and HDR
    bool fallbackInvalidDisplaySetup = (resolutions.empty() || displayCoord.IsEmpty());
    if (fallbackInvalidDisplaySetup && fullscreen)
    {
        fullscreen = false;
        exclusiveFullscreen = false;
        fullscreenMode = kWindowed;

        winutils::AddErrorMessage("Failed to find a valid fullscreen resolution for exclusiveFullscreen %dx%d failed, trying without exclusive fullscreen mode", width, height);
    }

    // try setting up resolutions & render windows until we succeed
    int vSyncCount = GetQualitySettings().GetCurrent().vSyncCount;
    while (true)
    {
        Rational refreshRate = { 0, 1 };
        if (exclusiveFullscreen)
        {
            // We use the best matching resolution in exclusive mode.
            width = resolutions[resIndex].width;
            height = resolutions[resIndex].height;

            auto defaultRefreshRate = m_LastRequestedExclusiveFullscreenRefreshRate > 0 ? m_LastRequestedExclusiveFullscreenRefreshRate : desktopDisplayMode.refreshRate;
            refreshRate = FindBestRefresh(resolutions[resIndex], preferredRefreshRate, defaultRefreshRate);
            m_LastRequestedExclusiveFullscreenRefreshRate = refreshRate.Numerator / refreshRate.Denominator;
        }
        else
        {
            // Use a borderless window for fullscreen instead of exclusive mode
            // Clamp window size to monitor dimensions in fullscreen mode.
            // For windowed mode allow using all virtual desktop space.
            width = std::min(width, fullscreen ? displayCoord.width : m_VirtualDesktopWidth);
            height = std::min(height, fullscreen ? displayCoord.height : m_VirtualDesktopHeight);
        }

        // Fix for case 1242757: Built project crashes when Screen.SetResolution() method is used with height or width set to 0
        width = std::max(width, 1);
        height = std::max(height, 1);

        m_RealWidth = width;
        m_RealHeight = height;
        // Position of actual image when we add black bars in fullscreen mode.
        m_RepositionRect.Set(0, 0, m_RealWidth, m_RealHeight);
        // Scale to convert from desktop coordinates to backbuffer coordinates when using upscaling. Needed for input handling.
        m_CoordinateScale.Set(1.0f, 1.0f);
        if (fullscreenMode == kFullscreenWindow)
        {
            // Upscale to desktop resolution
            m_RealWidth = displayCoord.width;
            m_RealHeight = displayCoord.height;
            m_RepositionRect.Set(0, 0, m_RealWidth, m_RealHeight);
            float origAspect = float(width) / float(height);
            float newAspect = float(m_RealWidth) / float(m_RealHeight);
            float aspectChange = newAspect / origAspect;
            if (aspectChange < 1.0f)
            {
                // Letterbox image
                int adjustedHeight = RoundfToInt(float(m_RealHeight) * aspectChange);
                int borderSize = (m_RealHeight - adjustedHeight) / 2;
                m_RepositionRect.y += borderSize;
                m_RepositionRect.height -= borderSize * 2;
            }
            else if (aspectChange > 1.0f)
            {
                // Add black bars on the sides
                int adjustedWidth = RoundfToInt(float(m_RealWidth) / aspectChange);
                int borderSize = (m_RealWidth - adjustedWidth) / 2;
                m_RepositionRect.x += borderSize;
                m_RepositionRect.width -= borderSize * 2;
            }
            m_CoordinateScale.Set(float(width) / float(m_RepositionRect.width), float(height) / float(m_RepositionRect.height));
        }

        AdjustWindowFlags flags = kAdjustWindowSetStyle | kAdjustWindowSetSizeAndCenter;
        // Allow a window resize in exclusive fullscreen only when first entering fullscreen.
        if (fullscreenMode == kFullscreenWindow || !wasFullscreen)
            flags |= kAdjustWindowSetSizeWhenFullscreen;
        if (m_MultiDisplays)
            flags |= kAdjustWindowForced;
        AdjustWindowForModeChange(desktopDisplayMode, previousFullscreenMode, fullscreenMode, m_RealWidth, m_RealHeight, flags);

        m_Width = width;
        m_Height = height;
        if (!fullscreen)
        {
            m_WindowWidth = width;
            m_WindowHeight = height;
        }
        m_FullscreenMode = fullscreenMode;
        m_Stereoscopic = stereo;

        bool hdrDisplay = false;
        HDROutputSettings* HDRSettings = GetScreenManager().GetHDROutputSettings();
        // We do not support HDR when we have to fallback to windowed mode because of invalid resolutions
        // or have an invalid displaycoords as the DXGI calls made will throw exceptions and its likely
        // not valid HDR anyway
        if (HDRSettings)
        {
            UpdateHDRDisplaySupport();
            bool desiredState = HDRSettings->GetDesiredHDRActive();
            if (HDRSettings->GetHDRModeChangeRequested())
                desiredState = !HDRSettings->GetActive();
            hdrDisplay = HDRSettings->GetAvailable() && desiredState;
        }

        bool ok = false;
        switch (rendererType)
        {
#           if GFX_SUPPORTS_OPENGL_UNIFIED
            case kGfxRendererOpenGLCore:
            case kGfxRendererOpenGLES20:
            case kGfxRendererOpenGLES3x:
            {
                ok = true;
                // Setup downscaling later, once we have the new main GL context in place.
            }
            break;
#           endif//GFX_SUPPORTS_OPENGL
#           if GFX_SUPPORTS_D3D11
            case kGfxRendererD3D11:
            {
                ok = InitializeOrResetD3D11SwapChainHWND(
                    m_Window, m_RepositionRect, m_RealWidth, m_RealHeight, width, height, DXGI_RATIONAL{ refreshRate.Numerator, refreshRate.Denominator }, exclusiveFullscreen, stereo, vSyncCount, hdrDisplay,
                    outBackbufferBPP, outFrontbufferBPP, outDepthBPP);
            }
            break;
#           endif//GFX_SUPPORTS_D3D11
#           if GFX_SUPPORTS_D3D12
            case kGfxRendererD3D12:
            {
                ok = InitializeOrResetPrimaryD3D12SwapChainHWND(
                    m_Window, m_RepositionRect, m_RealWidth, m_RealHeight, width, height, DXGI_RATIONAL{ refreshRate.Numerator, refreshRate.Denominator }, exclusiveFullscreen, stereo, vSyncCount, hdrDisplay,
                    outBackbufferBPP, outFrontbufferBPP, outDepthBPP);
            }
            break;
#           endif//GFX_SUPPORTS_D3D12
#           if GFX_SUPPORTS_VULKAN
            case kGfxRendererVulkan:
            {
                auto refreshRateInteger = refreshRate.Denominator != 0 ? (refreshRate.Numerator / refreshRate.Denominator) : 0;
                ok = vk::InitializeOrResetSwapChain(
                    m_Window, m_RealWidth, m_RealHeight, width, height, refreshRateInteger, exclusiveFullscreen, stereo, vSyncCount, hdrDisplay,
                    outBackbufferBPP, outFrontbufferBPP, outDepthBPP);
            }
            break;
#           endif//GFX_SUPPORTS_VULKAN
        }
        if (ok)
            break;

        if (exclusiveFullscreen)
        {
            // try going windowed fullscreen
            exclusiveFullscreen = false;
            fullscreenMode = kFullscreenWindow;
            winutils::AddErrorMessage("Switching to resolution %dx%d failed, trying without  exclusive fullscreen mode", width, height);
        }
        else if (stereo)
        {
            // try disabling stereo
            stereo = false;
            winutils::AddErrorMessage("Switching to resolution %dx%d failed, trying without stereoscopic rendering", width, height);
        }
        else
        {
            winutils::AddErrorMessage("Switching to resolution %dx%d failed", width, height);
            return false;
        }
    }

    return true;
}

#if GFX_SUPPORTS_OPENGL_UNIFIED
void WinScreenSetup::SetupScreenScaling()
{
    bool usingDefaultFramebuffer = !IsModeFullscreen(m_FullscreenMode) && (m_RealWidth == m_Width && m_RealHeight == m_Height);
    if (!usingDefaultFramebuffer && m_Stereoscopic)
    {
        winutils::AddErrorMessage("OpenGL quad buffer rendering only support native full screen resolution. Disabling downscaling.");
        usingDefaultFramebuffer = true;
    }

    if (!usingDefaultFramebuffer)
    {
        // In downscale cases, we render offscreen and blit to the default framebuffer.
        BindUpscaledFramebufferGL(m_Width, m_Height);
    }
    else
    {
        // In native resolution cases, we render directly to the default framebuffer.
        BindDefaultFramebufferGL();
    }
}

#endif

int WinScreenSetup::GetCurrentlyUsedDisplayIndex()
{
    RECT rc;
    GetWindowRect(m_Window, &rc);
    int currentMonitorIndex = 0;
    int matchValue = 0;
    const RectInt unityWndRect(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);

    for (int i = 0; i < UnityDisplayManager_DisplayCount(); ++i)
    {
        const DisplayDevice* device = UnityDisplayManager_GetDisplayDeviceAt(i);
        if (device)
        {
            const RectInt displayRect(device->originX, device->originY, device->screenWidth, device->screenHeight);
            if (unityWndRect.Intersects(displayRect))
            {
                const int width = std::min(unityWndRect.x + unityWndRect.width, displayRect.x + displayRect.width)
                    - std::max(unityWndRect.x, displayRect.x);
                const int height = std::min(unityWndRect.y + unityWndRect.height, displayRect.y + displayRect.height)
                    - std::max(unityWndRect.y, displayRect.y);
                const int match = width + height;
                if (match > matchValue)
                {
                    matchValue = match;
                    currentMonitorIndex = i;
                }
            }
        }
    }
    return currentMonitorIndex;
}

#endif // !UNITY_EDITOR
