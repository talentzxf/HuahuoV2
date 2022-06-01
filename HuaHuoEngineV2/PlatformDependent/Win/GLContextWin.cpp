#include "UnityPrefix.h"

#include "Runtime/GfxDevice/GfxDevice.h"
#include "Runtime/GfxDevice/opengl/GLContext.h"
#include "Runtime/Graphics/QualitySettings.h"
#include "ErrorMessages.h"
#include "WinUtils.h"
#include "Runtime/Shaders/GraphicsCaps.h"
#include "Runtime/Misc/PlayerSettings.h"
#include "WGLExtensions.h"
#include "Runtime/Interfaces/IVRDevice.h"
#include "Runtime/Utilities/RuntimeStatic.h"
#include "Runtime/Utilities/Argv.h"
#include "Runtime/Utilities/ArrayUtility.h"
#include <windows.h>

// define to 1 to print lots of context info
#if UNITY_RELEASE
#   define DEBUG_GL_CONTEXT 0
#else
#   define DEBUG_GL_CONTEXT 1
#endif

#ifndef WGL_CONTEXT_OPENGL_NO_ERROR_ARB
#define WGL_CONTEXT_OPENGL_NO_ERROR_ARB 0x31B3
#endif

// TBD: make bits configurable!
const int kColorChannelBits = 8;
const int kColorBits = kColorChannelBits * 4;
const int kDepthBits = 24;
const int kStencilBits = 8;
const int kDepthMemoryBits = 32;

static RuntimeStatic<std::set<HWND> > s_WindowSetupsDone(kMemGfxDevice);

namespace
{
    struct CapsWGL
    {
        bool ARB_framebuffer_sRGB;
        bool EXT_framebuffer_sRGB;
        bool ARB_create_context_profile;
        bool ARB_create_context_robustness;
        bool ARB_robustness_application_isolation;
        bool ARB_context_flush_control;
        bool EXT_create_context_es_profile;
        bool EXT_swap_control_tear;
        bool NV_delay_before_swap;
        bool EXT_colorspace;
        bool ARB_create_context_no_error;
    };

    CapsWGL gCapsWGL;
}//namespace

bool HasWGLColorSpace()
{
    return gCapsWGL.EXT_colorspace;
}

GfxDeviceLevelGL GetMasterContextLevel();
void SetMasterContextLevel(const GfxDeviceLevelGL& level);

// forMasterContext: wglChoosePixelFormatARB is an extension for which we need to create an OpenGL context to load the pointer. We never render in the master context so we just use any pixel format.
static bool SetupPixelFormat(int width, int height, HWND window, HDC dc, bool doublebuffer, bool &outStereoEnabled, bool forMasterContext)
{
    // You can only legally SetPixelFormat on a window once in its lifetime.
    // So we track which windows already have SetPixelFormat done.
    if (s_WindowSetupsDone->find(window) != s_WindowSetupsDone->end())
    {
        #if DEBUG_GL_CONTEXT
        printf_console("GLDebug context: pixel format for window %x already set, skipping\n", (DWORD)(size_t)window);
        #endif
        return true;
    }

    outStereoEnabled = (GetIVRDevice() && GetIVRDevice()->GetStereoscopicBackbuffer()) || HasARGV("enable-stereoscopic3d");

    const PIXELFORMATDESCRIPTOR pixelFormatDesc =
    {
        sizeof(PIXELFORMATDESCRIPTOR), 1,
        static_cast<DWORD>(PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | (doublebuffer ? PFD_DOUBLEBUFFER : 0) | (outStereoEnabled ? PFD_STEREO : 0)),
        PFD_TYPE_RGBA, kColorBits,
        0, 0, 0, 0, 0, 0, 0, 0, // R G B A
        0, 0, 0, 0, 0,
        kDepthBits, kStencilBits,
        0, PFD_MAIN_PLANE, 0, 0, 0, 0
    };

    int selectedPixelFormat = 0;

    if (!forMasterContext)
    {
        int pixelFormatDescWGL[] =
        {
            WGL_STEREO_ARB,                     outStereoEnabled ? GL_TRUE : GL_FALSE,
            WGL_DRAW_TO_WINDOW_ARB,             GL_TRUE,
            WGL_ACCELERATION_ARB,               WGL_FULL_ACCELERATION_ARB,
            WGL_DOUBLE_BUFFER_ARB,              doublebuffer ? GL_TRUE : GL_FALSE,
            WGL_RED_BITS_ARB,                   kColorChannelBits,
            WGL_GREEN_BITS_ARB,                 kColorChannelBits,
            WGL_BLUE_BITS_ARB,                  kColorChannelBits,
            WGL_ALPHA_BITS_ARB,                 kColorChannelBits,
            WGL_DEPTH_BITS_ARB,                 kDepthBits,
            WGL_STENCIL_BITS_ARB,               kStencilBits,
            WGL_FRAMEBUFFER_SRGB_CAPABLE_EXT,   GL_TRUE,
            WGL_SAMPLE_BUFFERS_ARB,             GL_FALSE,
            WGL_SAMPLES_ARB,                    0,
            gCapsWGL.EXT_colorspace ? WGL_COLORSPACE_ARB : 0, gCapsWGL.EXT_colorspace ? WGL_COLORSPACE_SRGB_ARB : 0,
            0, 0
        };
        AssertMsg(pixelFormatDescWGL[0] == WGL_STEREO_ARB, "OPENGL ERROR: WGL_STEREO_ARB should remain  the first pixel format parameter");

        UINT numberPixelFormat = 0;
        BOOL result = wglChoosePixelFormatARB(dc, pixelFormatDescWGL, NULL, 1, &selectedPixelFormat, &numberPixelFormat);

        if ((GL_TRUE != result || 0 == numberPixelFormat) && outStereoEnabled)
        {
            pixelFormatDescWGL[1] = GL_FALSE;
            outStereoEnabled = false;
            result = wglChoosePixelFormatARB(dc, pixelFormatDescWGL, NULL, 1, &selectedPixelFormat, &numberPixelFormat);
        }
        AssertMsg(result == GL_TRUE && numberPixelFormat > 0, "OPENGL ERROR: Failed to query the list of pixel formats available");
    }

    if (selectedPixelFormat == 0)
    {
        selectedPixelFormat = ChoosePixelFormat(dc, &pixelFormatDesc);
    }

    if (selectedPixelFormat == 0)
    {
        winutils::AddErrorMessage("OPENGL ERROR: failed to choose pixel format for dc %x", (DWORD)(size_t)dc);
        return false;
    }

    if (!SetPixelFormat(dc, selectedPixelFormat, &pixelFormatDesc))
    {
        winutils::AddErrorMessage("OPENGL ERROR: failed to set pixel format %i for dc %x", selectedPixelFormat, (DWORD)(size_t)dc);
        return false;
    }

    s_WindowSetupsDone->insert(window);

    return true;
}

static LRESULT CALLBACK MasterWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_ERASEBKGND:
            return 1; // do not erase background
        default:
            return DefWindowProcW(hWnd, message, wParam, lParam);
    }
}

static core::wstring s_MasterContextClassName;

void SetMasterContextClassName(const core::wstring& windowClassName)
{
    s_MasterContextClassName = windowClassName;
}

const core::wstring& GetMasterContextClassName()
{
    Assert(!s_MasterContextClassName.empty());
    return s_MasterContextClassName;
}

void InitWGL()
{
    wglGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)wglGetProcAddress("wglGetExtensionsStringEXT");
    Assert(wglGetExtensionsStringEXT != NULL);
    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
    Assert(wglCreateContextAttribsARB != NULL);
    wglGetPixelFormatAttribfvARB = (PFNWGLGETPIXELFORMATATTRIBFVARBPROC)wglGetProcAddress("wglGetPixelFormatAttribfvARB");
    Assert(wglGetPixelFormatAttribfvARB != NULL);
    wglGetPixelFormatAttribivARB = (PFNWGLGETPIXELFORMATATTRIBIVARBPROC)wglGetProcAddress("wglGetPixelFormatAttribivARB");
    Assert(wglGetPixelFormatAttribivARB != NULL);
    wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
    Assert(wglChoosePixelFormatARB != NULL);
    wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
    Assert(wglSwapIntervalEXT != NULL);

    if (!wglGetExtensionsStringEXT)
        return;

    const char* extensions = reinterpret_cast<const char*>(wglGetExtensionsStringEXT());

    gCapsWGL.ARB_framebuffer_sRGB = strstr(extensions, "WGL_ARB_framebuffer_sRGB") != NULL;
    gCapsWGL.EXT_framebuffer_sRGB = strstr(extensions, "WGL_EXT_framebuffer_sRGB") != NULL;
    gCapsWGL.ARB_create_context_profile = strstr(extensions, "WGL_ARB_create_context_profile") != NULL;
    gCapsWGL.ARB_create_context_robustness = strstr(extensions, "WGL_ARB_create_context_robustness") != NULL;
    gCapsWGL.ARB_robustness_application_isolation = strstr(extensions, "WGL_ARB_robustness_application_isolation") != NULL;
    gCapsWGL.ARB_context_flush_control = strstr(extensions, "WGL_ARB_context_flush_control") != NULL;
    gCapsWGL.EXT_create_context_es_profile = strstr(extensions, "WGL_EXT_create_context_es_profile") != NULL;
    gCapsWGL.EXT_swap_control_tear = strstr(extensions, "WGL_EXT_swap_control_tear") != NULL;
    gCapsWGL.NV_delay_before_swap = strstr(extensions, "WGL_NV_delay_before_swap") != NULL;
    gCapsWGL.EXT_colorspace = strstr(extensions, "WGL_EXT_colorspace") != NULL;
    gCapsWGL.ARB_create_context_no_error = strstr(extensions, "WGL_ARB_create_context_no_error") != NULL;
}

struct ContextAttribs
{
    int major;
    int minor;
    int profile;
};

const ContextAttribs & GetContextAttrib(GfxDeviceLevelGL requestedLevel)
{
    AssertMsg((requestedLevel >= kGfxLevelFirst && requestedLevel <= kGfxLevelLast), "OPENGL ERROR: Invalid device level at context creation");

    // force-desktop-glcontext is used for Nsight which doesn't support ES profile.
    const bool ForceGLCompContext = HasARGV("force-desktop-glcontext") || (IsGfxLevelES(requestedLevel) && !gCapsWGL.EXT_create_context_es_profile);
    const GfxDeviceLevelGL level = ForceGLCompContext ? kGfxLevelCore45 : requestedLevel;

    static const ContextAttribs table[] =
    {
        {2, 0, WGL_CONTEXT_ES_PROFILE_BIT_EXT},             // kGfxLevelES2,
        {3, 0, WGL_CONTEXT_ES_PROFILE_BIT_EXT},             // kGfxLevelES3,
        {3, 1, WGL_CONTEXT_ES_PROFILE_BIT_EXT},             // kGfxLevelES31,
        {3, 1, WGL_CONTEXT_ES_PROFILE_BIT_EXT},             // kGfxLevelES31AEP
        {3, 2, WGL_CONTEXT_ES_PROFILE_BIT_EXT},             // kGfxLevelES32
        {3, 2, ForceGLCompContext ? WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB : WGL_CONTEXT_CORE_PROFILE_BIT_ARB},          // kGfxLevelCore32,
        {3, 3, ForceGLCompContext ? WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB : WGL_CONTEXT_CORE_PROFILE_BIT_ARB},          // kGfxLevelCore33,
        {4, 0, ForceGLCompContext ? WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB : WGL_CONTEXT_CORE_PROFILE_BIT_ARB},          // kGfxLevelCore40,
        {4, 1, ForceGLCompContext ? WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB : WGL_CONTEXT_CORE_PROFILE_BIT_ARB},          // kGfxLevelCore41,
        {4, 2, ForceGLCompContext ? WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB : WGL_CONTEXT_CORE_PROFILE_BIT_ARB},          // kGfxLevelCore42,
        {4, 3, ForceGLCompContext ? WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB : WGL_CONTEXT_CORE_PROFILE_BIT_ARB},          // kGfxLevelCore43,
        {4, 4, ForceGLCompContext ? WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB : WGL_CONTEXT_CORE_PROFILE_BIT_ARB},          // kGfxLevelCore44,
        {4, 5, ForceGLCompContext ? WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB : WGL_CONTEXT_CORE_PROFILE_BIT_ARB},          // kGfxLevelCore45
    };
    CompileTimeAssertArraySize(table, kGfxLevelCount);

    return table[level - kGfxLevelFirst];
}

HGLRC CreateContextGL(HDC dc, HGLRC sharedContext, GfxDeviceLevelGL requestedLevel, GfxDeviceLevelGL & actualLevel)
{
    AssertMsg((requestedLevel >= kGfxLevelFirst && requestedLevel <= kGfxLevelLast), "OPENGL ERROR: Invalid device level at context creation");

    HGLRC glrc = wglCreateContext(dc);
    AssertMsg(glrc, "OPENGL ERROR: Failed to create an OpenGL context");
    if (!glrc)
        return NULL;

#   if (OPENGL_DEBUG & OPENGL_DEBUG_LOG_CONTEXT)
    printf_console("OPENGL DEBUG: make current %x\n", (DWORD)(size_t)glrc);
#   endif

    if (!wglMakeCurrent(dc, glrc))
    {
        AssertMsg(0, "OPENGL ERROR: Failed to make current an OpenGL context");
        return NULL;
    }

    // If sharedContext is null, we are creating the master context and load WGL function
    if (sharedContext == NULL)
        InitWGL();

    if (IsGfxLevelES(requestedLevel) && !gCapsWGL.EXT_create_context_es_profile)
#   if UNITY_EDITOR // We can only fallback to OpenGL core in editor mode because in standalone the OpenGL core shader bundle is not going to be included with the player.
        requestedLevel = kGfxLevelCore45;
#   else
        return NULL;
#   endif

    // If we try to create a core profile context but it's not supported,
    // we don't fallback so that the GfxDevice fails too and we fallback to another GfxDevice
    if (IsGfxLevelCore(requestedLevel) && !gCapsWGL.ARB_create_context_profile)
    {
        actualLevel = kGfxLevelUninitialized;
        return NULL;
    }
    else
        actualLevel = requestedLevel;

    // Create an OpenGL context with attributes
    {
        const int minLevel = IsGfxLevelES(actualLevel) ? kGfxLevelESFirst : kGfxLevelCoreFirst;

        for (int currentLevel = requestedLevel; currentLevel >= minLevel; --currentLevel)
        {
            const ContextAttribs & contextAttribs = ::GetContextAttrib(static_cast<GfxDeviceLevelGL>(currentLevel));
            const int forward = contextAttribs.profile == WGL_CONTEXT_CORE_PROFILE_BIT_ARB ? WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB : 0;

            int attribs[] =
            {
                WGL_CONTEXT_MAJOR_VERSION_ARB, contextAttribs.major,
                WGL_CONTEXT_MINOR_VERSION_ARB, contextAttribs.minor,
                WGL_CONTEXT_FLAGS_ARB, (DEBUG_GL_CONTEXT ? WGL_CONTEXT_DEBUG_BIT_ARB : 0) | (!DEBUG_GL_CONTEXT && gCapsWGL.ARB_create_context_no_error ? WGL_CONTEXT_OPENGL_NO_ERROR_ARB : 0) | forward,
                WGL_CONTEXT_PROFILE_MASK_ARB, contextAttribs.profile,
                0
            };

            HGLRC newglrc = wglCreateContextAttribsARB(dc, sharedContext, attribs);

            // Try to create a context without the no error flag if the previous call fails.
            if (!newglrc)
            {
                attribs[5] = (DEBUG_GL_CONTEXT ? WGL_CONTEXT_DEBUG_BIT_ARB : 0) | forward;
                newglrc = wglCreateContextAttribsARB(dc, sharedContext, attribs);
            }

            if (newglrc)
            {
                actualLevel = static_cast<GfxDeviceLevelGL>(currentLevel);

                // Delete and replace the default OpenGL context
                wglMakeCurrent(0, 0);
                wglDeleteContext(glrc);
                glrc = newglrc;

                if (!wglMakeCurrent(dc, glrc))
                {
                    AssertMsg(0, "OPENGL ERROR: Failed to make current an OpenGL context");
                    actualLevel = kGfxLevelUninitialized;
                    return NULL;
                }

                break;
            }
            else
            {
                WarningStringMsg("OpenGL %s %d.%d is not supported on this platform.", contextAttribs.profile == WGL_CONTEXT_ES_PROFILE_BIT_EXT ? "ES " : "", contextAttribs.major, contextAttribs.minor);
            }
        }
    }

#   if DEBUG_GL_CONTEXT
    printf_console("OPENGL DEBUG: VSync %i\n", GetQualitySettings().GetCurrent().vSyncCount);
#   endif
    wglSwapIntervalEXT(GetQualitySettings().GetCurrent().vSyncCount);

    return glrc;
}

GfxDeviceLevelGL CreateMasterGraphicsContext(GfxDeviceLevelGL level)
{
    Assert(!s_MasterContextClassName.empty());

    wglMakeCurrent(NULL, NULL);

#   if DEBUG_GL_CONTEXT
    printf_console("GLDebug context: create master context %s\n", s_MasterContextClassName.c_str());
#   endif

    bool ok = winutils::RegisterWindowClass(s_MasterContextClassName.c_str(), MasterWndProc, CS_HREDRAW | CS_VREDRAW | CS_OWNDC);
    if (!ok)
    {
        winutils::AddErrorMessage("GLContext: failed to register master context class: %s", WIN_LAST_ERROR_TEXT);
        return kGfxLevelUninitialized;
    }

    GraphicsContextGL* wc = new GraphicsContextGL();
    const int kMasterWindowSize = 32;
    wc->hwnd = CreateWindowW(
        s_MasterContextClassName.c_str(),
        L"UnityHiddenWindow",
        WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        0, 0, kMasterWindowSize, kMasterWindowSize,
        NULL, NULL,
        winutils::GetInstanceHandle(), NULL);
    if (!wc->hwnd)
    {
        winutils::AddErrorMessage("GLContext: failed to create offscreen window: %s", WIN_LAST_ERROR_TEXT);
    }

#   if DEBUG_GL_CONTEXT
    printf_console("GLDebug context: created window %x\n", wc->hwnd);
#   endif

    if (wc->hwnd)
        wc->hdc = GetDC(wc->hwnd);
#   if DEBUG_GL_CONTEXT
    printf_console("GLDebug context: got dc %x for window %x\n", wc->hdc, wc->hwnd);
#   endif

    bool stereoEnabled = false;
    if (!SetupPixelFormat(kMasterWindowSize, kMasterWindowSize, wc->hwnd, wc->hdc, false, stereoEnabled, true))
    {
        winutils::AddErrorMessage("GLContext warn: failed to setup offscreen pixel format: %s", WIN_LAST_ERROR_TEXT);
    }

    HGLRC SharedContext = NULL;
    GfxDeviceLevelGL actualLevel = kGfxLevelUninitialized;
    wc->hglrc = CreateContextGL(wc->hdc, SharedContext, level, actualLevel);

#   if DEBUG_GL_CONTEXT
    printf_console("GLDebug context: created glrc %x for dc %x\n", wc->hglrc, wc->hdc);
#   endif

    if (wc->hglrc == NULL)
    {
        winutils::AddErrorMessage("GLContext: failed to create context: %s", WIN_LAST_ERROR_TEXT);
        ReleaseDC(wc->hwnd, wc->hdc);
        if (wc->hwnd)
            DestroyWindow(wc->hwnd);
        delete wc;
        return kGfxLevelUninitialized;
    }

    AssignMasterGraphicsContextGL(wc);

#   if DEBUG_GL_CONTEXT
    printf_console("GLDebug context: created master context %x\n", (DWORD)(size_t)wc->hglrc);
#   endif

    SetMasterContextLevel(actualLevel);
    return actualLevel;
}

GraphicsContextHandle SetupGraphicsContextFromWindow(HWND window, int width, int height, bool &outStereoEnabled)
{
    #pragma message("! On Unity editor this assert pops a lot, is it actually needed?")
#if !UNITY_EDITOR
    Assert(!(GetMainGraphicsContext() != GetMasterGraphicsContext() && GetMainGraphicsContext().IsValid()));
#endif

    GraphicsContextHandle context = MakeNewGraphicsContextFromWindow(window, width, height, outStereoEnabled);

    // Set the new context.
#if !UNITY_EDITOR
    GetGfxDevice().AcquireThreadOwnership();
#endif
    ActivateGraphicsContext(context, false, kGLContextSkipUnbindObjects);
    SetMainGraphicsContext(context);
#if !UNITY_EDITOR
    GetGfxDevice().ReleaseThreadOwnership();
#endif

    if (UNITY_EDITOR && (IsGfxLevelCore(::GetMasterContextLevel()) || IsGfxLevelES(::GetMasterContextLevel())))
    {
        void InitDebugGLES();
        InitDebugGLES();
    }

    return context;
}

GraphicsContextHandle MakeNewGraphicsContextFromWindow(HWND window, int width, int height, bool &outStereoEnabled)
{
#if DEBUG_GL_CONTEXT
    printf_console("GLDebug context: create context from window %x\n", (DWORD)(size_t)window);
#endif

    AutoGfxDeviceAcquireThreadOwnership autoOwn;

    HDC dc = ::GetDC(window);
    if (!dc)
    {
        winutils::AddErrorMessage("GLContext: failed to get DC for %x: %s", (DWORD)(size_t)window, WIN_LAST_ERROR_TEXT);
        return GraphicsContextHandle();
    }

    if (!SetupPixelFormat(width, height, window, dc, true, outStereoEnabled, false))
    {
        winutils::AddErrorMessage("GLContext: failed to setup pixel format for window %x: %s", (DWORD)(size_t)window, WIN_LAST_ERROR_TEXT);
        ReleaseDC(window, dc);
        return GraphicsContextHandle();
    }

    HGLRC masterRC = OBJECT_FROM_HANDLE(GetMasterGraphicsContext(), GraphicsContextGL)->hglrc;

    GfxDeviceLevelGL masterDeviceLevel = ::GetMasterContextLevel();
    AssertMsg(masterDeviceLevel != kGfxLevelUninitialized, "OPENGL ERROR: Uninitialized master context");

    GfxDeviceLevelGL actualDeviceLevel = kGfxLevelUninitialized;
    HGLRC glrc = CreateContextGL(dc, masterRC, masterDeviceLevel, actualDeviceLevel);
    AssertMsg(masterDeviceLevel == actualDeviceLevel, "OPENGL ERROR: Fail to create a window context with the same level as the master context");

    if (!glrc)
    {
        winutils::AddErrorMessage("GLContext: failed to create context for %x: %s", (DWORD)(size_t)window, WIN_LAST_ERROR_TEXT);
        ReleaseDC(window, dc);
        return GraphicsContextHandle();
    }

    GraphicsContextGL* ctx = new GraphicsContextGL();
    ctx->hdc = wglGetCurrentDC();
    ctx->hglrc = wglGetCurrentContext();
    ctx->hwnd = window;

    GraphicsContextHandle context(ctx);

    return context;
}
