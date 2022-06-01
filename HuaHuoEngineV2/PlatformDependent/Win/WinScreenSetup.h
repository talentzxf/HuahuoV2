#pragma once

#include "Runtime/Core/Containers/hash_map.h"
#include "Runtime/GfxDevice/GfxDeviceTypes.h"
#include "Runtime/Graphics/Resolution.h"
#include "Runtime/Math/Rect.h"
#include "Runtime/Utilities/dynamic_array.h"
#include "Runtime/Misc/PlayerSettingsTypes.h"

#if !UNITY_EDITOR && GFX_SUPPORTS_OPENGL_UNIFIED
#include "Runtime/GfxDevice/GfxDeviceObjects.h"
#endif// !UNITY_EDITOR && GFX_SUPPORTS_OPENGL

#include <windef.h>

struct Rational // Define our own instead of DXGI_RATIONAL so we wouldn't have to include dxgi headers in this header
{
    uint32_t Numerator;
    uint32_t Denominator;
};

class WinScreenSetup
{
public:
    typedef std::pair<int, int> TIntPair;

public:
    WinScreenSetup();
    ~WinScreenSetup();

    void    UpdateVirtualDesktopBounds();
    void    InvalidateResolutions();

    // Sets window. Sets size from client rect.
    void    SetWindow(HWND window);
    // Sets window size explicitly.
    void    SetWindowSize(int width, int height);
    HWND    GetWindow() const { return m_Window; }
    bool    HasFocus() const;

    #if !UNITY_EDITOR
    bool    SetResolution(GfxDeviceRenderer rendererType, int width, int height, FullscreenMode fullscreenMode, bool stereo, int preferredRefreshRate, int& outBackbufferBPP, int& outFrontbufferBPP, int& outDepthBPP);
    #if GFX_SUPPORTS_OPENGL_UNIFIED
    void    SetupScreenScaling(); // Can only be called after the new main context has been set
    #endif

    #endif

    int     GetWidth() const { return m_Width; }
    int     GetHeight() const { return m_Height; }
    FullscreenMode GetFullscreenMode() const { return m_FullscreenMode; }
    bool PendingModeChange() const { return m_PendingModeChange;  }
    void ClearPendingModeChange() { m_PendingModeChange = false; m_FailedToApplyRequestedResolution = false; }

    bool FailedToApplyRequestedResolution() const { return m_FailedToApplyRequestedResolution; }
    void SetFailedToApplyRequestedResolution() { m_FailedToApplyRequestedResolution = true; }

    bool    IsStereoscopic() const { return m_Stereoscopic; }
    void    SetStereoscopic(bool stereoscopic) { m_Stereoscopic = stereoscopic; }
    void    UpdateHDRDisplaySupport();
    int     GetWindowWidth() const { return m_WindowWidth; }
    int     GetWindowHeight() const { return m_WindowHeight; }

    const RectInt& GetRepositionRect() const { return m_RepositionRect; }
    const Vector2f& GetCoordinateScale() const { return m_CoordinateScale; }

    int GetDPI() const { return m_DPI; }
    void SetDPI(int dpi) { m_DPI = dpi; }

    #if SUPPORT_MULTIPLE_DISPLAYS
    bool    IsMultiDisplayEnabled() const { return m_MultiDisplays; }
    void    SetMultiDisplay(const bool enable) { m_MultiDisplays = enable; }
    #endif

    #if UNITY_EDITOR
    void    BoundRectangleToDesktops(RECT& rc) const;
    #endif

    #if !UNITY_EDITOR
    #if GFX_SUPPORTS_OPENGL_UNIFIED
    bool NeedsFullscreenUpscaling() const {return (m_Width < m_RealWidth) || (m_Height < m_RealHeight); }

    // Bind the default framebuffer
    void BindDefaultFramebufferGL();

    // Create and bind the render framebuffer object, for fullscreen upscaling cases
    void BindUpscaledFramebufferGL(int width, int height);

    // Blit with scaling the render framebuffer to the default framebuffer
    void BlitUpscaledFramebufferGL();

    // Release OpenGL ressources allocated for fullscreen upscaling cases
    void ReleaseUpscaledFramebufferGL(bool isShuttingDown);
    #endif//GFX_SUPPORTS_OPENGL


    enum AdjustWindowFlags
    {
        kAdjustWindowSetSizeAndCenter = (1 << 0),
        kAdjustWindowSetSizeWhenFullscreen = (1 << 1),
        kAdjustWindowSetStyle = (1 << 2),
        kAdjustWindowForced = (1 << 3),
    };
    ENUM_FLAGS_AS_MEMBER(AdjustWindowFlags);

    void AdjustWindowForModeChange(const Resolution& desktopDisplayMode, FullscreenMode previousFullscreenMode, FullscreenMode fullscreenMode, int width, int height, AdjustWindowFlags flags);
    void ResizeAndCenterWindowOnScreen(int width, int height, FullscreenMode fullscreenMode);
    #endif

private:
    #if !UNITY_EDITOR
    void AdjustWindowStyleForModeChange(FullscreenMode fullscreenMode);
    int GetCurrentlyUsedDisplayIndex();
    #endif

    RectInt GetDisplayCoordinates(HMONITOR monitor) const;
public:
    struct WinResolution
    {
        int width;
        int height;
        dynamic_array<Rational> refreshRates;
        bool IsRotated() const { return height > width; }
    };

    HMONITOR GetMonitorForMainWindow() const;
    const dynamic_array<WinResolution>& GetResolutions(HMONITOR monitor);
    static void GetMonitorDisplayMode(HMONITOR monitor, Resolution& outMode);

private:
    core::hash_map<HMONITOR, dynamic_array<WinResolution> > m_Resolutions;
    HWND    m_Window;
    int     m_DPI;

    int     m_VirtualDesktopX, m_VirtualDesktopY, m_VirtualDesktopWidth, m_VirtualDesktopHeight; // total desktop position/size of all monitors

    int     m_Width, m_RealWidth, m_WindowWidth;
    int     m_Height, m_RealHeight, m_WindowHeight;

    FullscreenMode m_FullscreenMode;
    bool    m_Stereoscopic;
    bool    m_MultiDisplays;
    bool    m_PendingModeChange;
    bool    m_FailedToApplyRequestedResolution;
    int     m_LastRequestedExclusiveFullscreenRefreshRate;

    RectInt m_RepositionRect;
    Vector2f m_CoordinateScale;

#if !UNITY_EDITOR && GFX_SUPPORTS_OPENGL_UNIFIED
    /////////////////////////////////////
    // For fullscreen upscaling support
    bool m_UseFullscreenUpscalingGL;

    RenderSurfaceHandle m_TargetColor;
    RenderSurfaceHandle m_TargetDepth;
    RenderSurfaceBase *m_SystemColor;
    RenderSurfaceBase *m_SystemDepth;

#endif//!UNITY_EDITOR && GFX_SUPPORTS_OPENGL
};
