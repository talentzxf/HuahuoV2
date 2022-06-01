#include "UnityPrefix.h"
#include "ContextGLES.h"
#include "Runtime/GfxDevice/opengles/ContextGLES.h"
#include "Runtime/GfxDevice/opengles/BlitFramebufferGLES.h"
#include "Runtime/GfxDevice/opengles/FrameBufferGLES.h"
#include "Runtime/GfxDevice/opengles/RenderSurfaceGLES.h"
#include "Runtime/GfxDevice/opengles/GfxDeviceGLES.h"
#include "JSBridge.h"
#include "Runtime/Graphics/QualitySettings.h"
#include "Runtime/Graphics/ScreenManager.h"
#include "Runtime/Graphics/RenderTexture.h"
#include "Runtime/Math/ColorSpaceConversion.h"
#include <emscripten.h>
#include <emscripten/html5.h>

#define ENABLE_OFFSCREEN_CANVAS 0

static BlitFramebuffer* s_BlitFramebuffer;

static BlitFramebuffer& BlitFramebufferInstance()
{
    if (!s_BlitFramebuffer)
        s_BlitFramebuffer = new BlitFramebuffer();

    return *s_BlitFramebuffer;
}

static void DestroyBlitFramebuffer()
{
    delete s_BlitFramebuffer;
    s_BlitFramebuffer = NULL;
}

static RenderSurfaceHandle s_SystemColor;
static RenderSurfaceHandle s_SystemDepth;
static RenderSurfaceHandle s_TargetColor;
static RenderSurfaceHandle s_TargetDepth;
static bool s_OffscreenFBOInitialized = false;
static EMSCRIPTEN_WEBGL_CONTEXT_HANDLE s_WebGLRenderingContext = (EMSCRIPTEN_WEBGL_CONTEXT_HANDLE)0;

static GfxFramebufferGLES& Framebuffer()
{
    return static_cast<GfxDeviceGLES&>(GetRealGfxDevice()).GetFramebuffer();
}

bool ContextGLES::Create(int glesVersion)
{
    if (JS_SystemInfo_HasWebGL())
    {
        EmscriptenWebGLContextAttributes attrs;

        // ensure all context attributes are initialized to default values
        emscripten_webgl_init_context_attributes(&attrs);

        // customize webgl context attributes
        attrs.alpha = 1;
        attrs.depth = 1;
        attrs.stencil = 1;
        // back-buffer anti-aliasing only on webgl1.0
        attrs.antialias = glesVersion == 2 && GetQualitySettings().GetCurrent().antiAliasing > 0;
        attrs.premultipliedAlpha = MAIN_THREAD_EM_ASM_INT({
            return Module.webglContextAttributes.premultipliedAlpha;
        });
        attrs.preserveDrawingBuffer = MAIN_THREAD_EM_ASM_INT({
            return Module.webglContextAttributes.preserveDrawingBuffer;
        });
        attrs.majorVersion = glesVersion - 1;
#if ENABLE_OFFSCREEN_CANVAS
        attrs.proxyContextToMainThread = EMSCRIPTEN_WEBGL_CONTEXT_PROXY_ALWAYS;
        attrs.renderViaOffscreenBackBuffer = 1;
        attrs.explicitSwapControl = 1;
#endif
        printf("Creating WebGL %d.0 context.\n", attrs.majorVersion);

        if (s_WebGLRenderingContext > 0)
            printf("Warning: Creating a new GLES context on top of an already existing one?!\n");

        s_WebGLRenderingContext = emscripten_webgl_create_context(0, &attrs);
        if (s_WebGLRenderingContext <= 0)
        {
            printf("Unable to create WebGL context.\n");
            return false;
        }
        if (!Acquire())
        {
            printf("Unable to acquire WebGL context.\n");
            return false;
        }

        // EXT_blend_minmax needs to be enabled for us to use it, as emscripten's GL wrapper won't do that for us.
        emscripten_webgl_enable_extension(s_WebGLRenderingContext, "EXT_blend_minmax");
        emscripten_webgl_enable_extension(s_WebGLRenderingContext, "WEBKIT_EXT_texture_filter_anisotropic");
        emscripten_webgl_enable_extension(s_WebGLRenderingContext, "WEBGL_compressed_texture_s3tc_srgb");

        if (glesVersion == 2)
            emscripten_webgl_enable_extension(s_WebGLRenderingContext, "EXT_sRGB");
        // EXT_color_buffer_float is required for float RenderTexture support in WebGL 2.0
        if (glesVersion >= 3)
            emscripten_webgl_enable_extension(s_WebGLRenderingContext, "EXT_color_buffer_float");
    }
    return true;
}

bool ContextGLES::IsValid()
{
    return s_WebGLRenderingContext != 0;
}

void ContextGLES::Destroy()
{
    ContextGLES::Release();

    emscripten_webgl_make_context_current(0);
    emscripten_webgl_destroy_context(s_WebGLRenderingContext);
    s_WebGLRenderingContext = 0;
}

// This would want to use emscripten_is_webgl_context_lost(), but won't quite yet work as desired
bool ContextGLES::HandleInvalidState(bool* new_context)
{
    *new_context = false;
    return true;
}

bool ContextGLES::Acquire()
{
    EMSCRIPTEN_RESULT res = emscripten_webgl_make_context_current(s_WebGLRenderingContext);
    if (res != EMSCRIPTEN_RESULT_SUCCESS)
    {
        printf("Unable to activate WebGL context (%d).\n", res);
        return false;
    }
    return true;
}

void ContextGLES::DeleteIntermediateFBOs()
{
    if (s_OffscreenFBOInitialized)
    {
        GetGfxDevice().SetBackBufferColorDepthSurface(s_SystemColor.object, s_SystemDepth.object);

        printf("Performance warning: deleting framebuffer on context thread release!!!!\n");
        DestroyBlitFramebuffer();
        GetRealGfxDevice().DestroyRenderSurface(s_TargetColor);
        GetRealGfxDevice().DestroyRenderSurface(s_TargetDepth);

        s_OffscreenFBOInitialized = false;
    }
}

void ContextGLES::Release()
{
    DeleteIntermediateFBOs();
    emscripten_webgl_make_context_current(0);
}

void ContextGLES::Present()
{
    if (!JS_SystemInfo_HasWebGL())
        return;

    const int width = GetScreenManager().GetWidth();
    const int height = GetScreenManager().GetHeight();
    Assert(width > 0 && height > 0);

    if (GetActiveColorSpace() == kLinearColorSpace)
    {
        // Either create the FBO for the first time or recreate it if the canvas resolution changed.
        bool recreate = false;
        if (s_OffscreenFBOInitialized)
        {
            if (s_TargetColor.object->width != width || s_TargetColor.object->height != height)
            {
                printf_console("Performance Warning: Recreating Offscreen FBO (%dx%d => %dx%d)\n", s_TargetColor.object->width, s_TargetColor.object->height, width, height);
                recreate = true;
            }
        }
        else
        {
            s_SystemColor = GetGfxDevice().GetBackBufferColorSurface();
            s_SystemDepth = GetGfxDevice().GetBackBufferDepthSurface();
        }

        if (!s_OffscreenFBOInitialized || recreate)
        {
            GfxDevice &gfx = GetGfxDevice();
            RenderSurfaceHandle newTargetColor = gfx.CreateRenderColorSurface(GetUncheckedRealGfxDevice().CreateTextureID(), width, height, 1, 0, kTexDim2D, kFormatR8G8B8A8_SRGB, kSurfaceRenderTextureAsBackBuffer | kSurfaceCreateSRGB);
            RenderSurfaceHandle newTargetDepth = gfx.CreateRenderDepthSurface(TextureID(), width, height, 1, 0, kTexDim2D, kDepthFormatMin24bits_Stencil, kSurfaceRenderTextureAsBackBuffer);

            gfx.SetBackBufferColorDepthSurface(newTargetColor.object, newTargetDepth.object);

            RenderTexture::SetActive(NULL, 0, kCubeFaceUnknown, 0, RenderTexture::kFlagForceSetRT);

            BlitFramebufferInstance().EnableSrgbConversion(GetActiveColorSpace() == kLinearColorSpace);

            gGL->BindFramebuffer(gl::kDrawFramebuffer, Framebuffer().GetDefaultFBO());
            gGL->Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, ColorRGBAf(0, 0, 0, 1));

            if (recreate)
            {
                // if recreating, we destroy the previous ones
                GetRealGfxDevice().DestroyRenderSurface(s_TargetColor);
                GetRealGfxDevice().DestroyRenderSurface(s_TargetDepth);
            }
            s_TargetColor = newTargetColor;
            s_TargetDepth = newTargetDepth;

            s_OffscreenFBOInitialized = true;
        }

        gGL->BindFramebuffer(gl::kDrawFramebuffer, gl::FramebufferHandle::Default());
        BlitFramebufferInstance().BlitTexture(s_TargetColor.object->textureID);
    }
#if ENABLE_OFFSCREEN_CANVAS
    emscripten_webgl_commit_frame();
#endif
}

void ContextGLES::SetVSyncInterval(UInt32 vsyncCount)
{
    // Not supported
}

namespace gl
{
    void* WebGLGetCurrentContext()
    {
        return reinterpret_cast<void*>(emscripten_webgl_get_current_context());
    }
}//namespace gl
