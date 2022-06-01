#include "UnityPrefix.h"
#include "ContextGLES.h"
#include "Runtime/Graphics/ScreenManager.h"
#include "JSBridge.h"
#include "Runtime/Camera/RenderManager.h"
#include "Runtime/Misc/Player.h"
#include "Runtime/Graphics/QualitySettings.h"
#include "Runtime/Input/InputManager.h"
#include "Runtime/GfxDevice/opengles/ContextGLES.h"
#include "Runtime/GfxDevice/opengles/GraphicsCapsGLES.h"
#include "Runtime/Shaders/GraphicsCaps.h"

#include <emscripten.h>
#include <emscripten/html5.h>

// CSS resolution is normally selected in such a way so that CSS pixel density is close to 96dpi (https://www.w3.org/TR/css-values-3/#absolute-lengths)
const double kCSSDPI = 96.0;

bool ScreenManagerWebGL::SetResolutionImmediate(int width, int height, FullscreenMode fullscreenMode, int preferredRefreshRate)
{
    int currentAA = GetQualitySettings().GetCurrent().antiAliasing;
    if (IsGfxLevelES2(GetGraphicsCaps().gles.featureLevel) && m_previousAA != -1 && m_previousAA != currentAA)
    {
        WarningString("Warning: changing antialiasing settings after startup is not supported in WebGL 1.0");
    }

    m_previousAA = currentAA;
    if (!IsModeFullscreen(GetFullscreenMode()) && !IsModeFullscreen(fullscreenMode))
    {
        // If we are not in fullscreen mode, or going into fullscreen mode, allow changing the size of the emscripten canvas.
        emscripten_set_canvas_element_size("#canvas", width, height);
        m_WindowedWidth = width;
        m_WindowedHeight = height;
    }

    if (IsModeFullscreen(fullscreenMode) != IsModeFullscreen(GetFullscreenMode()))
    {
        if (IsModeFullscreen(fullscreenMode))
        {
            if (!JS_SystemInfo_HasFullscreen())
            {
                ErrorString("Fullscreen is not supported on this browser.");
                return false;
            }
            emscripten_request_fullscreen(NULL, true);
        }
        else
            emscripten_exit_fullscreen();
    }

    return true;
}

int ScreenManagerWebGL::GetWidth() const
{
    // m_Width is updated with a non-zero value in the Update method, called at the beginning of the frame.
    if (m_Width)
        return m_Width;
    // If the screen width is requested before the first frame, calculate it from the canvas size.
    double width, height;
    GetCanvasClientSize(&width, &height);
    return CssPixelsToPhysicalPixels(width);
}

int ScreenManagerWebGL::GetHeight() const
{
    // m_Height is updated with a non-zero value in the Update method, called at the beginning of the frame.
    if (m_Height)
        return m_Height;
    // If the screen height is requested before the first frame, calculate it from the canvas size.
    double width, height;
    GetCanvasClientSize(&width, &height);
    return CssPixelsToPhysicalPixels(height);
}

float ScreenManagerWebGL::GetDPI() const
{
    return CssPixelsToPhysicalPixels(kCSSDPI);
}

FullscreenMode ScreenManagerWebGL::GetFullscreenMode() const
{
    struct EmscriptenFullscreenChangeEvent stat;
    emscripten_get_fullscreen_status(&stat);
    return stat.isFullscreen ? kFullscreenWindow : kWindowed;
}

EM_BOOL FullscreenChangeCallback(int eventType, const EmscriptenFullscreenChangeEvent *e, void *userData)
{
    ScreenManagerWebGL* sm = (ScreenManagerWebGL*)userData;
    sm->EmscriptenFullscreenChangeEvent(e);
    return true;
}

EM_BOOL FocusCallback(int eventType, const EmscriptenFocusEvent *e, void *userData)
{
    bool focus = eventType == EMSCRIPTEN_EVENT_FOCUS;
    SetPlayerFocus(focus);

    // When the player is set to not run in the background, pause when loosing focus.
    // This is not ideal, as it means we also pause when in the browser's address bar, which looks a bit odd. The alternative is to
    // use emscripten_set_visibilitychange_callback, but then we *only* pause when switching to another tab, but still run when, ie,
    // the application is sent to the background.

    if (!GetPlayerShouldRunInBackground())
        SetPlayerPause(focus ? kPlayerRunning : kPlayerPausing);

    // Reset input if we lose focus to avoid "stuck keys".
    if (!focus)
        GetInputManager().ResetInputAxes();

    return true;
}

EM_BOOL CanvasFocusCallback(int eventType, const EmscriptenFocusEvent *e, void *userData)
{
    bool focus = eventType == EMSCRIPTEN_EVENT_FOCUS;
    if (!focus && !IsCaptureAllKeyboardInputEnabled())
        GetInputManager().ResetInputAxes();
    return true;
}

void ScreenManagerWebGL::EmscriptenFullscreenChangeEvent(const void* fullscreenChangeEvent)
{
    int width, height;
    const struct EmscriptenFullscreenChangeEvent *e = (const struct EmscriptenFullscreenChangeEvent*)fullscreenChangeEvent;
    if (JS_SystemInfo_GetMatchWebGLToCanvasSize())
    {
        if (e->isFullscreen)
        {
            width = CssPixelsToPhysicalPixels(e->screenWidth);
            height = CssPixelsToPhysicalPixels(e->screenHeight);
        }
        else
        {
            width = m_WindowedWidth;
            height = m_WindowedHeight;
        }
        emscripten_set_canvas_element_size("#canvas", width, height);
    }
    ContextGLES::DeleteIntermediateFBOs();
}

ScreenManagerWebGL::ScreenManagerWebGL()
{
    //For fullscreen changes, we register a single callback on this screenmanager and we deal with it in conjuction with our fullscreen call.
    SET_EMSCRIPTEN_CALLBACK(emscripten_set_fullscreenchange_callback, 0, this, 1, FullscreenChangeCallback);

    SET_EMSCRIPTEN_CALLBACK(emscripten_set_focus_callback, 0, NULL, false, FocusCallback);
    SET_EMSCRIPTEN_CALLBACK(emscripten_set_blur_callback, 0, NULL, false, FocusCallback);

    SET_EMSCRIPTEN_CALLBACK(emscripten_set_blur_callback, "#canvas", NULL, false, CanvasFocusCallback);

    m_Width = 0;
    m_Height = 0;
    m_WindowedWidth = 0;
    m_WindowedHeight = 0;
    m_devicePixelRatio = JS_SystemInfo_GetPreferredDevicePixelRatio();
}

void ScreenManagerWebGL::SetShowCursor(bool show)
{
    if (m_ShowCursor != show)
    {
        JS_Cursor_SetShow(show);
        m_ShowCursor = show;
    }
}

void ScreenManagerWebGL::RestoreCursorLock()
{
    if (JS_SystemInfo_HasCursorLock())
    {
        CursorLockMode effectiveLockMode = GetAllowCursorLock() ? GetLockCursor() : kCursorNormal;
        if (effectiveLockMode != kCursorNormal)
        {
            SetLockCursorInternal(effectiveLockMode);
        }
    }
}

void ScreenManagerWebGL::SetLockCursorInternal(CursorLockMode lock)
{
    // TODO: Implement confine cursor
    if (kCursorLocked == lock)
    {
        if (!JS_SystemInfo_HasCursorLock())
        {
            ErrorString("Cursor locking is not supported on this browser.");
            return;
        }
        emscripten_request_pointerlock(NULL, true);
    }
    else
        emscripten_exit_pointerlock();
}

Resolution ScreenManagerWebGL::GetCurrentResolution() const
{
    Resolution res;
    double width, height;
    JS_SystemInfo_GetScreenSize(&width, &height);
    res.width = CssPixelsToPhysicalPixels(width);
    res.height = CssPixelsToPhysicalPixels(height);
    res.refreshRate = 60;
    return res;
}

void ScreenManagerWebGL::Update()
{
    RenderManager &rm = GetRenderManager();

    m_devicePixelRatio = JS_SystemInfo_GetPreferredDevicePixelRatio();

    // Check if we should keep the WebGL render target size in sync
    // with canvas size? (if not, user configures this manually from JS side)
    const bool matchWebGLToCanvasSize = JS_SystemInfo_GetMatchWebGLToCanvasSize();

    double width, height;

    if (matchWebGLToCanvasSize)
    {
        GetCanvasClientSize(&width, &height);
        width = CssPixelsToPhysicalPixels(width);
        height = CssPixelsToPhysicalPixels(height);
    }
    else
    {
        int iWidth, iHeight;
        // We weren't able to access the canvas, user has probably deleted the
        // canvas, or is in the process of relayouting the page.. the rest of this function
        // should not bother to run.
        EMSCRIPTEN_RESULT res = emscripten_get_canvas_element_size("#canvas", &iWidth, &iHeight);
        if (res != EMSCRIPTEN_RESULT_SUCCESS)
            return;
        width = iWidth;
        height = iHeight;
    }

    if (m_Width != width || m_Height != height)
    {
        if (matchWebGLToCanvasSize)
        {
            emscripten_set_canvas_element_size("#canvas", width, height);
        }
        if (!IsFullscreen())
        {
            m_WindowedWidth = width;
            m_WindowedHeight = height;
        }

        m_Width = width;
        m_Height = height;
        rm.OnWindowSizeHasChanged();
    }
}

int ScreenManagerWebGL::CssPixelsToPhysicalPixels(double value) const
{
    return (int)((value * m_devicePixelRatio) + 0.5 - (value < 0));
}
