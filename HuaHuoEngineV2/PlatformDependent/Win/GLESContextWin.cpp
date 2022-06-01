#include "UnityPrefix.h"

#if GFX_SUPPORTS_OPENGL_UNIFIED
#include "Runtime/GfxDevice/GfxDevice.h"
#include "Runtime/GfxDevice/opengles/GfxDeviceGLES.h"
#include "Runtime/GfxDevice/opengles/ContextGLES.h"
#include "Runtime/Graphics/ScreenManager.h"
#include "Runtime/Graphics/RenderTexture.h"

#define EGL_OPENGL_ES3_BIT_KHR 0x00000040

#if PLATFORM_WIN
#define USE_DESKTOP_GLES 1
#include "Runtime/GfxDevice/opengl/GLContext.h"
#include "PlatformDependent/Win/WinUtils.h"
#include <wingdi.h>
extern void CleanupMasterContext();
#else
#define USE_DESKTOP_GLES 0
#endif

// TODO: Swap this out for the generic egl impl

struct EGLESData
{
    void*   dsp;
    void*   cfg;
    void*   cxt;
    void*   surf;
    EGLESData() : dsp(NULL), cfg(NULL), cxt(NULL), surf(NULL) {}
};

static EGLESData sOpenGLESData;

bool ContextGLES::IsValid()
{
#if USE_DESKTOP_GLES
    return IsMasterGraphicsContextValid();
#else
    return sOpenGLESData.surf != NULL &&
        sOpenGLESData.cxt  != NULL &&
        sOpenGLESData.cfg  != NULL &&
        sOpenGLESData.dsp != NULL;
#endif
}

bool ContextGLES::HandleInvalidState(bool* new_context)
{
    *new_context = false; return IsValid();
}

extern bool gAlreadyClosing;

bool ContextGLES::Create(int glesVersion)
{
#if USE_DESKTOP_GLES
    return true; // Context created separately
#else

    HWND hWnd = GetScreenManager().GetWindow();
    if (!hWnd)
    {
        ErrorString("gles20: Can't initialize because HWND not set up");
        return false;
    }

    //Just in case
    Destroy();

    EGLint numConfigs;
    EGLint majorVersion;
    EGLint minorVersion;

    /// Build up the attribute list
    const EGLint configAttribs[] =
    {
        EGL_LEVEL,              0,
        EGL_SURFACE_TYPE,       EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE,    (glesVersion == 2) ? EGL_OPENGL_ES2_BIT : EGL_OPENGL_ES3_BIT_KHR,
        EGL_NATIVE_RENDERABLE,  EGL_FALSE,
        EGL_DEPTH_SIZE,         16,
        EGL_ALPHA_SIZE,         1,
        EGL_STENCIL_SIZE,       1,
        EGL_SAMPLES,            0,
        EGL_NONE
    };

    // Get Display
    sOpenGLESData.dsp = eglGetDisplay(hWnd ? GetDC(hWnd) : EGL_DEFAULT_DISPLAY);
    if (sOpenGLESData.dsp == EGL_NO_DISPLAY)
    {
        printf_console("GLES30: eglGetDisplay failed\n");
        return false;
    }
    //Hack : eglInitialize invokes WM_ACTIVATE message, and gAppActive is already true, so Unity will try to call some functions which requires some initialization,
    //       and this is not done yet
    bool last = ::gAlreadyClosing;
    ::gAlreadyClosing = true;
    // Initialize EGL
    if (!eglInitialize(sOpenGLESData.dsp, &majorVersion, &minorVersion))
    {
        printf_console("GLES30: eglInitialize failed\n");
        return false;
    }


    // Choose config
    if (!eglChooseConfig(sOpenGLESData.dsp, configAttribs, &sOpenGLESData.cfg, 1, &numConfigs))
    {
        printf_console("GLES30: eglChooseConfig failed\n");
        return false;
    }


    // Create a surface
    sOpenGLESData.surf = eglCreateWindowSurface(sOpenGLESData.dsp, sOpenGLESData.cfg, NativeWindowType(hWnd), NULL);
    if (sOpenGLESData.surf == EGL_NO_SURFACE)
    {
        printf_console("GLES30: eglCreateWindowSurface failed\n");
        return false;
    }

    // Create a GL context
    EGLint ctxAttribList[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
    sOpenGLESData.cxt = eglCreateContext(sOpenGLESData.dsp, sOpenGLESData.cfg, EGL_NO_CONTEXT, ctxAttribList);
    if (sOpenGLESData.cxt == EGL_NO_CONTEXT)
    {
        printf_console("GLES30: eglCreateContext failed\n");
        return false;
    }

    // Make the context current
    if (!eglMakeCurrent(sOpenGLESData.dsp, sOpenGLESData.surf, sOpenGLESData.surf, sOpenGLESData.cxt))
    {
        printf_console("GLES30: eglMakeCurrent failed\n");
        return false;
    }

    ::gAlreadyClosing = last;

    GLESAssert();

    return true;

#endif
}

void ContextGLES::Destroy()
{
#if USE_DESKTOP_GLES

#if PLATFORM_WIN
    CleanupMasterContext();
#endif


#else
    if (sOpenGLESData.dsp)
    {
        eglMakeCurrent(sOpenGLESData.dsp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroyContext(sOpenGLESData.dsp, sOpenGLESData.cxt);
        eglDestroySurface(sOpenGLESData.dsp, sOpenGLESData.surf);
        eglTerminate(sOpenGLESData.dsp);
    }
    sOpenGLESData.surf = NULL;
    sOpenGLESData.cxt  = NULL;
    sOpenGLESData.cfg  = NULL;
    sOpenGLESData.dsp = NULL;
#endif
}

bool ContextGLES::Acquire()
{
#if USE_DESKTOP_GLES
    if (GetGfxThreadingMode() == kGfxThreadingModeDirect)
        return true;

    GraphicsContextGL* context = OBJECT_FROM_HANDLE(GetMainGraphicsContext(), GraphicsContextGL);
    BOOL success = wglMakeCurrent(context->hdc, context->hglrc);
    if (!success)
        printf_console("GLContext: failed to activate %x: %s\n", context->hglrc, WIN_LAST_ERROR_TEXT);

    return success == TRUE;
#else
    if (sOpenGLESData.dsp)
    {
        // Make the context current
        if (eglMakeCurrent(sOpenGLESData.dsp, sOpenGLESData.surf, sOpenGLESData.surf, sOpenGLESData.cxt))
            return true;

        printf_console("GLES30: eglMakeCurrent failed\n");
    }
    return false;
#endif
}

void ContextGLES::Release()
{
#if USE_DESKTOP_GLES
    if (GetGfxThreadingMode() == kGfxThreadingModeDirect)
        return;

    wglMakeCurrent(NULL, NULL);
#else
    if (sOpenGLESData.dsp)
    {
        eglMakeCurrent(sOpenGLESData.dsp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    }
#endif
}

void ContextGLES::Present()
{
#if USE_DESKTOP_GLES
    PresentContextGL(GetMainGraphicsContext());
#else
    eglSwapBuffers(sOpenGLESData.dsp, sOpenGLESData.surf);
#endif
}

void ContextGLES::SetVSyncInterval(UInt32 vsyncCount)
{
    SetVSyncInterval(GetMainGraphicsContext(), vsyncCount);
}

#endif // GFX_SUPPORTS_OPENGL_UNIFIED
