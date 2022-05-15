#pragma once

#include "Runtime/Graphics/ScreenManager.h"

class ScreenManagerWebGL : public ScreenManager
{
public:
    ScreenManagerWebGL();

    bool SetResolutionImmediate(int width, int height, FullscreenMode fullscreenMode, int preferredRefreshRate) override;

    Resolution GetCurrentResolution() const override;

    bool GetShowCursor() const override { return m_ShowCursor; }
    void SetShowCursor(bool show) override;

    void RestoreCursorLock() override;

    int GetWidth() const override;
    int GetHeight() const override;
    float GetDPI() const override;
    FullscreenMode GetFullscreenMode() const override;

    // Override this to avoid the default behaviour of hiding the cursor when it is locked - the browser already takes care of that.
    void SetCursorInsideWindow(bool insideWindow) override { m_CursorInWindow = insideWindow; }

    void EmscriptenFullscreenChangeEvent(const void* fullscreenChangeEvent);
    void Update();

    int CssPixelsToPhysicalPixels(double value) const;

private:
    int m_previousAA;
    int m_Width;
    int m_Height;
    int m_WindowedWidth;
    int m_WindowedHeight;
    double m_devicePixelRatio;

    void SetLockCursorInternal(CursorLockMode lock) override;
};

#ifdef ScreenManagerPlatform
#undef ScreenManagerPlatform
#endif

#define ScreenManagerPlatform ScreenManagerWebGL
