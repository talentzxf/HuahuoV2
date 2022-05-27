#include "AssertGLES.h"
// #include "ContextGLES.h"
#include "ApiGLES.h"
#include "ApiConstantsGLES.h"
// #include "TimerQueryGLES.h"
#include "GfxGetProcAddressGLES.h"
// #include "GpuProgramsGLES.h"
#include "GfxDeviceGLES.h"
#include "ExtensionsGLES.h"

// #include "Runtime/GfxDevice/egl/ContextGLES.h"
// #include "Runtime/Graphics/ColorGamut.h"
#include "Math/ColorSpaceConversion.h"
//#include "Runtime/Misc/CPUInfo.h"
//#include "Runtime/Misc/SystemInfo.h"
//#include "Runtime/Misc/PlayerSettings.h"
#include "Shaders/GraphicsCaps.h"
//#include "Runtime/Utilities/Argv.h"
#include "Utilities/BitUtility.h"
#include "Utilities/Word.h"
#include "Graphics/ColorGamut.h"
#include "ApiTranslateGLES.h"

#if PLATFORM_ANDROID
#include "PlatformDependent/AndroidPlayer/Source/AndroidSystemInfo.h"
#endif

#if PLATFORM_WEBGL
#include "PlatformDependent/WebGL/Source/JSBridge.h"
#endif

#if PLATFORM_IOS
extern "C" int UnityDeviceGeneration();
#endif

#if PLATFORM_WIN
#include "PlatformDependent/Win/WinDriverUtils.h"
    #include <windows.h>
#endif

#if PLATFORM_OSX
#include <OpenGL/OpenGL.h>
    #include <CoreGraphics/CoreGraphics.h>
#endif

#include <cctype>
#include <cstring>
#include <string>

#if FORCE_DEBUG_BUILD_WEBGL
#   undef PLATFORM_WEBGL
#   define PLATFORM_WEBGL 1
#endif//FORCE_DEBUG_BUILD_WEBGL

#define UNITY_DESKTOP (PLATFORM_WIN || PLATFORM_LINUX || PLATFORM_OSX)

#if PLATFORM_LINUX && ENABLE_SDL2_WINDOWING
#include "PlatformDependent/Linux/SDLWrapper.h"
#endif
#define printf_console printf

bool HasWGLColorspace();

GraphicsCapsGLES* g_GraphicsCapsGLES = 0;

namespace systeminfo { int GetPhysicalMemoryMB(); }

#if PLATFORM_WIN

static void GetVideoCardIDsWin(VendorID& outVendorID, int& outDeviceID)
{
    outVendorID = kVendorDummyRef;
    outDeviceID = 0;

    DISPLAY_DEVICEA dev;
    dev.cb = sizeof(dev);
    int i = 0;
    while (EnumDisplayDevicesA(NULL, i, &dev, 0))
    {
        if (dev.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)
        {
            const char* sVendorID = strstr(dev.DeviceID, "VEN_");
            if (sVendorID)
                sscanf(sVendorID, "VEN_%x", &outVendorID);
            const char *sDeviceID = strstr(dev.DeviceID, "DEV_");
            if (sDeviceID)
                sscanf(sDeviceID, "DEV_%x", &outDeviceID);
            return;
        }
        ++i;
    }
}

#elif PLATFORM_OSX

static CFTypeRef SearchPortForProperty(io_registry_entry_t dspPort, CFStringRef propertyName)
{
    return IORegistryEntrySearchCFProperty(dspPort,
        kIOServicePlane,
        propertyName,
        kCFAllocatorDefault,
        kIORegistryIterateRecursively | kIORegistryIterateParents);
}

static UInt32 IntValueOfCFData(CFDataRef d)
{
    UInt32 value = 0;
    if (d)
    {
        const UInt32 *vp = reinterpret_cast<const UInt32*>(CFDataGetBytePtr(d));
        if (vp != NULL)
            value = *vp;
    }
    return value;
}

static void GetVideoCardIDsOSX(VendorID& outVendorID, int& outDeviceID)
{
    CFStringRef strVendorID = CFStringCreateWithCString(NULL, "vendor-id", kCFStringEncodingASCII);
    CFStringRef strDeviceID = CFStringCreateWithCString(NULL, "device-id", kCFStringEncodingASCII);

    CGDirectDisplayID displayID = kCGDirectMainDisplay;
    io_registry_entry_t dspPort = CGDisplayIOServicePort(displayID);
    CFTypeRef vendorIDRef = SearchPortForProperty(dspPort, strVendorID);
    CFTypeRef deviceIDRef = SearchPortForProperty(dspPort, strDeviceID);

    CFRelease(strDeviceID);
    CFRelease(strVendorID);

    outVendorID = static_cast<VendorID>(IntValueOfCFData((CFDataRef)vendorIDRef));
    outDeviceID = IntValueOfCFData((CFDataRef)deviceIDRef);

    if (vendorIDRef)
        CFRelease(vendorIDRef);
    if (deviceIDRef)
        CFRelease(deviceIDRef);
}

#elif PLATFORM_LINUX
typedef struct
{
    unsigned int vendor;
    unsigned int device;
    unsigned long vramSize;
} card_info_t;

static void GetPrimaryGraphicsCardInfo(card_info_t *info)
{
    char line[BUFSIZ];

    // Device ID is in the device file
    FILE *f = fopen("/sys/class/drm/card0/device/device", "r");
    if (f)
    {
        fgets(line, BUFSIZ, f);
        fclose(f);
        info->device = strtol(line, NULL, 16);
    }

    // Vendor ID is in the vendor file
    f = fopen("/sys/class/drm/card0/device/vendor", "r");
    if (f)
    {
        fgets(line, BUFSIZ, f);
        fclose(f);
        info->vendor = strtol(line, NULL, 16);
    }

    // We used to do some calculation of PCI apertures.
    // This hasn't been accurate for quite some time,
    // so now we default to 512M.
    info->vramSize = 0x20000000UL;
}

static card_info_t gCardInfo;
static void InitCardInfoLinux()
{
    gCardInfo.device = 0;
    gCardInfo.vendor = 0;
    gCardInfo.vramSize = 0;
    GetPrimaryGraphicsCardInfo(&gCardInfo);

    if (gCardInfo.vramSize < (64 * 1024 * 1024))
    {
        // Default to 64MB
        gCardInfo.vramSize = 64 * 1024 * 1024;
    }
}

static void GetVideoCardIDsLinux(VendorID& outVendorID, int& outDeviceID)
{
    outVendorID = static_cast<VendorID>(gCardInfo.vendor);
    outDeviceID = gCardInfo.device;
}

#endif // #if PLATFORM_LINUX

void GetVideoCardIDs(VendorID& vendorID, int& rendererID)
{
#if PLATFORM_WIN
    GetVideoCardIDsWin(vendorID, rendererID);
#elif PLATFORM_OSX
    GetVideoCardIDsOSX(vendorID, rendererID);
#elif PLATFORM_LINUX
    InitCardInfoLinux();
    GetVideoCardIDsLinux(vendorID, rendererID);
#else
    vendorID = kVendorDummyRef;
    rendererID = 0;
#endif
}

// Utilities for video memory size calculations

#if PLATFORM_OSX
// Adapted from https://developer.apple.com/library/mac/qa/qa1168/_index.html
// "How do I determine how much VRAM is available on my video card?"
static int GetVideoMemoryMBOSX(int wantedRendererID)
{
    int vramMB = 0;

    const int kMaxDisplays = 8;
    CGDirectDisplayID displays[kMaxDisplays];
    CGDisplayCount displayCount;
    CGGetActiveDisplayList(kMaxDisplays, displays, &displayCount);
    CGOpenGLDisplayMask openGLDisplayMask = CGDisplayIDToOpenGLDisplayMask(displays[0]);

    CGLRendererInfoObj info;
    GLint numRenderers = 0;
    CGLError err = CGLQueryRendererInfo(openGLDisplayMask, &info, &numRenderers);
    if (0 == err)
    {
        for (int j = 0; j < numRenderers; ++j)
        {
            GLint rv = 0;
            // Get the accelerated one. Typically macs report 2 renderers, one actual GPU
            // and one software one. This is true even for dual-gpu systems like MBPs,
            // normally only the dGPU is reported, but if forcing the integrated one then
            // only the iGPU is reported as accelerated.
            err = CGLDescribeRenderer(info, j, kCGLRPAccelerated, &rv);
            if (rv == 1)
            {
                GLint thisRendererMB = 0;
                err = CGLDescribeRenderer(info, j, kCGLRPVideoMemoryMegabytes, &thisRendererMB);
                vramMB = thisRendererMB;
                break;
            }
        }
        CGLDestroyRendererInfo(info);
    }

    // Safeguards: all Macs support at least 128MB, but things like software device could
    // return zeroes.
    if (vramMB < 128)
        vramMB = 128;
    return vramMB;
}

#endif // if PLATFORM_OSX

#if PLATFORM_LINUX

typedef int (*MESARendererIntegerQuery)(int, unsigned*);
#ifndef GLX_RENDERER_VIDEO_MEMORY_MESA
#define GLX_RENDERER_VIDEO_MEMORY_MESA 0x8187
#endif
#ifndef GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX
#define GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX 0x9047
#endif
#ifndef GL_TEXTURE_FREE_MEMORY_ATI
#define GL_TEXTURE_FREE_MEMORY_ATI 0x87FC
#endif

static int GetVideoMemoryMBLinux(const ApiGLES* api)
{
    int vram = 0;
    // Nvidia
    if (HasExtension(GLExt::kGL_NVX_gpu_memory_info))
    {
        vram = api->Get(GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX) / 1024;
    }
    // ATI
    if (HasExtension(GLExt::kGL_ATI_meminfo))
    {
        GLint vramData[4] = {0, 0, 0, 0};
        GLES_CALL(api, glGetIntegerv, GL_TEXTURE_FREE_MEMORY_ATI, vramData);
        vram = vramData[0] / 1024;
    }
    // Mesa
    if (HasExtension(GLExt::kGLX_MESA_query_renderer))
    {
        MESARendererIntegerQuery glXQueryCurrentRendererIntegerMESA = (MESARendererIntegerQuery)gles::GetProcAddress("glXQueryCurrentRendererIntegerMESA");
        if (glXQueryCurrentRendererIntegerMESA)
        {
            unsigned vramData = 0;
            glXQueryCurrentRendererIntegerMESA(GLX_RENDERER_VIDEO_MEMORY_MESA, &vramData);
            vram = vramData;
        }
    }

    // Got no data? Default to 512MB
    if (vram <= 0)
        vram = 512;

    // Safeguard, assume we have at least 128MB
    if (vram < 128)
        vram = 128;

    return vram;
}

#endif // #if PLATFORM_LINUX


// ---------------------------------------------------------------------------------------------
// Capability check per feature for each OpenGL renderer using the unified OpenGL back-end
// Those are pure functions only
namespace
{
    // List of the OpenGL capability queries
    const GLenum GL_MAX_TRANSFORM_FEEDBACK_BUFFERS = 0x8E70;
#if !PLATFORM_WEBGL
    const GLenum GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS = 0x92DC;
    const GLenum GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS = 0x90DD;
    const GLenum GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS = 0x90DC;
    const GLenum GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS = 0x90DB;
    const GLenum GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS = 0x90DA;
    const GLenum GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS = 0x90D6;
    const GLenum GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS = 0x90D7;
    const GLenum GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS = 0x90D8;
    const GLenum GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS = 0x90D9;
    const GLenum GL_MAX_IMAGE_UNITS = 0x8F38;
    const GLenum GL_MAX_UNIFORM_BUFFER_BINDINGS = 0x8A2F;
    const GLenum GL_MAX_VERTEX_UNIFORM_VECTORS = 0x8DFB;
    const GLenum GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS = 0x8B4D;
    const GLenum GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS = 0x8B4C;
    const GLenum GL_MAX_VERTEX_UNIFORM_COMPONENTS = 0x8B4A;
    const GLenum GL_MAX_TEXTURE_SIZE = 0x0D33;
    const GLenum GL_MAX_ARRAY_TEXTURE_LAYERS = 0x88FF;
    const GLenum GL_MAX_CUBE_MAP_TEXTURE_SIZE = 0x851C;
    const GLenum GL_MAX_UNIFORM_BLOCK_SIZE = 0x8A30;
    const GLenum GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT = 0x8A34;
    const GLenum GL_MAX_RENDERBUFFER_SIZE = 0x84E8;
    const GLenum GL_MAX_SAMPLES_IMG = 0x9135;
    const GLenum GL_MAX_SAMPLES = 0x8D57;
    const GLenum GL_MAX_COLOR_ATTACHMENTS = 0x8CDF;
    const GLenum GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT = 0x84FF;
    const GLenum GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS = 0x90EB;
    const GLenum GL_MAX_COMPUTE_WORK_GROUP_SIZE = 0x91BF;
    const GLenum GL_MAJOR_VERSION = 0x821B;
    const GLenum GL_MINOR_VERSION = 0x821C;
#endif // #if !PLATFORM_WEBGL

    bool RequireDrawBufferNone(const ApiGLES & api, const GraphicsCaps & caps, GfxDeviceLevelGL level)
    {
//        // Support of GL_ARB_ES2_compatibility isn't right on some Intel HD 4000 and AMD 7670M drivers
//        // Which requires to call glDrawBuffer(GL_NONE) on depth only framebuffer
//        if (IsGfxLevelCore(level))
//            return !HasExtension(GLExt::kGL_ARB_ES2_compatibility) || caps.gles.isIntelGpu || caps.gles.isAMDGpu;
//        return false;
    }

    bool HasDrawBuffers(const ApiGLES & api, GfxDeviceLevelGL level)
    {
        if (IsGfxLevelCore(level) || IsGfxLevelES3(level, kGfxLevelES3))
            return true;
        if (IsGfxLevelES2(level))
            return HasExtension(WebGLExt::kWEBGL_draw_buffers) || (HasExtension(GLExt::kGL_NV_draw_buffers) && HasExtension(GLExt::kGL_NV_fbo_color_attachments));
        return false;
    }

    bool HasES2Compatibility(const ApiGLES & api, GfxDeviceLevelGL level)
    {
        // To run OpenGL ES shaders on OpenGL core context which is not supported yet and cause errors on OSX
        // because we generate OpenGL ES shaders that relies EXT_shadow_samplers which isn't support on desktop in the extension form.
#       if 1
        return false;
#       else
        return IsGfxLevelCore(level) ? HasExtension(GLExt::kGL_ARB_ES2_compatibility) : false;
#       endif
    }

    bool HasES3Compatibility(const ApiGLES & api, GfxDeviceLevelGL level)
    {
        return IsGfxLevelCore(level) ? HasExtension(GLExt::kGL_ARB_ES3_compatibility) : false;
    }

    bool HasES31Compatibility(const ApiGLES & api, GfxDeviceLevelGL level)
    {
        return IsGfxLevelCore(level) ? HasExtension(GLExt::kGL_ARB_ES3_1_compatibility) : false;
    }

    bool HasES32Compatibility(const ApiGLES & api, GfxDeviceLevelGL level)
    {
        return IsGfxLevelCore(level) ? HasExtension(GLExt::kGL_ARB_ES3_2_compatibility) : false;
    }

    bool HasComputeShader(const ApiGLES & api, GfxDeviceLevelGL level, bool clamped)
    {
        if (IsGfxLevelCore(level, kGfxLevelCore43) || IsGfxLevelES3(level, kGfxLevelES31))
            return true;
        if (!clamped)
            return HasExtension(GLExt::kGL_ARB_compute_shader) && HasExtension(GLExt::kGL_ARB_shader_image_load_store) && HasExtension(GLExt::kGL_ARB_shader_storage_buffer_object);
        return false;
    }

    bool HasVertexArrayObject(const ApiGLES & api, GfxDeviceLevelGL level, bool clamped)
    {
        // We avoid as much as possible to use VAO on mobile because Qualcomm ES2 and ES3 drivers have random crashes on them
        // No extension checking and only use in ES 3.1 and core profile where it's required
        // Extension checking is required to run ES on Core context.

        if (IsGfxLevelCore(level) || IsGfxLevelES3(level, kGfxLevelES31))
            return true;
        if (UNITY_DESKTOP)
            return HasExtension(GLExt::kGL_ARB_vertex_array_object) || HasExtension(GLExt::kGL_OES_vertex_array_object);
        return false;
    }

    bool HasIndirectDraw(const ApiGLES & api, GfxDeviceLevelGL level, bool clamped)
    {
        if (IsGfxLevelCore(level, kGfxLevelCore40) || IsGfxLevelES3(level, kGfxLevelES31))
            return true;
        if (!clamped)
            return HasExtension(GLExt::kGL_ARB_draw_indirect);
        return false;
    }

    bool HasInstancedDraw(const ApiGLES & api, GfxDeviceLevelGL level, bool clamped)
    {
        if (IsGfxLevelCore(level, kGfxLevelCore32) || IsGfxLevelES3(level, kGfxLevelES3))
            return true;

        if (!clamped)
            return HasExtension(GLExt::kGL_NV_draw_instanced) || HasExtension(GLExt::kGL_EXT_draw_instanced) || HasExtension(GLExt::kGL_ARB_draw_instanced) || HasExtension(WebGLExt::kANGLE_instanced_arrays);
        return false;
    }

    bool HasDrawBaseVertex(const ApiGLES & api, GfxDeviceLevelGL level, bool clamped)
    {
        if (IsGfxLevelCore(level, kGfxLevelCore32) || IsGfxLevelES3(level, kGfxLevelES32))
            return true;
        if (!clamped)
            return HasExtension(GLExt::kGL_EXT_draw_elements_base_vertex) || HasExtension(GLExt::kGL_OES_draw_elements_base_vertex) || HasExtension(GLExt::kGL_ARB_draw_elements_base_vertex);
        return false;
    }

    bool Has32BitIndexBuffer(const ApiGLES & api, GfxDeviceLevelGL level, bool clamped)
    {
        if (IsGfxLevelCore(level) || IsGfxLevelES3(level, kGfxLevelES3))
            return true;
        if (!clamped)
            return HasExtension(GLExt::kGL_OES_element_index_uint);
        return false;
    }

    bool HasSeparateShaderObject(const ApiGLES & api, GfxDeviceLevelGL level, bool clamped)
    {
        if (IsGfxLevelCore(level, kGfxLevelCore41) || IsGfxLevelES3(level, kGfxLevelES31))
            return true;
        if (!clamped)
            return HasExtension(GLExt::kGL_ARB_separate_shader_objects) || HasExtension(GLExt::kGL_EXT_separate_shader_objects);
        return false;
    }

    bool HasSamplerObject(const ApiGLES & api, GfxDeviceLevelGL level, bool clamped)
    {
        if (IsGfxLevelCore(level, kGfxLevelCore33) || IsGfxLevelES3(level, kGfxLevelES3))
            return true;
        if (!clamped)
            return HasExtension(GLExt::kGL_ARB_sampler_objects);
        return false;
    }

    bool HasAnisoFilter(const ApiGLES & api, GfxDeviceLevelGL level, bool clamped)
    {
        if (!clamped)
            return HasExtension(GLExt::kGL_EXT_texture_filter_anisotropic) || HasExtension(WebGLExt::kEXT_texture_filter_anisotropic) || HasExtension(WebGLExt::kWEBKIT_EXT_texture_filter_anisotropic);
        return false;
    }

    CopyTextureSupport HasCopyTexture(const ApiGLES & api, GfxDeviceLevelGL level, bool clamped)
    {
        CopyTextureSupport caps = kCopyTextureSupportBasic | kCopyTextureSupport3D | kCopyTextureSupportDifferentTypes | kCopyTextureSupportTextureToRT | kCopyTextureSupportRTToTexture;
        if (IsGfxLevelCore(level, kGfxLevelCore43) || IsGfxLevelES3(level, kGfxLevelES31AEP))
            return caps;
        if (!clamped)
            return HasExtension(GLExt::kGL_ARB_copy_image) || HasExtension(GLExt::kGL_OES_copy_image) || HasExtension(GLExt::kGL_EXT_copy_image) ? caps : kCopyTextureSupportNone;
        return kCopyTextureSupportNone;
    }

    bool HasDirectTextureAccess(const ApiGLES & api, GfxDeviceLevelGL level, bool clamped)
    {
        if (IsGfxLevelCore(level, kGfxLevelCore45))
            return true;
        if (!clamped)
            return HasExtension(GLExt::kGL_EXT_direct_state_access) || HasExtension(GLExt::kGL_ARB_direct_state_access);
        return false;
    }

    bool HasTextureMirrorOnce(const ApiGLES & api, GfxDeviceLevelGL level, bool clamped)
    {
        if (IsGfxLevelCore(level, kGfxLevelCore44))
            return true;
        if (!clamped)
            return HasExtension(GLExt::kGL_EXT_texture_mirror_clamp_to_edge) || HasExtension(GLExt::kGL_ARB_texture_mirror_clamp_to_edge) || HasExtension(GLExt::kGL_EXT_texture_mirror_clamp) || HasExtension(GLExt::kGL_ATI_texture_mirror_once);
        return false;
    }

    SparseTextureCaps HasTextureSparse(const ApiGLES & api, GfxDeviceLevelGL level, bool clamped)
    {
        if (IsGfxLevelES2(level)) // NVidia shield has extensions, but crashes on GLES2
            return kSparseTextureNone;

        if (!clamped)
            return HasExtension(GLExt::kGL_ARB_sparse_texture) || HasExtension(GLExt::kGL_EXT_sparse_texture) ? kSparseTextureTier1 : kSparseTextureNone;
        return kSparseTextureNone;
    }

    bool HasTextureBuffer(const ApiGLES & api, GfxDeviceLevelGL level, bool clamped)
    {
        if (IsGfxLevelCore(level, kGfxLevelCore43) || IsGfxLevelES3(level, kGfxLevelES31AEP))
            return true;
        if (!clamped)
            return IsGfxLevelES3(level, kGfxLevelES31) ? HasExtension(GLExt::kGL_EXT_texture_buffer) : false;
        return false;
    }

    bool HasDebug(const ApiGLES& api, const GraphicsCaps& caps, GfxDeviceLevelGL level)
    {
//#       if PLATFORM_ANDROID
//        // Android PowerVR devices expose KHR_debug, but drivers seem to be broken (cases 712386, 787491).
//        // At least on Android 4.4.2 device Asus memo Pad K015.
//        // So we explicitly disable the support, even if everything claims to be in place, for all Androids with PVR GPUs.
//        if (caps.gles.isPvrGpu)
//            return false;
//
//        // Amazon Mali 450 devices generate OpenGL error when enabling debug outout
//        if (::strcasecmp(android::systeminfo::Manufacturer(), "Amazon") == 0 && caps.gles.isMaliGpu)
//            return false;
//#       endif//PLATFORM_ANDROID
//
//        // Some devices (Tegra 3, Tegra 4) report supporting KHR_debug but then entrypoints are missing. Check them.
//        return UNITY_DEVELOPER_BUILD && (HasExtension(GLExt::kGL_KHR_debug) || IsGfxLevelES3(level, kGfxLevelES32)) &&
//               !PLATFORM_LINUX && // FIXME: Linux gfx test agents claim to support this, but crash
//               api.glDebugMessageControl &&
//               api.glDebugMessageCallback &&
//               api.glDebugMessageInsert &&
//               api.glObjectLabel &&
//               api.glGetObjectLabel &&
//               api.glPushDebugGroup &&
//               api.glPopDebugGroup;
        return false;
    }

    bool HasDebugMarker(const ApiGLES & api, const GraphicsCaps & caps, GfxDeviceLevelGL level)
    {
//#       if UNITY_EDITOR && PLATFORM_OSX
//        return false;
//#       endif
//
//#       if PLATFORM_ANDROID
//        // Amazon has issues with debug marker, crashing development build
//        // Also, disable debug markers on all PVR Series 5
//        if ((::strcasecmp(android::systeminfo::Manufacturer(), "Amazon") == 0 && caps.gles.isPvrGpu) ||
//            (caps.gles.driverGLESVersion == 2 && caps.gles.isPvrGpu))
//            return false;
//#       endif//PLATFORM_ANDROID
//
//        return UNITY_DEVELOPER_BUILD && (HasDebug(api, caps, level) || HasExtension(GLExt::kGL_EXT_debug_marker));
        return false;
    }

    bool HasDebugLabel(const ApiGLES & api, const GraphicsCaps & caps, GfxDeviceLevelGL level)
    {
#       if UNITY_EDITOR && PLATFORM_OSX
        return false;
#       endif

        return (HasDebug(api, caps, level) || HasExtension(GLExt::kGL_EXT_debug_label));
    }

    bool HasUniformBuffer(const ApiGLES & api, GfxDeviceLevelGL level, bool clamped)
    {
        if (IsGfxLevelCore(level, kGfxLevelCore32) || IsGfxLevelES3(level, kGfxLevelES3))
            return true;
        if (!clamped)
            return HasExtension(GLExt::kGL_ARB_uniform_buffer_object);
        return false;
    }

    bool HasTessellationShader(const ApiGLES & api, const GraphicsCaps & caps, GfxDeviceLevelGL level, bool clamped)
    {
        if (IsGfxLevelCore(level, kGfxLevelCore40) || IsGfxLevelES3(level, kGfxLevelES31AEP))
            return true;
        if (!clamped || IsGfxLevelES3(level, kGfxLevelES31))
            return HasExtension(GLExt::kGL_ARB_tessellation_shader) || HasExtension(GLExt::kGL_OES_tessellation_shader) || HasExtension(GLExt::kGL_EXT_tessellation_shader);
        return false;
    }

    bool HasGeometryShader(const ApiGLES & api, GfxDeviceLevelGL level, bool clamped)
    {
        if (IsGfxLevelCore(level, kGfxLevelCore32) || IsGfxLevelES3(level, kGfxLevelES31AEP))
            return true;
        if (!clamped || IsGfxLevelES3(level, kGfxLevelES31))
            return HasExtension(GLExt::kGL_ARB_geometry_shader4) || HasExtension(GLExt::kGL_OES_geometry_shader) || HasExtension(GLExt::kGL_EXT_geometry_shader);
        return false;
    }

    bool HasProgramPointSizeEnable(const ApiGLES & api, GfxDeviceLevelGL level, bool clamped)
    {
        return IsGfxLevelCore(level);
    }

    bool HasBinaryProgram(const ApiGLES & api, const GraphicsCaps & caps, GfxDeviceLevelGL level, bool clamped)
    {
//#       if !CAN_HAVE_SHADER_CACHE
//        return false;
//#       endif//
//
//#       if 0 // Set to 1 to disable binary shader cache, e.g. for debugging the source shader path or to use third-party tools that require shader source
//        return false;
//#       endif
//
//#       if PLATFORM_ANDROID
//        // amazon have issues with binary shaders on all devices
//        if (::strcasecmp(android::systeminfo::Manufacturer(), "Amazon") == 0)
//            return false;
//#       endif//PLATFORM_ANDROID
//
//        // Huawei and Vivante both have broken binary shaders support
//        if ((GetGraphicsCaps().rendererString.find("Immersion") != std::string::npos) || (GetGraphicsCaps().gles.isVivanteGpu))
//            return false;
//
//        // Check extension support
//        bool SupportExtension = false;
//        if (IsGfxLevelCore(level, kGfxLevelCore41))
//            SupportExtension = true;
//        else if (IsGfxLevelES3(level, kGfxLevelES3))
//            SupportExtension = true;
//        else if (!clamped)
//            SupportExtension = HasExtension(GLExt::kGL_OES_get_program_binary) || HasExtension(GLExt::kGL_ARB_get_program_binary);
//        if (!SupportExtension)
//            return false;
//
//        // Check binary format
//        if (api.Get(GL_NUM_PROGRAM_BINARY_FORMATS) == 0)
//            return false;
//
//        InitShaderCacheGLES();
//        return true;
        return false;
    }

    bool HasBinaryProgramRetrievableHint(const ApiGLES & api, const GraphicsCaps & caps, GfxDeviceLevelGL level, bool clamped)
    {
        if (!HasBinaryProgram(api, caps, level, clamped))
            return false;

        return IsGfxLevelCore(level, kGfxLevelCore41) || IsGfxLevelES3(level, kGfxLevelES3) || HasExtension(GLExt::kGL_ARB_get_program_binary);
    }

    int GetTransformFeedbackBufferBindings(const ApiGLES & api, GfxDeviceLevelGL level, bool clamped)
    {
        if (IsGfxLevelES2(level))
            return 0;

        bool hasQueryAPI = false;
        if (IsGfxLevelCore(level, kGfxLevelCore40))
            hasQueryAPI = true;
        else if (!clamped)
            hasQueryAPI = HasExtension(GLExt::kGL_ARB_transform_feedback3);

        int maxTransformFeedbackBufferBindings = 0;
        if (hasQueryAPI)
            maxTransformFeedbackBufferBindings = std::min<int>(api.Get(GL_MAX_TRANSFORM_FEEDBACK_BUFFERS), gl::kMaxTransformFeedbackBufferBindings);

        if (IsGfxLevelCore(level, kGfxLevelCore40) && hasQueryAPI)
            return maxTransformFeedbackBufferBindings;
        if (IsGfxLevelCore(level, kGfxLevelCore32))
            return 1;
        if (IsGfxLevelES3(level, kGfxLevelES3))
            return 1;
        return 0;
    }

    int GetMaxVertexUniforms(const ApiGLES & api, GfxDeviceLevelGL level)
    {
        if (IsGfxLevelES2(level))
            return api.Get(GL_MAX_VERTEX_UNIFORM_VECTORS) * 4; // GLES2 expresses uniform variables with vector of 4 components
        else
            return api.Get(GL_MAX_VERTEX_UNIFORM_COMPONENTS);
    }

    bool HasBlitFramebuffer(const ApiGLES & api, GfxDeviceLevelGL level, bool clamped)
    {
        if (IsGfxLevelCore(level, kGfxLevelCore32) || IsGfxLevelES3(level, kGfxLevelES3))
            return true;
        if (!clamped)
            return (HasExtension(GLExt::kGL_NV_framebuffer_blit) && HasExtension(GLExt::kGL_NV_read_buffer)) || HasExtension(GLExt::kGL_ARB_framebuffer_blit);
        return false;
    }

    bool HasFramebufferColorRead(const ApiGLES & api, const GraphicsCaps & caps, GfxDeviceLevelGL level, bool clamped)
    {
//        // OSX drivers bug: OSX supports ES2_compability but generates an invalid operation error when calling glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_FORMAT)
//#       if PLATFORM_OSX
//        return false;
//#       endif
//
//        if (IsGfxLevelES(level))
//            return true;
//        if (IsGfxLevelCore(level, kGfxLevelCore41))
//            return !caps.gles.isIntelGpu; // Intel drivers (May 2015) have a bug and generates an invalid operation error on glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_FORMAT, ...)
//        if (!clamped)
//            return HasExtension(GLExt::kGL_ARB_ES2_compatibility) && !caps.gles.isIntelGpu;
        return false;
    }

    bool HasMultisampleBlitScaled(const ApiGLES & api, GfxDeviceLevelGL level, bool clamped)
    {
        if (!clamped)
            return HasExtension(GLExt::kGL_EXT_framebuffer_multisample_blit_scaled);
        return false;
    }

    bool HasPackedDepthStencil(const ApiGLES & api, GfxDeviceLevelGL level)
    {
        if (IsGfxLevelCore(level, kGfxLevelCore32) || IsGfxLevelES3(level, kGfxLevelES3) || PLATFORM_WEBGL)
            return true;
        if (IsGfxLevelES2(level))
            return HasExtension(GLExt::kGL_OES_packed_depth_stencil) || HasExtension(GLExt::kGL_EXT_packed_depth_stencil);
        return false;
    }

    bool HasBuggyRenderTargetDepthAndStencil(const ApiGLES & api, const GraphicsCaps & caps, GfxDeviceLevelGL level, bool clamped)
    {
//        if (IsGfxLevelCore(level, kGfxLevelCore32) || IsGfxLevelES3(level, kGfxLevelES3))
//            return false;
//        if (IsGfxLevelES2(level))
//        {
//            // Adreno 2xx seems to not like a texture attached to color & depth & stencil at once;
//            const bool isAdreno2xx = caps.gles.isAdrenoGpu && caps.gles.isES2Gpu;
//            if (isAdreno2xx)
//                return true;
//        }
        return false;
    }

    bool HasBuggyMSAA(const GraphicsCaps & caps)
    {
#if PLATFORM_OSX
        if (caps.gles.isIntelGpu &&
            (caps.rendererString.find("6000") != std::string::npos ||
             caps.rendererString.find("6100") != std::string::npos ||
             caps.rendererString.find("6200") != std::string::npos)
            && systeminfo::GetOperatingSystemNumeric() >= 101300)
            return true;
#endif
        return false;
    }

    GLint GetMaxAASamples(const ApiGLES & api, GfxDeviceLevelGL level, GLenum maxAASamplesConst)
    {
#       if PLATFORM_ANDROID

        // Prior to Android API level 21, GL_MAX_SAMPLES_EXT is not present in gl2ext.h and some
        // implementations emit a GL_INVALID_ERROR when it is queried, even if they support the
        // GL_EXT_multisampled_render_to_texture (which states that GL_MAX_SAMPLES_EXT can be
        // queried).
        //
        // In this case, we still should query the max samples value, but be prepared to ignore
        // errors to avoid breaking development builds where GL errors == crash.
        if (maxAASamplesConst == GL_MAX_SAMPLES && IsGfxLevelES2(level) && android::systeminfo::ApiLevel() < android::apiLollipop)
        {
            while (api.glGetError() != GL_NONE)
                ;
            GLint result = 0;
            api.glGetIntegerv(maxAASamplesConst, &result);

            const bool success =
#if DEBUG_GL_ERROR_CHECKS
                CheckErrorGLES(&api, "OPENGL ERROR ignored while querying GL_MAX_SAMPLES_EXT", __FILE__, __LINE__)
#else
                (api.glGetError() == GL_NONE)
#endif
            ;

            return success ? result : 0;
        }
#endif

        return api.Get(maxAASamplesConst);
    }

    bool HasInvalidateFramebuffer(const ApiGLES & api, GfxDeviceLevelGL level, bool clamped)
    {
//        // Only use InvalidateFramebuffer on GLES (non-desktop, non-editor)
//        // It'll only benefit us on mobile tiled GPUs and risks causing driver issues (case 1108929)
//        // This also avoids showing garbage in Editor
//        if (UNITY_EDITOR || !IsGfxLevelES(level))
//            return false;
//        if (IsGfxLevelCore(level, kGfxLevelCore43) || IsGfxLevelES3(level, kGfxLevelES3))
//            return true;
//        if (!clamped)
//            return HasExtension(GLExt::kGL_ARB_invalidate_subdata) || HasExtension(GLExt::kGL_EXT_discard_framebuffer);
        return false;
    }

    NPOTCaps HasNPOT(const ApiGLES& api, const GraphicsCaps & caps, GfxDeviceLevelGL level, bool clamped)
    {
//        if (!IsGfxLevelES2(level))
//            return kNPOTFull;
//        if (!clamped && !caps.gles.isVideoCoreGpu) // VideoCore claims to support full npot, but only has limited capabilities
//            return HasExtension(GLExt::kGL_OES_texture_npot) ? kNPOTFull : kNPOTRestricted;
        return kNPOTRestricted;
    }

    bool HasMipMaxLevel(const ApiGLES& api, GfxDeviceLevelGL level, bool clamped)
    {
        if (!IsGfxLevelES2(level))
            return true;
        if (!clamped)
            return HasExtension(GLExt::kGL_APPLE_texture_max_level);
        return false;
    }

    bool HasTextureStorage(const ApiGLES & api, const GraphicsCaps & caps, GfxDeviceLevelGL level, bool clamped)
    {
        // June2015 - In Catalyst 13.9 last drivers for Radeon HD 4000 series and older,
        // creating a cube map texture with texture storage generates and error.
        const int version = caps.gles.majorVersion * 10 + caps.gles.minorVersion;
        if (PLATFORM_WIN && caps.gles.isAMDGpu && version < 40)
            return false;

        if (IsGfxLevelCore(level, kGfxLevelCore42) || IsGfxLevelES3(level, kGfxLevelES3))
            return true;
        if (!clamped)
        {
            if (HasExtension(GLExt::kGL_ARB_texture_storage))
                return true;

            if (HasExtension(GLExt::kGL_EXT_texture_storage))
            {
                // GL_EXT_texture_storage does not guarantee a complete mipmap chain for ES 2.0,
                // so we either need TEXTURE_MAX_LEVEL or an ES 3 driver.
                if (!IsGfxLevelES2(level))
                    return true;

                if (HasMipMaxLevel(api, level, clamped))
                    return true;

                return caps.gles.driverGLESVersion > 2;
            }
        }
        return false;
    }

    bool HasAlphaLumTextureStorage(const ApiGLES & api, const GraphicsCaps & caps, GfxDeviceLevelGL level, bool clamped)
    {
        return IsGfxLevelES2(level) ? HasTextureStorage(api, caps, level, clamped) : false;
    }

    bool HasTextureSwizzle(const ApiGLES & api, const GraphicsCaps & caps, GfxDeviceLevelGL level, bool clamped)
    {
        if (PLATFORM_WEBGL) // WebGL doesn't support BGRA texture format or texture swizzle, even WebGL 2.0
            return false;

#       if PLATFORM_ANDROID
        // Mali GPUs have broken texture swizzle on Android 6, case 788821.
        if (caps.gles.isMaliGpu && android::systeminfo::ApiLevel() == android::apiMarshmallow)
            return false;
#       endif

        // Old AMD kernel driver has a texture swizzle bug. Disable texture swizzles for these GPUs.
        if (caps.rendererString.find("Radeon HD 2") != std::string::npos)
            return false;
        if (caps.rendererString.find("Radeon HD 3") != std::string::npos)
            return false;
        if (caps.rendererString.find("Radeon HD 4") != std::string::npos)
            return false;

        if (IsGfxLevelCore(level, kGfxLevelCore33) || IsGfxLevelES3(level, kGfxLevelES3))
            return true;
        if (!clamped)
            return HasExtension(GLExt::kGL_ARB_texture_swizzle) || HasExtension(GLExt::kGL_EXT_texture_swizzle);
        return false;
    }

    bool Has2DArrayTexture(const ApiGLES & api, const GraphicsCaps & caps, GfxDeviceLevelGL level, bool clamped)
    {
        if (caps.gles.isVivanteGpu)
            return false; // Vivante ES3 driver generates GL errors when trying to use texture arrays

        return !IsGfxLevelES2(level);
    }

    bool HasCubemapArrayTexture(const ApiGLES & api, const GraphicsCaps & caps, GfxDeviceLevelGL level, bool clamped)
    {
        if (IsGfxLevelCore(level, kGfxLevelCore40) || IsGfxLevelES3(level, kGfxLevelES31AEP))
            return true;

        // Cube map arrays crash at launch on NVIDIA Shield Tablet with ES 2 (possibly other devices).
        // Allow them for ES 3 and above though.
        if (IsGfxLevelES2(level))
            return false;

        if (!clamped)
            return HasExtension(GLExt::kGL_ARB_texture_cube_map_array) || HasExtension(GLExt::kGL_OES_texture_cube_map_array) || HasExtension(GLExt::kGL_EXT_texture_cube_map_array);
        return false;
    }

    bool HasSeamlessCubemapEnable(const ApiGLES& api, const GraphicsCaps& caps, GfxDeviceLevelGL level, bool /*clamped*/)
    {
        // Some older Nvidia GPUs (at least 8600M and 9600) have a bug in seamless cubemaps
        // corrupting all rendering on OSX.
        if (PLATFORM_OSX && caps.gles.isNvidiaGpu && level < kGfxLevelCore40)
            return false;

        return IsGfxLevelCore(level);// || HasARGV("force-desktop-glcontext");
    }

    bool HasMultiSampleAutoResolve(const ApiGLES & api, const GraphicsCaps & caps, GfxDeviceLevelGL level, bool clamped)
    {
        if (!clamped)
            return HasExtension(GLExt::kGL_EXT_multisampled_render_to_texture) || HasExtension(GLExt::kGL_IMG_multisampled_render_to_texture);
        return false;
    }

    bool HasMultisample(const ApiGLES & api, const GraphicsCaps & caps, GfxDeviceLevelGL level, bool clamped)
    {
        // June 2015 - Intel Sandy Bridge generate an invalid operation error on glEnable(GL_MULTISAMPLE)
        const int version = caps.gles.majorVersion * 10 + caps.gles.minorVersion;
        if (PLATFORM_WIN && caps.gles.isIntelGpu && version < 32)
            return false;

        if (caps.gles.buggyMSAA)
        {
            //WarningStringWithoutStacktrace("MSAA support is disabled on High Sierra for this GPU");
            WarningString("MSAA support is disabled on High Sierra for this GPU");
            return false;
        }

        if (IsGfxLevelCore(level) || IsGfxLevelES3(level, kGfxLevelES3))
            return true;
        if (HasMultiSampleAutoResolve(api, caps, level, clamped))
            return true;
        if (!clamped)
            return HasExtension(GLExt::kGL_ARB_framebuffer_object) || HasExtension(GLExt::kGL_APPLE_framebuffer_multisample) || (HasExtension(GLExt::kGL_NV_framebuffer_multisample) && HasExtension(GLExt::kGL_NV_framebuffer_blit));
        return false;
    }

    bool HasClearBuffer(const ApiGLES & api, GfxDeviceLevelGL level, bool clamped)
    {
        if (IsGfxLevelCore(level, kGfxLevelCore32) || IsGfxLevelES3(level))
            return true;
        return false;
    }

    bool HasClearDepthFloat(const ApiGLES & api, const GraphicsCaps & caps, GfxDeviceLevelGL level, bool clamped)
    {
        if (IsGfxLevelCore(level, kGfxLevelCore41) || IsGfxLevelES(level))
            return true;
        if (!clamped)
            return HasExtension(GLExt::kGL_ARB_ES2_compatibility);
        return false;
    }

    bool HasClipDistance(const ApiGLES & api, GfxDeviceLevelGL level, bool clamped)
    {
        if (IsGfxLevelCore(level))
            return true;
        if (!clamped)
            return HasExtension(GLExt::kGL_EXT_clip_cull_distance);
        return false;
    }

    bool HasDepthClamp(const ApiGLES & api, GfxDeviceLevelGL level, bool clamped)
    {
        if (IsGfxLevelCore(level))
            return true;
        if (!clamped)
            return HasExtension(GLExt::kGL_ARB_depth_clamp);
        return false;
    }

    bool HasConservativeRaster()
    {
        return HasExtension(GLExt::kGL_NV_conservative_raster);
    }

    bool HasTexSRGBDecode(const ApiGLES & api, GfxDeviceLevelGL level, bool clamped)
    {
        if (IsGfxLevelES3(level, kGfxLevelES31AEP))
            return true;

        if (!clamped)
            return HasExtension(GLExt::kGL_EXT_texture_sRGB_decode);
        return false;
    }

    bool HasMapbuffer(const ApiGLES & api, GfxDeviceLevelGL level, bool clamped)
    {
#       if PLATFORM_WEBGL
        return false;
#       endif

        if (IsGfxLevelCore(level))
            return true;
        if (!clamped)
            return HasExtension(GLExt::kGL_OES_mapbuffer) || HasExtension(GLExt::kGL_ARB_vertex_buffer_object);
        return false;
    }

    bool HasMapbufferRange(const ApiGLES & api, GfxDeviceLevelGL level, bool clamped)
    {
#       if PLATFORM_WEBGL
        return false;
#       endif

        if (IsGfxLevelCore(level, kGfxLevelCore32) || IsGfxLevelES3(level, kGfxLevelES3))
            return true;
        if (!clamped)
            return HasExtension(GLExt::kGL_EXT_map_buffer_range) || HasExtension(GLExt::kGL_ARB_map_buffer_range);
        return false;
    }

    bool HasCircularBuffer(const ApiGLES & api, const GraphicsCapsGLES & capsGles, GfxDeviceLevelGL level, bool clamped)
    {
#if PLATFORM_ANDROID || PLATFORM_IOS || PLATFORM_TVOS
        // Android: Circular buffer is significantly slower in performance tests (DynamicBatchingLitCubes) on Adreno and Mali drivers
        // On Tegra K1 using circular buffers causes broken geometry
        // iOS/tvOS: Circular buffer seems to cause some gliches (case 785036) and in general not properly tested for perf
        return false;
#endif
        return HasMapbufferRange(api, level, clamped);
    }

    bool HasBufferCopy(const ApiGLES & api, GfxDeviceLevelGL level, bool clamped)
    {
        if (IsGfxLevelCore(level, kGfxLevelCore32) || IsGfxLevelES3(level, kGfxLevelES3))
            return true;
        if (!clamped)
            return HasExtension(GLExt::kGL_ARB_copy_buffer);
        return false;
    }

    bool HasBufferClear(const ApiGLES & api, GfxDeviceLevelGL level, bool clamped)
    {
        if (IsGfxLevelCore(level, kGfxLevelCore44))
            return true;
        if (!clamped)
            return HasExtension(GLExt::kGL_ARB_clear_buffer_object);
        return false;
    }

    bool Has16BitFloatVertex(const ApiGLES & api, GfxDeviceLevelGL level, bool clamped)
    {
        if (IsGfxLevelCore(level, kGfxLevelCore32) || IsGfxLevelES3(level, kGfxLevelES3))
            return true;
        if (!clamped)
            return HasExtension(GLExt::kGL_OES_vertex_half_float);
        return false;
    }

    bool HasBlendSeparateMRT(const ApiGLES & api, GfxDeviceLevelGL level, bool clamped)
    {
        if (PLATFORM_OSX && GetGraphicsCaps().gles.isAMDVegaGpu) // MacOS OpenGL core driver of per-MRT blend is buggy when claimed to be supported (case 988072)
            return false;

        if (IsGfxLevelCore(level, kGfxLevelCore40) || IsGfxLevelES3(level, kGfxLevelES32))
            return true;
        // ColorMaski is since GL3.0, and per-MRT blend modes are in an extension - need both to be supported
        if (IsGfxLevelCore(level, kGfxLevelCore32) && HasExtension(GLExt::kGL_ARB_draw_buffers_blend))
            return true;

        return false;
    }

    bool HasBlendAdvanced(const ApiGLES & api, GfxDeviceLevelGL level, bool clamped)
    {
        if (IsGfxLevelES3(level, kGfxLevelES31AEP))
            return true;
        if (!clamped)
            return HasExtension(GLExt::kGL_KHR_blend_equation_advanced) || HasExtension(GLExt::kGL_NV_blend_equation_advanced);
        return false;
    }

    bool HasBlendAdvancedCoherent(const ApiGLES & api, GfxDeviceLevelGL level, bool clamped)
    {
        if (!clamped)
            return HasExtension(GLExt::kGL_KHR_blend_equation_advanced_coherent) || HasExtension(GLExt::kGL_NV_blend_equation_advanced_coherent);
        return false;
    }

    bool HasDisjointTimerQuery(const ApiGLES & api, GfxDeviceLevelGL level, bool clamped)
    {
        if (!clamped)
            return HasExtension(GLExt::kGL_EXT_disjoint_timer_query);
        return false;
    }

    bool HasTimerQuery(const ApiGLES & api, GfxDeviceLevelGL level, bool clamped)
    {
//        // FrameTimingGLES uses the same target GL_TIME_ELAPSED as GfxTimerQuery.
//        // Using it at the same time as GfxTimerQuery would cause problems.
//        // FrameTimingGLES is only enabled for GL_EXT_disjoint_timer_query.
//        if (GetPlayerSettings().enableFrameTimingStats && HasDisjointTimerQuery(api, level, clamped))
//            return false;
//
//        // TODO Linux: This fails on wayland, investigate before shipping wayland support
//        if (IsGfxLevelCore(level, kGfxLevelCore33))
//            return true;
//
//        if (!clamped)
//            return HasExtension(GLExt::kGL_ARB_timer_query) || HasExtension(GLExt::kGL_NV_timer_query) || HasDisjointTimerQuery(api, level, clamped);
        return false;
    }

    bool HasInternalformat(const ApiGLES & api, GfxDeviceLevelGL level, bool clamped)
    {
        if (IsGfxLevelCore(level, kGfxLevelCore43))
            return true;
        if (!clamped)
            return HasExtension(GLExt::kGL_ARB_internalformat_query2);
        return false;
    }

    bool HasFramebufferSRGBEnable(const ApiGLES & api, const GraphicsCaps & caps, GfxDeviceLevelGL level)
    {
//        // Some devices support sRGB framebuffer enable but not all. It's only useful for us for the legacy UI.
//        if (PLATFORM_ANDROID || UNITY_APPLE_PVR)
//            return false;
//        if (IsGfxLevelCore(level))
//            return true;
//        return HasExtension(GLExt::kGL_EXT_sRGB_write_control) || HasExtension(GLExt::kGL_ARB_framebuffer_sRGB);
        return false;
    }

    bool HasSRGBReadWrite(const ApiGLES& api, const GraphicsCaps& caps, GfxDeviceLevelGL level)
    {
//        if (PLATFORM_ANDROID && IsGfxLevelES2(level))
//            return false;
//
//#       if PLATFORM_WIN
//        bool HasWGLColorSpace();
//
//        if (IsGfxLevelES(level))
//        {
//            // With the editor, we need to disable the sRGB conversion with glDisable(GL_FRAMEBUFFER_SRGB) in gamma mode
//#               if UNITY_EDITOR
//            return HasWGLColorSpace() && HasExtension(GLExt::kGL_EXT_sRGB_write_control);
//#               else
//            return HasWGLColorSpace();
//#               endif
//        }
//#       endif
//
//        // EXT_sRGB for sRGB support in WebGL 1.0
//        if (!IsGfxLevelES2(level) || (UNITY_WEBGL && HasExtension(WebGLExt::kEXT_sRGB)))
//            return true;
//
//        // All we really need on OpenGL ES 2.0 is GL_EXT_sRGB but it means the compressed textures will be decompressed
//        // With OpenGL ES 2.0:
//        // - On iOS GL_EXT_sRGB and GL_EXT_pvrtc_sRGB are supported on SGX 543 and SGX 554, which represent ~97% of the device on June 2016
//        // - On Android, the offenders are Mali 4XX (~30%), PowerVR 5 (~6%) and Videocore IV (2%), the rest supports GL_EXT_sRGB
//        // - On WebGL, only Chrome implementation supports it
//        return
//                (UNITY_APPLE_PVR && HasExtension(GLExt::kGL_EXT_sRGB) && HasExtension(GLExt::kGL_EXT_pvrtc_sRGB)) || // ES2 support in iOS
//                (UNITY_DESKTOP && (HasExtension(GLExt::kGL_EXT_sRGB) && (HasExtension(GLExt::kGL_NV_sRGB_formats) || caps.gles.isIntelGpu))); // ES2 support on desktop
        return false;
    }

    bool HasTextureView(const ApiGLES& api, GfxDeviceLevelGL level, bool clamped)
    {
        if (IsGfxLevelCore(level, kGfxLevelCore43))
            return true;
        if (!clamped)
            return HasExtension(GLExt::kGL_ARB_texture_view) || HasExtension(GLExt::kGL_OES_texture_view) || HasExtension(GLExt::kGL_EXT_texture_view);
        return false;
    }

    bool HasS3TC(const ApiGLES& api, GfxDeviceLevelGL level, bool clamped)
    {
        if (IsGfxLevelCore(level))
            return true;

        if (!clamped)
#if PLATFORM_ANDROID
            // NVidia Tegra Android devices have GL_EXT_texture_compression_s3tc flag so they dont fallback from unsupported DXT formats this results in artifacts
            if (GetGraphicsCaps().gles.isTegraGpu)
                return false;
#endif
            return HasExtension(GLExt::kGL_EXT_texture_compression_s3tc) || HasExtension(WebGLExt::kWEBGL_compressed_texture_s3tc) || HasExtension(WebGLExt::kWEBKIT_WEBGL_compressed_texture_s3tc);
        return false;
    }

    bool HasS3TC_SRGB(const ApiGLES& api, const GraphicsCaps& caps, GfxDeviceLevelGL level, bool clamped)
    {
        if (IsGfxLevelCore(level))
            return true;

        if (!clamped)
            return HasExtension(GLExt::kGL_EXT_sRGB) || HasExtension(GLExt::kGL_NV_sRGB_formats) || caps.gles.isIntelGpu || HasExtension(WebGLExt::kWEBGL_compressed_texture_s3tc_srgb);
        return false;
    }

    bool HasRGTC(const ApiGLES & api, GfxDeviceLevelGL level, bool clamped)
    {
        if (IsGfxLevelCore(level))
            return true;
        if (!clamped)
            return HasExtension(GLExt::kGL_ARB_texture_compression_rgtc) || HasExtension(GLExt::kGL_EXT_texture_compression_rgtc);
        return false;
    }

    bool HasBPTC(const ApiGLES & api, GfxDeviceLevelGL level, bool clamped)
    {
        if (IsGfxLevelCore(level, kGfxLevelCore42))
            return true;
        if (!clamped)
            return HasExtension(GLExt::kGL_ARB_texture_compression_bptc);
        return false;
    }

    bool HasPVRTC(const ApiGLES& api, GfxDeviceLevelGL level, bool clamped)
    {
        if (!clamped)
            return HasExtension(GLExt::kGL_IMG_texture_compression_pvrtc) || HasExtension(WebGLExt::kWEBGL_compressed_texture_pvrtc) || HasExtension(WebGLExt::kWEBKIT_WEBGL_compressed_texture_pvrtc);
        return false;
    }

    bool HasPVRTC_SRGB(const ApiGLES& api, GfxDeviceLevelGL level, bool clamped)
    {
        if (!clamped)
            return HasExtension(GLExt::kGL_EXT_pvrtc_sRGB);
        return false;
    }

    bool HasASTC(const ApiGLES & api, GfxDeviceLevelGL level, bool clamped)
    {
        if (IsGfxLevelES3(level, kGfxLevelES31AEP))
            return true;
        if (!clamped)
            return HasExtension(GLExt::kGL_KHR_texture_compression_astc_ldr) || HasExtension(WebGLExt::kWEBGL_compressed_texture_astc_ldr);
        return false;
    }

    bool HasASTC_HDR(const ApiGLES & api, GfxDeviceLevelGL level, bool clamped)
    {
        if (!clamped)
            return HasExtension(GLExt::kGL_KHR_texture_compression_astc_hdr);
        return false;
    }

    bool HasASTCDecodeMode(const ApiGLES & api, GfxDeviceLevelGL level, bool clamped)
    {
        if (!clamped)
            return HasExtension(GLExt::kGL_EXT_texture_compression_astc_decode_mode);
        return false;
    }

    bool HasASTCDecodeModeRGB9E5(const ApiGLES & api, GfxDeviceLevelGL level, bool clamped)
    {
        if (!clamped)
            return HasExtension(GLExt::kGL_EXT_texture_compression_astc_decode_mode_rgb9e5);
        return false;
    }

    bool HasASTCSliced3D(const ApiGLES & api, GfxDeviceLevelGL level, bool clamped)
    {
        if (!clamped)
            return HasExtension(GLExt::kGL_KHR_texture_compression_astc_hdr) || HasExtension(GLExt::kGL_KHR_texture_compression_astc_sliced_3d);
        return false;
    }

    bool HasETC1(const ApiGLES & api, GfxDeviceLevelGL level, bool clamped)
    {
        if (!clamped)
            return HasExtension(GLExt::kGL_OES_compressed_ETC1_RGB8_texture) || HasExtension(WebGLExt::kWEBGL_compressed_texture_etc1);
        return false;
    }

    bool HasETC2AndEAC(const ApiGLES & api, const GraphicsCaps & caps, GfxDeviceLevelGL level, bool clamped)
    {
//        // ETC2 and EAC are in OpenGL 4.3 through GL_ARB_ES3_compatibility but most desktop implementations support these formats through online decompression
//        // In March 2015, AMD had no intention to support any format of ETC2 or EAC in their drivers
//        if (caps.gles.isAMDGpu)
//            return false;
//        if (IsGfxLevelCore(level, kGfxLevelCore43) || (IsGfxLevelES3(level, kGfxLevelES3) && !UNITY_WEBGL))
//            return true;
//        if (!clamped)
//            return HasExtension(GLExt::kGL_ARB_ES3_compatibility) || HasExtension(WebGLExt::kWEBGL_compressed_texture_es3);
        return false;
    }

    bool HasFloatTexture(const ApiGLES & api, const GraphicsCaps & caps, GfxDeviceLevelGL level, bool clamped)
    {
        if (!IsGfxLevelES2(level))
            return true;
        if (!clamped)
            return HasExtension(GLExt::kGL_OES_texture_float);
        return false;
    }

    bool HasHalfTexture(const ApiGLES & api, const GraphicsCaps & caps, GfxDeviceLevelGL level, bool clamped)
    {
        if (!IsGfxLevelES2(level))
            return true;
        if (!clamped)
            return HasExtension(GLExt::kGL_OES_texture_half_float);
        return false;
    }

    bool HasPackedFloatTexture(const ApiGLES & api, const GraphicsCaps & caps, GfxDeviceLevelGL level, bool clamped)
    {
        if (!IsGfxLevelES2(level))
            return true;
        if (!clamped)
            return HasExtension(GLExt::kGL_APPLE_texture_packed_float) || HasExtension(GLExt::kGL_NV_packed_float);
        return false;
    }

    bool HasRenderToRG32F(const ApiGLES& api, const GraphicsCaps& caps, GfxDeviceLevelGL level, bool clamped)
    {
        if (IsGfxLevelCore(level) || IsGfxLevelES3(level, kGfxLevelES32))
            return true;
        if (!clamped)
            return HasExtension(GLExt::kGL_EXT_color_buffer_float);
        return false;
    }

    bool HasRenderToRGBA32F(const ApiGLES& api, const GraphicsCaps& caps, GfxDeviceLevelGL level, bool clamped)
    {
        if (IsGfxLevelCore(level) || IsGfxLevelES3(level, kGfxLevelES32))
            return true;
        if (!clamped)
        {
            // PowerVR Rogue on GLES 2 reports extension support, but returns errors when used (case 1106542)
            bool buggyFloatBuffer = (PLATFORM_ANDROID && caps.gles.isPvrGpu);
#           if PLATFORM_ANDROID
            // Adreno 3XX reports kGL_EXT_color_buffer_float but returns incomplete frame buffer errors on ES2 on Android <5
            buggyFloatBuffer |= (IsGfxLevelES2(level) && android::systeminfo::ApiLevel() < android::apiLollipop && caps.rendererString.find("Adreno (TM) 3") != std::string::npos);
#           endif
            return (HasExtension(WebGLExt::kWEBGL_color_buffer_float) || HasExtension(GLExt::kGL_EXT_color_buffer_float)) && !buggyFloatBuffer;
        }
        return false;
    }

    bool HasRenderToRG16F(const ApiGLES& api, const GraphicsCaps& caps, GfxDeviceLevelGL level, bool clamped)
    {
        if (IsGfxLevelCore(level) || IsGfxLevelES3(level, kGfxLevelES32))
            return true;
        if (!clamped && HasExtension(GLExt::kGL_EXT_color_buffer_float)) // GL_EXT_color_buffer_float is a superset extension supporting half and float on ES2, ES3 and WebGL 2.0
        {
            //Adreno 3xx reports GL_EXT_color_buffer_float but returns invalid operation when R16F or RG16F render target are used on ES2
            if (caps.gles.isAdrenoGpu && IsGfxLevelES2(level) && caps.rendererString.find("Adreno (TM) 3") != std::string::npos)
                return false;
            return true;
        }
        if (!clamped && HasExtension(GLExt::kGL_EXT_color_buffer_half_float)) // On ES2: GL_EXT_color_buffer_half_float && GL_EXT_texture_rg && GL_OES_texture_half_float; on ES3: GL_EXT_color_buffer_half_float && ES3
            return IsGfxLevelES3(level, kGfxLevelES3) || (HasExtension(GLExt::kGL_EXT_texture_rg) && HasExtension(GLExt::kGL_OES_texture_half_float));
        return false;
    }

    bool HasRenderToRGBA16F(const ApiGLES& api, const GraphicsCaps& caps, GfxDeviceLevelGL level, bool clamped)
    {
        if (IsGfxLevelCore(level) || IsGfxLevelES3(level, kGfxLevelES32))
            return true;
        if (!clamped)
        {
            // PowerVR Rogue on GLES 2 reports extension support, but returns errors when used (case 1106542)
            bool buggyHalfBuffer = (PLATFORM_ANDROID && caps.gles.isPvrGpu);
#           if PLATFORM_ANDROID
            // Adreno 3XX reports kGL_EXT_color_buffer_half_float but returns incomplete frame buffer errors on ES2 on Android <5.0
            buggyHalfBuffer |= (IsGfxLevelES2(level) && android::systeminfo::ApiLevel() < android::apiLollipop && caps.rendererString.find("Adreno (TM) 3") != std::string::npos);
#           endif
            return (HasExtension(GLExt::kGL_EXT_color_buffer_float) || HasExtension(GLExt::kGL_EXT_color_buffer_half_float)) && !buggyHalfBuffer;
        }
        return false;
    }

    bool HasRenderToRGB9E5F(const ApiGLES& api, const GraphicsCaps& caps, GfxDeviceLevelGL level, bool clamped)
    {
        if (!clamped)
            return (IsGfxLevelES3(level, kGfxLevelES3) || HasExtension(GLExt::kGL_APPLE_texture_packed_float)) && HasExtension(GLExt::kGL_APPLE_color_buffer_packed_float);
        return false;
    }

    bool HasRenderToRG11B10F(const ApiGLES& api, const GraphicsCaps& caps, GfxDeviceLevelGL level, bool clamped)
    {
        if (IsGfxLevelCore(level) || IsGfxLevelES3(level, kGfxLevelES31AEP))
            return true;
        if (!clamped)
        {
            if (IsGfxLevelES3(level, kGfxLevelES3))
                return HasExtension(GLExt::kGL_EXT_color_buffer_float) || HasExtension(GLExt::kGL_APPLE_color_buffer_packed_float);
            else
                return (HasExtension(GLExt::kGL_APPLE_texture_packed_float) && HasExtension(GLExt::kGL_APPLE_color_buffer_packed_float)) || HasExtension(GLExt::kGL_NV_packed_float);
        }
        return false;
    }

    bool HasTextureRG(const ApiGLES& api, GfxDeviceLevelGL level, bool clamped)
    {
        if (IsGfxLevelES3(level, kGfxLevelES3) || IsGfxLevelCore(level))
            return true;
        if (!clamped)
            return HasExtension(GLExt::kGL_EXT_texture_rg);
        return false;
    }

    bool HasNativeDepthTexture(const ApiGLES & api, const GraphicsCaps & caps, GfxDeviceLevelGL level)
    {
        if (IsGfxLevelES2(level))
        {
#           if PLATFORM_ANDROID && defined(__i386__)
            if (caps.gles.isPvrGpu && android::systeminfo::ApiLevel() < android::apiLollipop)     // avoid buggy drivers on Intel devices
                return false;
#           endif

            return HasExtension(GLExt::kGL_OES_depth_texture) || HasExtension(GLExt::kGL_GOOGLE_depth_texture) || HasExtension(WebGLExt::kWEBGL_depth_texture) || HasExtension(GLExt::kGL_WEBGL_depth_texture) || HasExtension(GLExt::kGL_ARB_depth_texture);
        }
        return true;
    }

    bool HasTexLodSamplers(const ApiGLES & api, GfxDeviceLevelGL level)
    {
        if (IsGfxLevelES2(level))
            return HasExtension(GLExt::kGL_EXT_shader_texture_lod);

        return true;
    }

    bool HasManualMipmaps(const ApiGLES& api, GfxDeviceLevelGL level)
    {
        if (IsGfxLevelES2(level))
#if PLATFORM_WEBGL
            return false;
#else
            return HasExtension(GLExt::kGL_OES_fbo_render_mipmap);
#endif

        return true;
    }

    bool HasStereoscopic3D(const ApiGLES & api, GfxDeviceLevelGL level)
    {
        if (IsGfxLevelCore(level))
            return api.Get(GL_STEREO) == GL_TRUE;
        return false;
    }

    bool HasFenceSync(const ApiGLES &api, GfxDeviceLevelGL level)
    {
        // Workaround for buggy drivers, causing fences to be super slow (case 827110, case 924891)
#       if PLATFORM_ANDROID
        if (GetGraphicsCaps().driverLibraryString == "V@139.0 (GIT@I1b38baa288)"
            || GetGraphicsCaps().driverLibraryString == "V@145.0 (GIT@I7b6ba47bd6)"
            || GetGraphicsCaps().driverLibraryString == "V@139.0 (GIT@If3abccc389)")
            return false;
#       endif
        // NOTE: fences are quite slow (even just for checking whether we've passed them) on OSX NVidias.
        if (IsGfxLevelCore(level, kGfxLevelCore32) || IsGfxLevelES3(level, kGfxLevelES3))
            return true;
        return false;
    }

    bool HasHighpFloatInFragmentShader(ApiGLES *api, GfxDeviceLevelGL level)
    {
        if (IsGfxLevelES2(level))
        {
            GLint range[2];
            GLint precision;
            GLES_CALL(api, glGetShaderPrecisionFormat, GL_FRAGMENT_SHADER, GL_HIGH_FLOAT, range, &precision);
            //gles2 will return zero for both range and precision if high float is not supported
            return !(precision == 0 && range[0] == 0 && range[1] == 0);
        }

        return true;
    }

    bool HasSampler2dMS(const ApiGLES & api, GfxDeviceLevelGL level)
    {
#       if PLATFORM_WEBGL
        return false;
#       endif

        if (IsGfxLevelCore(level))
            return true;
        return (IsGfxLevelES3(level, kGfxLevelES31));
    }

    bool HasTexStorageMultisample(const ApiGLES & api, GfxDeviceLevelGL level)
    {
#       if PLATFORM_WEBGL
        return false;
#       endif

        if (IsGfxLevelCore(level, kGfxLevelCore43))
            return true;
        return (IsGfxLevelES3(level, kGfxLevelES31));
    }

    bool HasSRPBatcherSupport(const ApiGLES & api, GfxDeviceLevelGL level)
    {
#       if PLATFORM_WEBGL
        return false;
#       endif

        if (IsGfxLevelCore(level, kGfxLevelCore43))
            return true;
        return (IsGfxLevelES3(level, kGfxLevelES31));
    }

    int GetMaxAnisoSamples(const ApiGLES & api, GfxDeviceLevelGL level, bool clamped)
    {
        bool hasAPI = !clamped && (HasExtension(GLExt::kGL_EXT_texture_filter_anisotropic) || HasExtension(WebGLExt::kWEBKIT_EXT_texture_filter_anisotropic));

        return hasAPI ? api.Get(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT) : 1;
    }

    int GetMaxColorAttachments(const ApiGLES & api, GfxDeviceLevelGL level)
    {
        if (!HasDrawBuffers(api, level))
            return 1;

        return clamp<int>(api.Get(GL_MAX_COLOR_ATTACHMENTS), 1, kMaxSupportedRenderTargets);
    }

    bool DisableSoftShadows(const ApiGLES & api)
    {
        return IsGfxLevelES2(GetGraphicsCaps().gles.featureLevel) ? !PLATFORM_WEBGL : false;
    }

    int EstimateVRAM(const ApiGLES* api, const GraphicsCaps *caps)
    {
        return 512;
        
//        (void)api; (void)caps;
//#       if UNITY_APPLE_PVR
//        return systeminfo::GetVideoMemoryMBApple(caps->rendererString);
//
//#       elif PLATFORM_ANDROID || PLATFORM_LUMIN
//        const int total_mem_mb = systeminfo::GetPhysicalMemoryMB();
//        const int physical_mem_mb = NextPowerOfTwo(total_mem_mb);
//        const int video_mem_cutout = physical_mem_mb - total_mem_mb;
//        // Adreno 22x seems to have very limited video memory compared to system memory
//        if (video_mem_cutout && (caps->rendererString.find("Adreno (TM) 22") != std::string::npos))
//            return 256;
//        return physical_mem_mb >> 2; // let's assume we can use at least a quarter of the system memeory
//
//#       elif PLATFORM_WEBGL
//        // The primary target for WebGL are desktops for now, so 512 MB should be available.
//        // If we are going to support mobiles on WebGL, this should probably be revisited
//        return 512;
//
//#       elif PLATFORM_OSX
//        return GetVideoMemoryMBOSX(caps->rendererID);
//
//#       elif PLATFORM_LINUX
//        return GetVideoMemoryMBLinux(api);
//
//#       elif PLATFORM_WIN
//        const char* vramMethod = "";
//        return windriverutils::GetVideoMemorySizeMB(MonitorFromWindow(GetDesktopWindow(), MONITOR_DEFAULTTOPRIMARY), &vramMethod);
//
//#       else
//        // unknown platform: error
//#       error "Unknown OpenGL/ES platform, implement VRAM size estimation"
//#       endif
    }

    bool isAMDVegaGpu(const std::string& rendererString)
    {
        std::string rendererStringCopy = rendererString;
        std::transform(rendererStringCopy.begin(), rendererStringCopy.end(), rendererStringCopy.begin(), ::tolower);
        return rendererStringCopy.find("vega") != std::string::npos;
    }

    void InitVersion(const ApiGLES& api, GfxDeviceLevelGL level, int& majorVersion, int& minorVersion)
    {
        majorVersion = 0;
        minorVersion = 0;

        if (IsGfxLevelES2(level))
        {
            majorVersion = 2;
            minorVersion = 0;
        }
        else
        {
#   if PLATFORM_WEBGL
            // WebGL 2.0 does not seem to support querying GL_MAJOR_VERSION/GL_MINOR_VERSION
            majorVersion = 3;
            minorVersion = 0;
#   else
            majorVersion = api.Get(GL_MAJOR_VERSION);
            minorVersion = api.Get(GL_MINOR_VERSION);
#   endif //PLATFORM_WEBGL
        }
    }

    // Tweaks GL level downwards if not supported by current context
    GfxDeviceLevelGL AdjustAPILevel(GfxDeviceLevelGL level, int majorVersion, int minorVersion)
    {
        GfxDeviceLevelGL maxSupported = kGfxLevelCoreLast;
        if (level < kGfxLevelCoreFirst)
        {
            if (majorVersion == 2)
                maxSupported = kGfxLevelES2;
            if (majorVersion == 3)
            {
                if (minorVersion == 1)
                    maxSupported = kGfxLevelES31;
                else if (minorVersion == 0)
                    maxSupported = kGfxLevelES3;
            }
        }
        else
        {
            if (majorVersion == 3)
            {
                maxSupported = kGfxLevelCore32;
                if (minorVersion >= 3)
                    maxSupported = kGfxLevelCore33;
            }
            else if (majorVersion == 4)
            {
                maxSupported = (GfxDeviceLevelGL)(kGfxLevelCore40 + (GfxDeviceLevelGL)minorVersion);
            }
        }
        return std::min(maxSupported, level);
    }

    bool IsOpenGLES2OnlyGPU(const GfxDeviceLevelGL level, const std::string& renderer)
    {
        if (!IsGfxLevelES2(level))
            return false;

        const char* const es2GpuStrings[] =
                {
                        "Mali-200",
                        "Mali-300",
                        "Mali-400",
                        "Mali-450",
                        "Mali-470",
                        "PowerVR SGX",
                        "Adreno (TM) 2",
                        "Tegra 3",
                        "Tegra 4",
                        "Vivante GC1000",
                        "GC1000 core",
                        "VideoCore IV",
                        "Bluestacks"
                };

        for (int i = 0; i < ARRAY_SIZE(es2GpuStrings); ++i)
        {
            if (renderer.find(es2GpuStrings[i]) != std::string::npos)
                return true;
        }
        return false;
    }

    int ParseAdrenoDriverVersionES3x(std::string versionString)
    {
        // Most Adreno GL_VERSION strings look like this: "OpenGL ES 3.2 V@225.0 (GIT@7142022, Ib5823dd10c) (Date:06/23/17)"
        // This function returns 225 when given that string

        size_t pos = 0;
        const char glesVersionPrefix[] = "OpenGL ES 3.";
        pos = versionString.find(glesVersionPrefix, pos);
        if (pos == std::string::npos)
            return -1;

        pos += sizeof(glesVersionPrefix) - 1;
        if (pos >= versionString.length())
            return -1;

        if (versionString[pos] < '0' || versionString[pos] > '2')
            return -1;

        ++pos;
        const char versionPrefix[] = "V@";
        pos = versionString.find(versionPrefix, pos);
        if (pos == std::string::npos)
            return -1;

        pos += sizeof(versionPrefix) - 1;

        std::string versionStr(versionString.substr(pos));
        int version = StringToInt(versionStr);
        return version <= 0 ? -1 : version;
    }

    GraphicsTier GetActiveTier(const GfxDeviceLevelGL level, const std::string& renderer)
    {
#if PLATFORM_IOS
        // the logic behind this: before A7, GPU ALU were pretty limited, so for better PBR support we need A7+
        // after looking at different alternatives we decided that most robust/full solution would be to have the list of pre-ios7 devices
        // as it will never be updated
        // luckily for us our DeviceGeneration is organized in the order of devices coming to the market, so check is actually quite trivial
        if (UnityDeviceGeneration() < 18 /*iPhone5S*/)
            return kGraphicsTier1;
        else
            return kGraphicsTier2;
#elif PLATFORM_TVOS
        return kGraphicsTier2;
#else
        if (IsGfxLevelES(level))
        {
            if (IsOpenGLES2OnlyGPU(level, renderer))
                return kGraphicsTier1;
            else
                return kGraphicsTier2;
        }
#endif

        return kGraphicsTier3;
    }

    FormatUsageFlags MakeGraphicsFormatFlags(bool supportSampledTexture, bool supportRenderTexture, bool supportLoadStoreTexture, bool supportBlendedRenderTexture, bool supportLinearSampling, bool supportSparseTexture)
    {
        FormatUsageFlags supportsFormatUsageBits = supportSampledTexture ? kUsageSampleBit : kUsageNoneBit;
        supportsFormatUsageBits |= supportLinearSampling ? kUsageLinearBit : kUsageNoneBit;
        supportsFormatUsageBits |= supportSparseTexture && (supportSampledTexture || supportRenderTexture) ? kUsageSparseBit : kUsageNoneBit;
        supportsFormatUsageBits |= supportRenderTexture ? kUsageRenderBit : kUsageNoneBit;
        supportsFormatUsageBits |= supportLoadStoreTexture ? kUsageLoadStoreBit : kUsageNoneBit;
        supportsFormatUsageBits |= supportBlendedRenderTexture ? kUsageBlendBit : kUsageNoneBit;
        return supportsFormatUsageBits;
    }

    void InitDefaultFormat(const ApiGLES& api, GraphicsCaps* caps, const GfxDeviceLevelGL level)
    {
        caps->InitDefaultFormat();

        caps->defaultFormatLDR[kLinearColorSpace] = caps->defaultFormatLDR[kGammaColorSpace] = kFormatR8G8B8A8_UNorm;

        if (caps->IsFormatSupported(kFormatR8G8B8A8_SRGB, kUsageRender))
            caps->defaultFormatLDR[kLinearColorSpace] = kFormatR8G8B8A8_SRGB;

        if (!IsGfxLevelCore(level))
        {
            const FramebufferInfoGLES& fbInfo = api.GetFramebufferInfo();
            if (fbInfo.redBits == 5 && fbInfo.greenBits == 6 && fbInfo.blueBits == 5)
                caps->defaultFormatLDR[kGammaColorSpace] = kFormatB5G6R5_UNormPack16;
        }

        if (GetActiveColorGamut() == kColorGamutDisplayP3 && PlatformIsColorGamutSupported(kColorGamutDisplayP3))
        {
            caps->defaultFormatLDR[kGammaColorSpace] = kFormatR8G8B8A8_UNorm;
        }

        caps->UpdateDefaultLDRFormat();

        if (caps->IsFormatSupported(kFormatR16G16B16A16_SFloat, kUsageRender))
            caps->SetDefaultHDRFormat(kFormatR16G16B16A16_SFloat);
        else if (caps->IsFormatSupported(kFormatR32G32B32A32_SFloat, kUsageRender))
            caps->SetDefaultHDRFormat(kFormatR32G32B32A32_SFloat);
        else
            caps->SetDefaultHDRFormat(caps->GetGraphicsFormat(kDefaultFormatLDR));
    }

    void InitFormatCaps(const ApiGLES& api, GraphicsCaps* caps, const GfxDeviceLevelGL level, bool clamped)
    {
        const bool hasRGB_SRGB = IsGfxLevelCore(level);
        const bool hasRGBA_SRGB = IsGfxLevelCore(level) || IsGfxLevelES3(level, kGfxLevelES3) || (HasExtension(GLExt::kGL_EXT_sRGB) /*&& !UNITY_APPLE_PVR*/);
        const bool hasS3TC = HasS3TC(api, level, clamped);
        const bool hasS3TC_SRGB = HasS3TC_SRGB(api, *caps, level, clamped);
        const bool hasRGTC = HasRGTC(api, level, clamped);
        const bool hasBPTC = HasBPTC(api, level, clamped);
        const bool hasPVRTC = HasPVRTC(api, level, clamped);
        const bool hasPVRTC_SRGB = HasPVRTC_SRGB(api, level, clamped);
        const bool hasASTC = HasASTC(api, level, clamped);
        const bool hasASTC_HDR = HasASTC_HDR(api, level, clamped);
        const bool hasETC2AndEAC = HasETC2AndEAC(api, *caps, level, clamped);
        const bool hasETC1 = HasETC1(api, level, clamped) || hasETC2AndEAC;
        const bool hasTexFloat = HasFloatTexture(api, *caps, level, clamped);
        const bool hasTexHalf = HasHalfTexture(api, *caps, level, clamped);
        const bool hasPackedFloat = HasPackedFloatTexture(api, *caps, level, clamped);
        const bool hasTexR16 = IsGfxLevelCore(level) || (IsGfxLevelES3(level) && HasExtension(GLExt::kGL_EXT_texture_norm16));
        const bool hasTexRGB10A2 = IsGfxLevelES2(caps->gles.featureLevel) ? HasExtension(GLExt::kGL_EXT_texture_type_2_10_10_10_REV) : true;
        const bool hasTexRGB10A2Int = IsGfxLevelCore(level) && !PLATFORM_OSX;
        const bool hasSNorm = !IsGfxLevelES2(level);
        const bool hasRG = HasTextureRG(api, level, clamped);
        const bool hasLuminance = IsGfxLevelES(level);

        const bool hasRTRG32F = HasRenderToRG32F(api, *caps, caps->gles.featureLevel, caps->gles.featureClamped);
        const bool hasRTRGBA32F = HasRenderToRGBA32F(api, *caps, caps->gles.featureLevel, caps->gles.featureClamped);
        const bool hasRTRG16F = HasRenderToRG16F(api, *caps, caps->gles.featureLevel, caps->gles.featureClamped);
        const bool hasRTRGBA16F = HasRenderToRGBA16F(api, *caps, caps->gles.featureLevel, caps->gles.featureClamped);
        const bool hasRTRGB9E5 = HasRenderToRGB9E5F(api, *caps, caps->gles.featureLevel, caps->gles.featureClamped);
        const bool hasRTRG11B10F = HasRenderToRG11B10F(api, *caps, caps->gles.featureLevel, caps->gles.featureClamped);
        const bool hasRTSNormAny = IsGfxLevelCore(level) || HasExtension(GLExt::kGL_EXT_render_snorm);
        const bool hasRTSNormR16 = IsGfxLevelCore(level) || HasExtension(GLExt::kGL_EXT_texture_norm16);

        const bool hasFloatBlend = HasExtension(GLExt::kGL_EXT_float_blend) || IsGfxLevelCore(level);
        const bool hasLoadStoreCore = HasFlag(caps->shaderCaps, kShaderRequireCompute) && IsGfxLevelCore(level);
        const bool hasLoadStoreAny = HasFlag(caps->shaderCaps, kShaderRequireCompute);

        const bool hasBGRA = caps->gles.hasTextureSwizzle || IsGfxLevelCore(level) || HasExtension(GLExt::kGL_APPLE_texture_format_BGRA8888) || HasExtension(GLExt::kGL_EXT_texture_format_BGRA8888) || HasExtension(GLExt::kGL_IMG_texture_format_BGRA8888);

        const bool hasHalfLinearFilter = (IsGfxLevelES2(level) && HasExtension(GLExt::kGL_OES_texture_half_float_linear)) || IsGfxLevelCore(level) || IsGfxLevelES3(level);
        const bool hasFloatLinearFilter = (IsGfxLevelES(level) && HasExtension(GLExt::kGL_OES_texture_float_linear)) || IsGfxLevelCore(level);
#if PLATFORM_WEBGL
        // WebGL 2.0 (based on the OpenGL ES 3.0.2 spec) does not support linear filtering of depth formats, but WebGL 1.0 does.
        const bool hasDepthLinearFilter = IsGfxLevelES3(level) ? false : true;
#else
        const bool hasDepthLinearFilter = IsGfxLevelCore(level);
#endif

        const bool hasDepthStencil = caps->gles.hasPackedDepthStencil;
        const bool hasDepth24 = caps->gles.hasDepth24;

        const bool isES2 = IsGfxLevelES2(level);
        const bool hasRenderInt = !IsGfxLevelES2(level) && !PLATFORM_OSX;
        const bool hasRenderRGBInt = IsGfxLevelCore(level) && !PLATFORM_OSX;

        const bool hasSparse = caps->sparseTextures != kSparseTextureNone;

        FormatUsageFlags supportsFormatUsageBits[] =              //kFormatCount
                {
                        kUsageNoneBit,                                                                                                              // kFormatNone

                        MakeGraphicsFormatFlags(HasExtension(GLExt::kGL_EXT_texture_sRGB_R8), false, false, false, true, false),                    // kFormatR8_SRGB
                        MakeGraphicsFormatFlags(HasExtension(GLExt::kGL_EXT_texture_sRGB_RG8), false, false, false, true, false),                   // kFormatR8G8_SRGB
                        MakeGraphicsFormatFlags(hasRGBA_SRGB, hasRGB_SRGB, false, hasRGB_SRGB, true, false),                                        // kFormatR8G8B8_SRGB
                        MakeGraphicsFormatFlags(hasRGBA_SRGB, hasRGBA_SRGB, false, hasRGBA_SRGB, true, false),                                      // kFormatR8G8B8A8_SRGB

                        MakeGraphicsFormatFlags(hasRG || hasLuminance, hasRG, hasLoadStoreCore, hasRG, true, hasSparse),                            // kFormatR8_UNorm
                        MakeGraphicsFormatFlags(hasRG, hasRG, hasLoadStoreCore, hasRG, true, hasSparse),                                            // kFormatR8G8_UNorm
                        MakeGraphicsFormatFlags(true, true, false, true, true, false),                                                              // kFormatR8G8B8_UNorm
                        MakeGraphicsFormatFlags(true, true, hasLoadStoreAny, true, true, hasSparse),                                                // kFormatR8G8B8A8_UNorm

                        MakeGraphicsFormatFlags(hasSNorm, hasRTSNormAny, hasLoadStoreCore, hasRTSNormAny, true, hasSparse),                         // kFormatR8_SNorm
                        MakeGraphicsFormatFlags(hasSNorm, hasRTSNormAny, hasLoadStoreCore, hasRTSNormAny, true, hasSparse),                         // kFormatR8G8_SNorm
                        MakeGraphicsFormatFlags(hasSNorm, IsGfxLevelCore(level), false, IsGfxLevelCore(level), true, false),                        // kFormatR8G8B8_SNorm
                        MakeGraphicsFormatFlags(hasSNorm, hasRTSNormAny, hasLoadStoreAny, hasRTSNormAny, true, hasSparse),                          // kFormatR8G8B8A8_SNorm

                        MakeGraphicsFormatFlags(!IsGfxLevelES2(level), hasRenderInt, hasLoadStoreCore, false, false, hasSparse),                    // kFormatR8_UInt
                        MakeGraphicsFormatFlags(!IsGfxLevelES2(level), hasRenderInt, hasLoadStoreCore, false, false, hasSparse),                    // kFormatR8G8_UInt
                        MakeGraphicsFormatFlags(!IsGfxLevelES2(level), hasRenderRGBInt, false, false, false, false),                                // kFormatR8G8B8_UInt
                        MakeGraphicsFormatFlags(!IsGfxLevelES2(level), hasRenderInt, hasLoadStoreAny, false, false, hasSparse),                     // kFormatR8G8B8A8_UInt
                        MakeGraphicsFormatFlags(!IsGfxLevelES2(level), hasRenderInt, hasLoadStoreCore, false, false, hasSparse),                    // kFormatR8_SInt
                        MakeGraphicsFormatFlags(!IsGfxLevelES2(level), hasRenderInt, hasLoadStoreCore, false, false, hasSparse),                    // kFormatR8G8_SInt
                        MakeGraphicsFormatFlags(!IsGfxLevelES2(level), hasRenderRGBInt, false, false, false, false),                                // kFormatR8G8B8_SInt
                        MakeGraphicsFormatFlags(!IsGfxLevelES2(level), hasRenderInt, hasLoadStoreAny, false, false, hasSparse),                     // kFormatR8G8B8A8_SInt

                        MakeGraphicsFormatFlags(hasTexR16, hasRTSNormR16, hasLoadStoreCore, hasRTSNormR16, true, hasSparse),                        // kFormatR16_UNorm
                        MakeGraphicsFormatFlags(hasTexR16, hasRTSNormR16, hasLoadStoreCore, hasRTSNormR16, true, hasSparse),                        // kFormatR16G16_UNorm
                        MakeGraphicsFormatFlags(hasTexR16, IsGfxLevelCore(level), false, IsGfxLevelCore(level), true, false),                       // kFormatR16G16B16_UNorm
                        MakeGraphicsFormatFlags(hasTexR16, hasRTSNormR16, hasLoadStoreCore, hasRTSNormR16, true, hasSparse),                        // kFormatR16G16B16A16_UNorm
                        MakeGraphicsFormatFlags(hasTexR16, hasRTSNormAny, hasLoadStoreCore, hasRTSNormAny, true, hasSparse),                        // kFormatR16_SNorm
                        MakeGraphicsFormatFlags(hasTexR16, hasRTSNormAny, hasLoadStoreCore, hasRTSNormAny, true, hasSparse),                        // kFormatR16G16_SNorm
                        MakeGraphicsFormatFlags(hasTexR16, IsGfxLevelCore(level), false, IsGfxLevelCore(level), true, false),                       // kFormatRGB16_SNorm
                        MakeGraphicsFormatFlags(hasTexR16, hasRTSNormAny, hasLoadStoreCore, hasRTSNormAny, true, hasSparse),                        // kFormatR16G16B16A16_SNorm
                        MakeGraphicsFormatFlags(!IsGfxLevelES2(level), hasRenderInt, hasLoadStoreCore, false, false, hasSparse),                    // kFormatR16_UInt
                        MakeGraphicsFormatFlags(!IsGfxLevelES2(level), hasRenderInt, hasLoadStoreCore, false, false, hasSparse),                    // kFormatR16G16_UInt
                        MakeGraphicsFormatFlags(!IsGfxLevelES2(level), hasRenderRGBInt, false, false, false, false),                                // kFormatR16G16B16_UInt
                        MakeGraphicsFormatFlags(!IsGfxLevelES2(level), hasRenderInt, hasLoadStoreAny, false, false, hasSparse),                     // kFormatR16G16B16A16_UInt
                        MakeGraphicsFormatFlags(!IsGfxLevelES2(level), hasRenderInt, hasLoadStoreCore, false, false, hasSparse),                    // kFormatR16_SInt
                        MakeGraphicsFormatFlags(!IsGfxLevelES2(level), hasRenderInt, hasLoadStoreCore, false, false, hasSparse),                    // kFormatR16G16_SInt
                        MakeGraphicsFormatFlags(!IsGfxLevelES2(level), hasRenderRGBInt, false, false, false, false),                                // kFormatR16G16B16_SInt
                        MakeGraphicsFormatFlags(!IsGfxLevelES2(level), hasRenderInt, hasLoadStoreAny, false, false, hasSparse),                     // kFormatR16G16B16A16_SInt

                        MakeGraphicsFormatFlags(!IsGfxLevelES2(level), hasRenderInt, hasLoadStoreAny, false, false, hasSparse),                     // kFormatR32_UInt
                        MakeGraphicsFormatFlags(!IsGfxLevelES2(level), hasRenderInt, hasLoadStoreCore, false, false, hasSparse),                    // kFormatR32G32_UInt
                        MakeGraphicsFormatFlags(!IsGfxLevelES2(level), hasRenderRGBInt, false, false, false, hasSparse),                            // kFormatR32G32B32_UInt
                        MakeGraphicsFormatFlags(!IsGfxLevelES2(level), hasRenderInt, hasLoadStoreAny, false, false, hasSparse),                     // kFormatR32G32B32A32_UInt
                        MakeGraphicsFormatFlags(!IsGfxLevelES2(level), hasRenderInt, hasLoadStoreAny, false, false, hasSparse),                     // kFormatR32_SInt
                        MakeGraphicsFormatFlags(!IsGfxLevelES2(level), hasRenderInt, hasLoadStoreCore, false, false, hasSparse),                    // kFormatR32G32_SInt
                        MakeGraphicsFormatFlags(!IsGfxLevelES2(level), hasRenderRGBInt, false, false, false, hasSparse),                            // kFormatR32G32B32_SInt
                        MakeGraphicsFormatFlags(!IsGfxLevelES2(level), hasRenderInt, hasLoadStoreAny, false, false, hasSparse),                     // kFormatR32G32B32A32_SInt

                        MakeGraphicsFormatFlags(hasRG && hasTexHalf, hasRTRG16F, hasLoadStoreCore, hasRTRG16F, hasHalfLinearFilter, hasSparse),             // kFormatR16_SFloat
                        MakeGraphicsFormatFlags(hasRG && hasTexHalf, hasRTRG16F, hasLoadStoreCore, hasRTRG16F, hasHalfLinearFilter, hasSparse),             // kFormatR16G16_SFloat
                        MakeGraphicsFormatFlags(hasTexHalf, IsGfxLevelCore(level), false, IsGfxLevelCore(level), hasHalfLinearFilter, false),               // kFormatR16G16B16_SFloat
                        MakeGraphicsFormatFlags(hasTexHalf, hasRTRGBA16F, hasLoadStoreAny, hasRTRGBA16F, hasHalfLinearFilter, hasSparse),                   // kFormatR16G16B16A16_SFloat
                        MakeGraphicsFormatFlags(hasRG && hasTexFloat, hasRTRG32F, hasLoadStoreAny, hasRTRG32F && hasFloatBlend, hasFloatLinearFilter, hasSparse),       // kFormatR32_SFloat
                        MakeGraphicsFormatFlags(hasRG && hasTexFloat, hasRTRG32F, hasLoadStoreCore, hasRTRG32F && hasFloatBlend, hasFloatLinearFilter, hasSparse),      // kFormatR32G32_SFloat
                        MakeGraphicsFormatFlags(hasTexFloat, IsGfxLevelCore(level), false, IsGfxLevelCore(level) && hasFloatBlend, hasFloatLinearFilter, false),        // kFormatR32G32B32_SFloat
                        MakeGraphicsFormatFlags(hasTexFloat, hasRTRGBA32F, hasLoadStoreAny, hasRTRGBA32F && hasFloatBlend, hasFloatLinearFilter, hasSparse),            // kFormatR32G32B32A32_SFloat

                        MakeGraphicsFormatFlags(IsGfxLevelES2(level), false, false, false, true, false),                                                    // kFormatL8_UNorm
                        MakeGraphicsFormatFlags((hasRG && caps->gles.hasTextureSwizzle) || IsGfxLevelES2(level), false, false, false, true, false),         // kFormatA8_UNorm
                        MakeGraphicsFormatFlags(hasTexR16 && caps->gles.hasTextureSwizzle, false, false, false, true, false),                               // kFormatA16_UNorm

                        MakeGraphicsFormatFlags(caps->gles.hasTextureSwizzle, false, false, false, true, false),                                            // kFormatB8G8R8_SRGB
                        MakeGraphicsFormatFlags(hasBGRA, false, false, false, true, false),                                                                 // kFormatB8G8R8A8_SRGB
                        MakeGraphicsFormatFlags(caps->gles.hasTextureSwizzle, false, false, false, true, false),                                            // kFormatB8G8R8_UNorm
                        MakeGraphicsFormatFlags(hasBGRA, false, false, false, true, false),                                                                 // kFormatB8G8R8A8_UNorm
                        MakeGraphicsFormatFlags(!IsGfxLevelES2(level), false, false, false, true, false),                                                   // kFormatBGR8_SNorm
                        MakeGraphicsFormatFlags(!IsGfxLevelES2(level), false, false, false, true, false),                                                   // kFormatB8G8R8A8_SNorm
                        MakeGraphicsFormatFlags(!IsGfxLevelES2(level), false, false, false, false, false),                                                  // kFormatBGR8_UInt
                        MakeGraphicsFormatFlags(!IsGfxLevelES2(level), false, false, false, false, false),                                                  // kFormatB8G8R8A8_UInt
                        MakeGraphicsFormatFlags(!IsGfxLevelES2(level), false, false, false, false, false),                                                  // kFormatBGR8_SInt
                        MakeGraphicsFormatFlags(!IsGfxLevelES2(level), false, false, false, false, false),                                                  // kFormatB8G8R8A8_SInt

                        MakeGraphicsFormatFlags(true, true, false, false, true, false),                                                                     // kFormatR4G4B4A4_UNormPack16
                        MakeGraphicsFormatFlags(false, true, false, false, true, false),                                                                    // kFormatB4G4R4A4_UNormPack16
                        MakeGraphicsFormatFlags(IsGfxLevelCore(level), true, false, false, true, hasSparse),                                                // kFormatR5G6B5_UNormPack16
                        MakeGraphicsFormatFlags(true, true, false, false, true, false),                                                                     // kFormatB5G6R5_UNormPack16
                        MakeGraphicsFormatFlags(IsGfxLevelES(level), true, false, false, true, false),                                                      // kFormatR5G5B5A1_UNormPack16
                        MakeGraphicsFormatFlags(!UNITY_DESKTOP, true, false, false, true, false),                                                           // kFormatB5G5R5A1_UNormPack16
                        MakeGraphicsFormatFlags(IsGfxLevelCore(level), IsGfxLevelCore(level), false, false, true, false),                                   // kFormatA1RGB5_UNorm

                        MakeGraphicsFormatFlags(hasPackedFloat, hasRTRGB9E5, false, hasRTRGB9E5, true, hasSparse),                                          // kFormatE5B9G9R9_UFloatPack32
                        MakeGraphicsFormatFlags(hasPackedFloat, hasRTRG11B10F, hasLoadStoreCore, hasRTRG11B10F, true, hasSparse),                           // kFormatB10G11R11_UFloatPack32

                        MakeGraphicsFormatFlags(hasTexRGB10A2, hasTexRGB10A2, hasLoadStoreCore, hasTexRGB10A2, true, hasSparse),                            // kFormatA2B10G10R10_UNormPack32
                        MakeGraphicsFormatFlags(hasTexRGB10A2Int, hasTexRGB10A2Int, hasLoadStoreCore, false, true, hasSparse),                              // kFormatA2B10G10R10_UIntPack32
                        MakeGraphicsFormatFlags(false, false, false, false, false, false),                                                                  // kFormatA2B10G10R10_SIntPack32
                        MakeGraphicsFormatFlags(!IsGfxLevelES2(level), false, false, false, true, false),                                                   // kFormatA2R10G10B10_UNormPack32
                        MakeGraphicsFormatFlags(!IsGfxLevelES2(level), false, false, false, true, false),                                                   // kFormatA2R10G10B10_UintPack32
                        kUsageNoneBit,                                                                   // kFormatA2R10G10B10_SintPack32
                        kUsageNoneBit,                                                                   // kFormatA2R10G10B10_XRSRGBPack32
                        kUsageNoneBit,                                                                   // kFormatA2R10G10B10_XRUNormPack32
                        kUsageNoneBit,                                                                   // kFormatR10G10B10_XRSRGBPack32
                        kUsageNoneBit,                                                                   // kFormatR10G10B10_XRUNormPack32
                        kUsageNoneBit,                                                                   // kFormatA10R10G10B10_XRSRGBPack32
                        kUsageNoneBit,                                                                   // kFormatA10R10G10B10_XRUNormPack32

                        MakeGraphicsFormatFlags(false, hasBGRA && hasRGBA_SRGB, false, hasBGRA && hasRGBA_SRGB, true, false),                   // kFormatA8R8G8B8_SRGB
                        MakeGraphicsFormatFlags(false, hasBGRA, false, hasBGRA, true, false),                 // kFormatA8R8G8B8_UNorm
                        kUsageNoneBit,                                                                                  // kFormatA32R32G32B32_SFloat

                        MakeGraphicsFormatFlags(true, true, false, true, hasDepthLinearFilter, false),                                                 // kFormatD16_UNorm
                        MakeGraphicsFormatFlags(hasDepth24, hasDepth24, false, hasDepth24, hasDepthLinearFilter, false),                               // kFormatD24_UNorm

                        // With some drivers (old ES 2.0, e.g. Tegra 3) we interpret kFormatD24_UNorm_S8_UInt as something else, see ApiTranslateGLES.cpp
                        // So always mark it as supported for use as RenderTexture
                        MakeGraphicsFormatFlags(hasDepthStencil, true, false, hasDepthStencil, hasDepthLinearFilter, false),                           // kFormatD24_UNorm_S8_UInt
                        MakeGraphicsFormatFlags(!isES2, !isES2, false, !isES2, hasDepthLinearFilter, false),                                           // kFormatD32_SFloat
                        MakeGraphicsFormatFlags(!isES2, !isES2, false, !isES2, hasDepthLinearFilter, false),                                           // kFormatD32_SFloat_S8_Uint
                        MakeGraphicsFormatFlags(!isES2, !isES2, false, !isES2, true, false),                                                           // kFormatS8_Uint

                        MakeGraphicsFormatFlags(hasS3TC_SRGB, false, false, false, true, false),                    // kFormatRGB_DXT1_SRGB
                        MakeGraphicsFormatFlags(hasS3TC, false, false, false, true, false),                         // kFormatRGB_DXT1_UNorm
                        MakeGraphicsFormatFlags(hasS3TC_SRGB, false, false, false, true, false),                    // kFormatRGBA_DXT3_SRGB
                        MakeGraphicsFormatFlags(hasS3TC, false, false, false, true, false),                         // kFormatRGBA_DXT3_UNorm
                        MakeGraphicsFormatFlags(hasS3TC_SRGB, false, false, false, true, false),                    // kFormatRGBA_DXT5_SRGB
                        MakeGraphicsFormatFlags(hasS3TC, false, false, false, true, false),                         // kFormatRGBA_DXT5_UNorm
                        MakeGraphicsFormatFlags(hasRGTC, false, false, false, true, false),                         // kFormatR_BC4_UNorm
                        MakeGraphicsFormatFlags(hasRGTC, false, false, false, true, false),                         // kFormatR_BC4_SNorm
                        MakeGraphicsFormatFlags(hasRGTC, false, false, false, true, false),                         // kFormatRG_BC5_UNorm
                        MakeGraphicsFormatFlags(hasRGTC, false, false, false, true, false),                         // kFormatRG_BC5_SNorm
                        MakeGraphicsFormatFlags(hasBPTC, false, false, false, true, false),                         // kFormatRGB_BC6H_UFloat
                        MakeGraphicsFormatFlags(hasBPTC, false, false, false, true, false),                         // kFormatRGB_BC6H_SFloat
                        MakeGraphicsFormatFlags(hasBPTC, false, false, false, true, false),                         // kFormatRGBA_BC7_SRGB
                        MakeGraphicsFormatFlags(hasBPTC, false, false, false, true, false),                         // kFormatRGBA_BC7_UNorm

                        MakeGraphicsFormatFlags(hasPVRTC_SRGB, false, false, false, true, false),                   // kFormatRGB_PVRTC_2Bpp_SRGB
                        MakeGraphicsFormatFlags(hasPVRTC, false, false, false, true, false),                        // kFormatRGB_PVRTC_2Bpp_UNorm
                        MakeGraphicsFormatFlags(hasPVRTC_SRGB, false, false, false, true, false),                   // kFormatRGB_PVRTC_4Bpp_SRGB
                        MakeGraphicsFormatFlags(hasPVRTC, false, false, false, true, false),                        // kFormatRGB_PVRTC_4Bpp_UNorm
                        MakeGraphicsFormatFlags(hasPVRTC_SRGB, false, false, false, true, false),                   // kFormatRGBA_PVRTC_2Bpp_SRGB
                        MakeGraphicsFormatFlags(hasPVRTC, false, false, false, true, false),                        // kFormatRGBA_PVRTC_2Bpp_UNorm
                        MakeGraphicsFormatFlags(hasPVRTC_SRGB, false, false, false, true, false),                   // kFormatRGBA_PVRTC_4Bpp_SRGB
                        MakeGraphicsFormatFlags(hasPVRTC, false, false, false, true, false),                        // kFormatRGBA_PVRTC_4Bpp_UNorm

                        MakeGraphicsFormatFlags(hasETC1, false, false, false, true, false),                         // kFormatRGB_ETC_UNorm
                        MakeGraphicsFormatFlags(hasETC2AndEAC, false, false, false, true, false),                   // kFormatRGB_ETC2_SRGB
                        MakeGraphicsFormatFlags(hasETC2AndEAC, false, false, false, true, false),                   // kFormatRGB_ETC2_UNorm
                        MakeGraphicsFormatFlags(hasETC2AndEAC, false, false, false, true, false),                   // kFormatRGB_A1_ETC2_SRGB
                        MakeGraphicsFormatFlags(hasETC2AndEAC, false, false, false, true, false),                   // kFormatRGB_A1_ETC2_UNorm
                        MakeGraphicsFormatFlags(hasETC2AndEAC, false, false, false, true, false),                   // kFormatRGBA_ETC2_SRGB
                        MakeGraphicsFormatFlags(hasETC2AndEAC, false, false, false, true, false),                   // kFormatRGBA_ETC2_UNorm
                        MakeGraphicsFormatFlags(hasETC2AndEAC, false, false, false, true, false),                   // kFormatR_EAC_UNorm
                        MakeGraphicsFormatFlags(hasETC2AndEAC, false, false, false, true, false),                   // kFormatR_EAC_SNorm
                        MakeGraphicsFormatFlags(hasETC2AndEAC, false, false, false, true, false),                   // kFormatRG_EAC_UNorm
                        MakeGraphicsFormatFlags(hasETC2AndEAC, false, false, false, true, false),                   // kFormatRG_EAC_SNorm

                        MakeGraphicsFormatFlags(hasASTC, false, false, false, true, false),                         // kFormatRGBA_ASTC4X4_SRGB
                        MakeGraphicsFormatFlags(hasASTC, false, false, false, true, false),                         // kFormatRGBA_ASTC4X4_UNorm
                        MakeGraphicsFormatFlags(hasASTC, false, false, false, true, false),                         // kFormatRGBA_ASTC5X5_SRGB
                        MakeGraphicsFormatFlags(hasASTC, false, false, false, true, false),                         // kFormatRGBA_ASTC5X5_UNorm
                        MakeGraphicsFormatFlags(hasASTC, false, false, false, true, false),                         // kFormatRGBA_ASTC6X6_SRGB
                        MakeGraphicsFormatFlags(hasASTC, false, false, false, true, false),                         // kFormatRGBA_ASTC6X6_UNorm
                        MakeGraphicsFormatFlags(hasASTC, false, false, false, true, false),                         // kFormatRGBA_ASTC8X8_SRGB
                        MakeGraphicsFormatFlags(hasASTC, false, false, false, true, false),                         // kFormatRGBA_ASTC8X8_UNorm
                        MakeGraphicsFormatFlags(hasASTC, false, false, false, true, false),                         // kFormatRGBA_ASTC10X10_SRGB
                        MakeGraphicsFormatFlags(hasASTC, false, false, false, true, false),                         // kFormatRGBA_ASTC10X10_UNorm
                        MakeGraphicsFormatFlags(hasASTC, false, false, false, true, false),                         // kFormatRGBA_ASTC12X12_SRGB
                        MakeGraphicsFormatFlags(hasASTC, false, false, false, true, false),                         // kFormatRGBA_ASTC12X12_UNorm

                        MakeGraphicsFormatFlags(PLATFORM_ANDROID ? true : false, false, false, false, true, false), // kFormatYUV2

                        MakeGraphicsFormatFlags(false, caps->hasNativeDepthTexture, false, false, hasDepthLinearFilter, false),     // kFormatDepthAuto
                        MakeGraphicsFormatFlags(false, caps->hasNativeShadowMap, false, false, true, false),        // kFormatShadowAuto
                        kUsageNoneBit,                                                                               // kFormatVideoAuto

                        MakeGraphicsFormatFlags(hasASTC_HDR, false, false, false, true, false),                     // kFormatRGBA_ASTC4X4_UFloat
                        MakeGraphicsFormatFlags(hasASTC_HDR, false, false, false, true, false),                     // kFormatRGBA_ASTC5X5_UFloat
                        MakeGraphicsFormatFlags(hasASTC_HDR, false, false, false, true, false),                     // kFormatRGBA_ASTC6X6_UFloat
                        MakeGraphicsFormatFlags(hasASTC_HDR, false, false, false, true, false),                     // kFormatRGBA_ASTC8X8_UFloat
                        MakeGraphicsFormatFlags(hasASTC_HDR, false, false, false, true, false),                     // kFormatRGBA_ASTC10X10_UFloat
                        MakeGraphicsFormatFlags(hasASTC_HDR, false, false, false, true, false),                     // kFormatRGBA_ASTC12X12_UFloat
                };
        CompileTimeAssertArraySize(supportsFormatUsageBits, kGraphicsFormatCount);

        std::copy(supportsFormatUsageBits, &supportsFormatUsageBits[0] + ARRAY_SIZE(supportsFormatUsageBits), caps->supportsFormatUsageBits);

        for (int i = 0; i < kGraphicsFormatCount; ++i)
        {
            const GraphicsFormat format = static_cast<GraphicsFormat>(i);

            FormatUsageFlags perFormatUsageFlags = kUsageNoneBit;

            if (supportsFormatUsageBits[i] & kUsageSampleBit)
            {
                if (caps->SupportsFormatUsageGetPixel(format))
                    perFormatUsageFlags |= kUsageGetPixelsBit;
                if (caps->SupportsFormatUsageSetPixel(format))
                    perFormatUsageFlags |= kUsageSetPixelsBit;
            }

            if (supportsFormatUsageBits[i] & kUsageRenderBit)
            {
                if (caps->SupportsFormatUsageReadback(format))
                    perFormatUsageFlags |= kUsageReadPixelsBit;
            }

            if (GetGraphicsCaps().SupportsFormatUsagePixels32(format) && (caps->supportsFormatUsageBits[i] & kUsageSampleBit))
                caps->supportsFormatUsageBits[i] |= kUsageSetPixels32Bit;

            caps->supportsFormatUsageBits[i] |= perFormatUsageFlags;
        }

#if ENABLE_TEXTURE_STREAMING
        #if !PLATFORM_WEBGL
        caps->supportsMipStreaming = true;
#endif
#endif
    }
}//namespace

GraphicsCapsGLES::GraphicsCapsGLES()
{
    memset(this, 0x00000000, sizeof(GraphicsCapsGLES));
    // cannot do any non-zero initialization here, GraphicCaps::GraphicsCaps() will memset it to 0 later.
}

namespace gles
{
    void InitRenderTextureAACaps(ApiGLES* api, GraphicsCaps* caps)
    {
        const int maxSamples = caps->gles.maxAASamples;
        if (maxSamples <= 1)
            return;

        std::vector<GLint> samples; //(kMemTempAlloc);

        for (int i = 0; i < kGraphicsFormatCount; ++i)
        {
            const GraphicsFormat format = static_cast<GraphicsFormat>(i);

            if (caps->supportsFormatUsageBits[i] & kUsageRenderBit)
            {
                FormatUsageFlags supportedMSAA = kUsageNoneBit;

                if (maxSamples >= 2)
                    supportedMSAA |= kUsageMSAA2Bit;
                if (maxSamples >= 4)
                    supportedMSAA |= kUsageMSAA4Bit;
                if (maxSamples >= 8)
                    supportedMSAA |= kUsageMSAA8Bit;
                if (maxSamples >= 16)
                    supportedMSAA |= kUsageMSAA16Bit;
                if (maxSamples >= 32)
                    supportedMSAA |= kUsageMSAA32Bit;

                if (!IsGfxLevelES2(caps->gles.featureLevel))
                {
                    api->QuerySampleCounts(GL_RENDERBUFFER, api->translate.GetFormatDesc(format, false).internalFormat, samples);
                    FormatUsageFlags supportedMSAAPerFormat = kUsageNoneBit;
                    for (GLint sampleCount : samples)
                    {
                        switch (sampleCount)
                        {
                            case 2: supportedMSAAPerFormat |= kUsageMSAA2Bit; break;
                            case 4: supportedMSAAPerFormat |= kUsageMSAA4Bit; break;
                            case 8: supportedMSAAPerFormat |= kUsageMSAA8Bit; break;
                            case 16: supportedMSAAPerFormat |= kUsageMSAA16Bit; break;
                            case 32: supportedMSAAPerFormat |= kUsageMSAA32Bit; break;
                        }
                    }

                    supportedMSAA &= supportedMSAAPerFormat;
                }

                caps->supportsFormatUsageBits[i] |= supportedMSAA;
            }
        }
    }

    void InitCaps(ApiGLES* api, GraphicsCaps* caps, GfxDeviceLevelGL &level, const std::vector<std::string>& allExtensions)
    {
        Assert(api && caps);
        Assert(caps->gles.featureLevel == level);
        Assert(IsGfxLevelCore(level) || IsGfxLevelES(level));

        g_GraphicsCapsGLES = &caps->gles;
        ::InitVersion(*api, level, caps->gles.majorVersion, caps->gles.minorVersion);

        caps->hasGraphicsFormat = true;

        caps->gles.featureLevel = level = ::AdjustAPILevel(level, caps->gles.majorVersion, caps->gles.minorVersion);
        caps->has16BitReadPixel = IsGfxLevelES(level);

        caps->shaderCaps = gl::GetLevelDesc(level).shaderCaps;

        GetVideoCardIDs(caps->vendorID, caps->rendererID);

        caps->vendorString = api->GetDriverString(gl::kDriverQueryVendor);
        caps->rendererString = api->GetDriverString(gl::kDriverQueryRenderer);
        caps->driverVersionString = api->GetDriverString(gl::kDriverQueryVersion);

        caps->gles.featureClamped = false; //HasARGV("force-clamped");
        const bool clamped = caps->gles.featureClamped;

        // Distill
        //    driverVersionString = "OpenGL ES 2.0 build 1.8@905891"
        // into
        //    driverLibraryString = "build 1.8@905891"
        //
        // See http://www.khronos.org/opengles/sdk/1.1/docs/man/glGetString.xml
        //
        caps->driverLibraryString = "n/a";
        caps->gles.driverGLESVersion = 0;
        std::vector<std::string> parts;
        //int no = SplitString(parts, caps->driverVersionString, " ", 4);
        core::Split(caps->driverVersionString, ' ', parts, 4);
        int no = parts.size();

        if (no >= 3)
        {
            if (parts[0] == "OpenGL" && parts[1] == "ES")
            {
                caps->gles.driverGLESVersion = atoi(parts[2].c_str());
                if (no >= 4)
                    caps->driverLibraryString = parts[3];
            }
        }

        if (no >= 1 && parts[0] == "OpenGL")
        {
            caps->fixedVersionString = caps->driverVersionString;
        }
        else
        {
            // add OpenGL prefix if missing to give valid value on SystemInfo.graphicsDeviceVersion
            caps->fixedVersionString = "OpenGL ";
            caps->fixedVersionString.append(caps->driverVersionString);
        }

#       if ENABLE_EGL && !PLATFORM_LUMIN
        ContextGLES::GetVSyncIntervalRange(NULL, &(caps->maxVSyncInterval));

        if (PLATFORM_ANDROID && caps->maxVSyncInterval > 1)
            caps->maxVSyncInterval = 1;     // Most drivers ignore swapInterval, so cap it
#       elif PLATFORM_STANDALONE && !PLATFORM_OSX
        caps->maxVSyncInterval = 4;
#       else
        // OS X only actually supports values of 1 in my testing and docs seem to hint at the same:
        // "The swap interval can be set only to 0 or 1", from
        // (https://developer.apple.com/library/archive/documentation/GraphicsImaging/Conceptual/OpenGL-MacProgGuide/opengl_designstrategies/opengl_designstrategies.html#//apple_ref/doc/uid/TP40001987-CH2-SW4)
        caps->maxVSyncInterval = 1;
#       endif

        caps->usesOpenGLTextureCoords = true;

        caps->gles.isVideoCoreGpu = (caps->rendererString.find("VideoCore") != std::string::npos);
        // OpenGL is going away from Apple platforms, but treat it as PowerVR for the remaining time
        caps->gles.isPvrGpu = (caps->rendererString.find("PowerVR") != std::string::npos || caps->rendererString.find("Apple") != std::string::npos);
        caps->gles.isMaliGpu = (caps->rendererString.find("Mali") != std::string::npos);
        caps->gles.isAdrenoGpu = (caps->rendererString.find("Adreno") != std::string::npos);
        caps->gles.isTegraGpu = (caps->rendererString.find("Tegra") != std::string::npos);
        caps->gles.isIntelGpu = (caps->rendererString.find("Intel") != std::string::npos);
        caps->gles.isNvidiaGpu = (caps->rendererString.find("NVIDIA") != std::string::npos);
        caps->gles.isAMDGpu = (caps->rendererString.find("AMD") != std::string::npos) || (caps->rendererString.find("ATI") != std::string::npos);
        caps->gles.isAMDVegaGpu = ::isAMDVegaGpu(caps->rendererString);
        caps->gles.isVivanteGpu = (caps->vendorString.find("Vivante") != std::string::npos);
        caps->gles.isES2Gpu = IsOpenGLES2OnlyGPU(level, caps->rendererString);

        bool hasAdrenoHSR = false;
#if PLATFORM_ANDROID
        bool isAdreno2 = false;
        bool isAdreno3 = false;
        bool isAdreno4 = false;
        bool isAdreno5 = false;
        bool isAdreno6 = false;
        int adrenoDriverVersionES3x = -1;

        if (caps->gles.isAdrenoGpu)
        {
            isAdreno2 = caps->gles.isES2Gpu;
            isAdreno3 = caps->rendererString.find("Adreno (TM) 3") != std::string::npos;
            isAdreno4 = caps->rendererString.find("Adreno (TM) 4") != std::string::npos;
            isAdreno5 = caps->rendererString.find("Adreno (TM) 5") != std::string::npos;
            isAdreno6 = caps->rendererString.find("Adreno (TM) 6") != std::string::npos;
            hasAdrenoHSR = !isAdreno2 && !isAdreno3 && !isAdreno4;

            if (isAdreno3)
                caps->skyboxProjectionEpsilonFactor = 10.0f;

            adrenoDriverVersionES3x = ParseAdrenoDriverVersionES3x(caps->driverVersionString);
        }
#endif  // PLATFORM_ANDROID


        caps->hasTiledGPU = caps->gles.isPvrGpu || caps->gles.isAdrenoGpu || caps->gles.isMaliGpu || caps->gles.isVivanteGpu;
        caps->hasHiddenSurfaceRemovalGPU = caps->gles.isPvrGpu || hasAdrenoHSR;
        caps->usesLoadStoreActions = ::HasInvalidateFramebuffer(*api, level, clamped);

        if (IsGfxLevelES2(level))
        {
            bool noDynamicIndex = caps->gles.isVivanteGpu ||    // Vivante GC1000 just hangs while compiling shaders
                                  caps->gles.isTegraGpu;                          // Tegra is slower when dynamic indexing is enabled
            caps->hasDynamicUniformArrayIndexing = !PLATFORM_WEBGL && !noDynamicIndex;
        }
        else
            caps->hasDynamicUniformArrayIndexing = true;

        caps->hasNonFullscreenClear = false; // glClear doesn't obey viewport settings, draw a quad instead.

#if UNITY_DESKTOP
        caps->gles.advanceBufferManagerFrameAfterSwapBuffers = true;
#elif PLATFORM_ANDROID
        caps->gles.advanceBufferManagerFrameAfterSwapBuffers = caps->gles.isMaliGpu;
#endif

        if (GetGraphicsCaps().gles.isPvrGpu)
        {
            caps->gles.useClearToAvoidRestore = true;
        }
        else if (GetGraphicsCaps().gles.isMaliGpu)
        {
            // There is a problem in the Mali Bifrost driver when using glClear in some situations (case 1065919)
            const bool isMaliBifrost = caps->rendererString.find("Mali-G") != std::string::npos;
            caps->gles.useClearToAvoidRestore = !isMaliBifrost || !HasInvalidateFramebuffer(*api, level, clamped);
        }
        caps->gles.useDiscardToAvoidRestore = !caps->gles.useClearToAvoidRestore && ::HasInvalidateFramebuffer(*api, level, clamped);

        caps->gles.supportsManualMipmaps = ::HasManualMipmaps(*api, level);

        caps->videoMemoryMB         = ::EstimateVRAM(api, caps);
        caps->hasStereoscopic3D     = ::HasStereoscopic3D(*api, level);

        caps->gles.hasDebug         = ::HasDebug(*api, *caps, level);
        caps->gles.hasDebugMarker   = ::HasDebugMarker(*api, *caps, level);
        caps->gles.hasDebugLabel    = ::HasDebugLabel(*api, *caps, level);
        caps->gles.hasNVNLZ         = !clamped && HasExtension(GLExt::kGL_NV_depth_nonlinear);
        caps->gles.hasNVCSAA        = !clamped && HasExtension(GLExt::kGL_NV_coverage_sample);

        const bool hasGeometry = ::HasGeometryShader(*api, level, clamped);
        const bool hasTessellation = ::HasTessellationShader(*api, *caps, level, clamped);

        caps->maxRandomWrites       = 0;
        if (::HasComputeShader(*api, level, clamped))
        {
            caps->shaderCaps |= kShaderRequireCompute;

            caps->gles.maxShaderStorageBufferBindings = std::min<int>(api->Get(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS), gl::kMaxShaderStorageBufferBindings);
            caps->gles.maxAtomicCounterBufferBindings = std::min<int>(api->Get(GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS), gl::kMaxAtomicCounterBufferBindings);

            const int maxSSBOs = std::min(api->Get(GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS), caps->gles.maxShaderStorageBufferBindings);
            caps->maxRandomWrites = std::min<int>(api->Get(GL_MAX_IMAGE_UNITS), maxSSBOs);

            caps->maxComputeBufferInputsCompute  = std::min<int>(caps->gles.maxShaderStorageBufferBindings, api->Get(GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS));
            caps->maxComputeBufferInputsVertex   = std::min<int>(caps->gles.maxShaderStorageBufferBindings, api->Get(GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS));
            caps->maxComputeBufferInputsFragment = std::min<int>(caps->gles.maxShaderStorageBufferBindings, api->Get(GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS));
            caps->maxComputeBufferInputsDomain   = hasTessellation ? std::min<int>(caps->gles.maxShaderStorageBufferBindings, api->Get(GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS)) : 0;
            caps->maxComputeBufferInputsHull     = hasTessellation ? std::min<int>(caps->gles.maxShaderStorageBufferBindings, api->Get(GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS)) : 0;
            caps->maxComputeBufferInputsGeometry = hasGeometry ? std::min<int>(caps->gles.maxShaderStorageBufferBindings, api->Get(GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS)) : 0;
            caps->maxComputeBufferInputsCompute  = std::min<int>(kMaxSupportedComputeResources, caps->maxComputeBufferInputsCompute);

            caps->maxComputeWorkGroupSize   = api->Get(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS);
            caps->maxComputeWorkGroupSizeX  = api->Get(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0);
            caps->maxComputeWorkGroupSizeY  = api->Get(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1);
            caps->maxComputeWorkGroupSizeZ  = api->Get(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2);

            caps->shaderCaps = SetOrClearFlags(caps->shaderCaps, kShaderRequireRandomWrite, caps->maxRandomWrites > 0);
        }
        caps->gles.hasIndirectDraw      = ::HasIndirectDraw(*api, level, clamped);
        caps->gles.hasDrawBaseVertex    = ::HasDrawBaseVertex(*api, level, clamped);

        // -- Version --

        ::printf_console("Renderer: %s\n", caps->rendererString.c_str());
        ::printf_console("Vendor:   %s\n", caps->vendorString.c_str());
        ::printf_console("Version:  %s\n", caps->driverVersionString.c_str());
        ::printf_console("GLES:     %d\n", caps->gles.driverGLESVersion);

        const std::string& extensionString = GetExtensionsString(allExtensions);

        if (extensionString.size())
            DebugTextLineByLine(extensionString.c_str());
        else
            ::printf_console("glGetString(GL_EXTENSIONS) - failure");

        // -- Handle tokens referencing the same feature between ES2 and other GL API --
        caps->gles.memoryBufferTargetConst      = ::HasBufferCopy(*api, level, clamped) ? gl::kCopyWriteBuffer : gl::kArrayBuffer;

        caps->gles.buggyMSAA                    = ::HasBuggyMSAA(*caps);

        // -- Multisampling --

        caps->hasMultiSampleAutoResolve         = ::HasMultiSampleAutoResolve(*api, *caps, level, clamped);
        caps->hasMultiSample                    = ::HasMultisample(*api, *caps, level, clamped);
        if (caps->hasMultiSample || caps->hasMultiSampleAutoResolve)
        {
            const GLenum maxAASamplesConst      = HasExtension(GLExt::kGL_IMG_multisampled_render_to_texture) ? GL_MAX_SAMPLES_IMG : GL_MAX_SAMPLES;
            caps->gles.maxAASamples             = GetMaxAASamples(*api, level, maxAASamplesConst);

            // This is true for some older PVR devices (GT-I9000)
            if (caps->gles.maxAASamples == 0)
            {
                caps->hasMultiSampleAutoResolve = false;
                caps->hasMultiSample            = false;
                caps->gles.maxAASamples         = 1;
            }
        }
        else
        {
            caps->gles.maxAASamples             = 1;
        }

        caps->usesStoreAndResolveAction = !caps->hasMultiSampleAutoResolve;

        // -- sRGB features --

        caps->gles.hasTexSRGBDecode             = ::HasTexSRGBDecode(*api, level, clamped);
        caps->gles.hasFramebufferSRGBEnable     = ::HasFramebufferSRGBEnable(*api, *caps, level); // Don't use clamped for hasFramebufferSRGBEnable because it would break the editor rendering with OpenGL ES and -force-clamped
        caps->hasSRGBReadWrite                  = ::HasSRGBReadWrite(*api, *caps, level); // Don't use clamped for HasSRGBReadWrite because it would break the editor rendering with OpenGL ES and -force-clamped

        caps->gles.hasTextureView               = ::HasTextureView(*api, level, clamped);

        // When blitting the depth buffer while FRAMEBUFFER_SRGB is enabled, OSX occasionally puts garbage into the draw buffer.
        // verified on 10.9 with GT750M and 10.10 with GT650M
        caps->buggyDepthBlitWithSRGBEnabled     = PLATFORM_OSX;

        // OSX performs sRGB conversions when writing to linear textures if FRAMEBUFFER_SRGB is enabled
        // verified on 10.8 with a Radeon 6630M
        caps->buggySRGBWritesOnLinearTextures   = PLATFORM_OSX;

        // -- Texture features --

        caps->hasMipLevelBias                   = IsGfxLevelCore(level);
        caps->hasMipMaxLevel                    = HasMipMaxLevel(*api, level, clamped);
        caps->gles.hasMipBaseLevel              = !IsGfxLevelES2(level);
        caps->npot                              = ::HasNPOT(*api, *caps, level, clamped);

        caps->hasTextureWrapMirrorOnce          = ::HasTextureMirrorOnce(*api, level, clamped);
        caps->gles.hasTextureStorage            = ::HasTextureStorage(*api, *caps, level, clamped);
        caps->sparseTextures                    = ::HasTextureSparse(*api, level, clamped);
        if (caps->sparseTextures >= kSparseTextureTier2)
            caps->shaderCaps |= kShaderRequireSparseTex;

        caps->gles.hasTextureBuffer             = ::HasTextureBuffer(*api, level, clamped);
        caps->gles.hasAlphaLumTexStorage        = ::HasAlphaLumTextureStorage(*api, *caps, level, clamped);
        caps->gles.hasTextureSwizzle            = ::HasTextureSwizzle(*api, *caps, level, clamped);
        caps->gles.hasTextureAlpha              = !::IsGfxLevelCore(level);
        caps->gles.hasTextureRG                 = ::HasTextureRG(*api, level, clamped);

        caps->has3DTexture                      = IsGfxLevelES2(level) ? HasExtension(GLExt::kGL_OES_texture_3D) : true;

        // Untested on GL core. It's only available with ES3 with extensions, otherwise we decompress (see hasASTCSliced3D)
        caps->hasCompressedTexture3D            = IsGfxLevelES3(level, kGfxLevelES31);

        caps->supportsDepthCubeTexture          = !IsGfxLevelES2(level);
        if (::Has2DArrayTexture(*api, *caps, level, clamped))
            caps->shaderCaps |= kShaderRequire2DArray;
        if (::HasCubemapArrayTexture(*api, *caps, level, clamped))
            caps->shaderCaps |= kShaderRequireCubeArray;
        caps->gles.hasDirectTextureAccess       = ::HasDirectTextureAccess(*api, level, clamped);

        caps->copyTextureSupport                = ::HasCopyTexture(*api, level, clamped);

        caps->gles.hasSeamlessCubemapEnable     = ::HasSeamlessCubemapEnable(*api, *caps, level, clamped);
        caps->gles.hasSamplerObject             = ::HasSamplerObject(*api, level, clamped);

        caps->gles.hasInternalformat            = ::HasInternalformat(*api, level, clamped);

        caps->maxTextureBinds                   = std::min<int>(api->Get(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS), gl::kMaxTextureBindings);
        caps->gles.hasVertexShaderTexUnits      = api->Get(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS) > 0;
        caps->maxTextureSize                    = api->Get(GL_MAX_TEXTURE_SIZE);
        caps->maxCubeMapSize                    = api->Get(GL_MAX_CUBE_MAP_TEXTURE_SIZE);
        caps->maxTextureArraySlices             = HasFlag(caps->shaderCaps, kShaderRequire2DArray) ? api->Get(GL_MAX_ARRAY_TEXTURE_LAYERS) : 1;
        caps->maxAnisoLevel                     = ::GetMaxAnisoSamples(*api, level, clamped);
        caps->hasAnisoFilter                    = caps->maxAnisoLevel > 1;

        // -- Framebuffer features --

        caps->hasRenderTo3D                     = caps->has3DTexture && HasFlag(caps->shaderCaps, kShaderRequire2DArray);
        caps->gles.buggyRenderTargetDepthAndStencil = ::HasBuggyRenderTargetDepthAndStencil(*api, *caps, level, clamped);
        caps->hasNativeDepthTexture             = ::HasNativeDepthTexture(*api, *caps, level);
        caps->gles.hasPackedDepthStencil        = ::HasPackedDepthStencil(*api, level);
        caps->hasStencilInDepthTexture          = caps->hasNativeDepthTexture && !caps->gles.buggyRenderTargetDepthAndStencil && caps->gles.hasPackedDepthStencil;

        caps->gles.hasInvalidateFramebuffer     = ::HasInvalidateFramebuffer(*api, level, clamped);
        caps->gles.hasBlitFramebuffer           = ::HasBlitFramebuffer(*api, level, clamped);
        caps->gles.hasReadDrawFramebuffer       = caps->gles.hasBlitFramebuffer || HasExtension(GLExt::kGL_APPLE_framebuffer_multisample);
        caps->gles.hasMultisampleBlitScaled     = ::HasMultisampleBlitScaled(*api, level, clamped);
        caps->gles.requireDrawBufferNone        = ::RequireDrawBufferNone(*api, *caps, level);
        caps->gles.hasDrawBuffers               = ::HasDrawBuffers(*api, level);
        caps->gles.hasDepth24                   = IsGfxLevelES2(level) ? HasExtension(GLExt::kGL_OES_depth24) : true;

        caps->maxMRTs                           = ::GetMaxColorAttachments(*api, level);
        if (caps->maxMRTs >= 4)
            caps->shaderCaps |= kShaderRequireMRT4;
        if (caps->maxMRTs >= 8)
            caps->shaderCaps |= kShaderRequireMRT8;

        caps->disableSoftShadows                = ::DisableSoftShadows(*api);
        caps->hasDecentRGBMCompression          = IsGfxLevelCore(level) || PLATFORM_WEBGL;  // currently we support only desktop GL, hence dxt
        caps->gles.hasFramebufferColorRead      = ::HasFramebufferColorRead(*api, *caps, level, clamped);
        caps->gles.requireClearAlpha            = PLATFORM_WEBGL;

        // ES 3 spec says that "FRAMEBUFFER is equivalent to DRAW_FRAMEBUFFER" when passed as target to glFramebufferTexture2D etc., but the Vivante ES3 driver generates errors
        caps->gles.framebufferTargetForBindingAttachments = caps->gles.hasReadDrawFramebuffer && caps->gles.isVivanteGpu ? GL_DRAW_FRAMEBUFFER : GL_FRAMEBUFFER;

        caps->gles.hasClearDepthFloat          = ::HasClearDepthFloat(*api, *caps, level, clamped);
        caps->gles.hasClearBuffer              = ::HasClearBuffer(*api, level, clamped);

        caps->hasClearMRT = true;

        caps->maxRenderTextureSize          = api->Get(GL_MAX_RENDERBUFFER_SIZE);

//        if (UNITY_APPLE_PVR)
//        {
//            // On iOS/tvOS, we only support native shadowmaps,
//            // since everything (except iPhone4) supports it. And iPhone4 does not deserve to have realtime
//            // shadows anyway.
//            caps->hasNativeShadowMap = HasExtension(GLExt::kGL_EXT_shadow_samplers);
//            caps->hasShadows = caps->hasNativeShadowMap;
//        }
        /*else*/ if (IsGfxLevelES2(level))
        {
            // On all GLES2.0 platforms, we always do shadowmaps manually as a depth texture.
            // We don't use EXT_shadow_samplers since almost nothing supports it (outside iOS),
            // and we want to save on runtime shader variant count.
            caps->hasNativeShadowMap = false;
            caps->hasShadows = caps->hasNativeDepthTexture;
        }
        else
        {
            // On GL / GLES3+ platforms, we always do shadows via native shadow comparison sampling;
            // which is a core feature that does not need extension.
            caps->hasShadows = caps->hasNativeShadowMap = true;
        }

        // -- Buffer features --

        caps->vertexFormatCaps[kVertexFormatFloat]      = kVertexFormatCapsAnyDim;
        caps->vertexFormatCaps[kVertexFormatFloat16]    = ::Has16BitFloatVertex(*api, level, clamped) ? kVertexFormatCapsAnyDim : kVertexFormatCapsNone;
        caps->vertexFormatCaps[kVertexFormatUNorm8]     = kVertexFormatCapsAnyDim;
        caps->vertexFormatCaps[kVertexFormatSNorm8]     = kVertexFormatCapsAnyDim;
        caps->vertexFormatCaps[kVertexFormatUNorm16]    = kVertexFormatCapsAnyDim;
        caps->vertexFormatCaps[kVertexFormatSNorm16]    = kVertexFormatCapsAnyDim;
        caps->vertexFormatCaps[kVertexFormatUInt8]      = kVertexFormatCapsAnyDim;
        caps->vertexFormatCaps[kVertexFormatSInt8]      = kVertexFormatCapsAnyDim;
        caps->vertexFormatCaps[kVertexFormatUInt16]     = kVertexFormatCapsAnyDim;
        caps->vertexFormatCaps[kVertexFormatSInt16]     = kVertexFormatCapsAnyDim;
        caps->vertexFormatCaps[kVertexFormatUInt32]     = kVertexFormatCapsAnyDim;
        caps->vertexFormatCaps[kVertexFormatSInt32]     = kVertexFormatCapsAnyDim;

        // At the moment we have no way to configure vertex attribute alignment requirements.
        // Workaround: disable vertex format support to ensure alignment
        // OpenGL Spec: "Clients must align data elements consistent with the requirements of the client platform,
        // with an additional base-level requirement that an offset within a buffer to a datum comprising N basic machine units be a multiple of N."
        caps->vertexFormatCaps[kVertexFormatUInt16] &= kVertexFormatCapsEvenDim;
        caps->vertexFormatCaps[kVertexFormatSInt16] &= kVertexFormatCapsEvenDim;
        caps->vertexFormatCaps[kVertexFormatUNorm16] &= kVertexFormatCapsEvenDim;
        caps->vertexFormatCaps[kVertexFormatSNorm16] &= kVertexFormatCapsEvenDim;
        caps->vertexFormatCaps[kVertexFormatFloat16] &= kVertexFormatCapsEvenDim;
        caps->vertexFormatCaps[kVertexFormatUInt8] &= kVertexFormatCapsXYZW;
        caps->vertexFormatCaps[kVertexFormatSInt8] &= kVertexFormatCapsXYZW;
        caps->vertexFormatCaps[kVertexFormatUNorm8] &= kVertexFormatCapsXYZW;
        caps->vertexFormatCaps[kVertexFormatSNorm8] &= kVertexFormatCapsXYZW;

        caps->has32BitIndexBuffer = ::Has32BitIndexBuffer(*api, level, clamped);

        caps->bufferCaps = kGfxBufferCapsZeroStrideVertexStreams | kGfxBufferCapsCopyBuffer;

        caps->gles.hasMapbuffer                 = ::HasMapbuffer(*api, level, clamped);
        caps->gles.hasMapbufferRange            = ::HasMapbufferRange(*api, level, clamped);
        caps->gles.hasBufferCopy                = ::HasBufferCopy(*api, level, clamped);
        caps->gles.hasVertexArrayObject         = ::HasVertexArrayObject(*api, level, clamped);
        caps->gles.hasCircularBuffer            = ::HasCircularBuffer(*api, caps->gles, level, clamped);

        // On ES2 either there's no sampling with comparison, or there is with an ext, but it can be switched off. On ES3 it can be switched off. (hasNativeShadowMap)
        caps->hasRawShadowDepthSampling         = caps->hasShadows;

        // -- Shader features --
        caps->activeTier                        = GetActiveTier(level, caps->rendererString);

        caps->gles.hasES2Compatibility          = HasES2Compatibility(*api, level);
        caps->gles.hasES3Compatibility          = HasES3Compatibility(*api, level);
        caps->gles.hasES31Compatibility         = HasES31Compatibility(*api, level);
        caps->gles.hasES32Compatibility         = HasES32Compatibility(*api, level);

        caps->gles.useHighpDefaultFSPrec        = HasHighpFloatInFragmentShader(api, level);

        caps->useRGBAForPointShadows            = IsGfxLevelES(level);

        caps->gles.hasTexLodSamplers            = HasTexLodSamplers(*api, level);

        caps->gles.hasFenceSync                 = HasFenceSync(*api, level);

        caps->hasRenderTargetArrayIndexFromAnyShader = HasExtension(GLExt::kGL_NV_viewport_array2) || HasExtension(GLExt::kGL_AMD_vertex_shader_layer) || HasExtension(GLExt::kGL_ARB_shader_viewport_layer_array);
        if (caps->hasRenderTargetArrayIndexFromAnyShader)
            caps->shaderCaps |= kShaderRequireSetRTArrayIndexFromAnyShader;

        if (IsGfxLevelES3(level) && !clamped)
        {
            caps->gles.hasMultiview = HasExtension(GLExt::kGL_OVR_multiview);
            caps->hasMultiSampleTexture2DArrayAutoResolve = HasExtension(GLExt::kGL_OVR_multiview_multisampled_render_to_texture);
        }

        if (IsGfxLevelCore(level, kGfxLevelCore32) || IsGfxLevelES3(level, kGfxLevelES31AEP) || caps->hasMultiSampleTexture2DArrayAutoResolve)
        {
            caps->hasMultiSampleTexture2DArray = !PLATFORM_OSX;
        }

#if PLATFORM_ANDROID
        caps->gles.haspolygonOffsetBug = (caps->gles.driverGLESVersion == 2 && (caps->gles.isMaliGpu || caps->gles.isPvrGpu));

#if DEBUGMMODE
        if (caps->gles.haspolygonOffsetBug)
            printf_console("Activating PolygonOffset compensation\n");
#endif

        // Cases 859561 and 864374, doesn't repro on Android 8.0 and higher
        caps->gles.buggyInvalidateFrameBuffer = caps->gles.isAdrenoGpu && android::systeminfo::ApiLevel() < android::apiOreo;

        // TODO: do we still need this workaround with min API set to 4.4 KitKat? There are still Adreno 2xx devices on Android 4.4.
        // Problem exists only on older Adreno drivers without ES3.0 support, which was added in Android 4.3
        caps->gles.buggyTextureUploadSynchronization = caps->gles.isAdrenoGpu && caps->gles.isES2Gpu;

        // Disable uniform buffers on Adreno 3xx's. ES 3.1 GPUs should be fine.
        caps->gles.buggyUniformBuffers = caps->gles.isAdrenoGpu && caps->driverVersionString.find("OpenGL ES 3.0") != std::string::npos;

        caps->gles.buggyNearPlaneTrianglesClipping = caps->gles.isVivanteGpu &&
            ((caps->rendererString.find("GC1000") != std::string::npos)
                || (caps->rendererString.find("GC2000") != std::string::npos));
        caps->gles.hasGLSLTransposeWithVersion100 = caps->gles.isIntelGpu && (caps->rendererString.find("BayTrail") != std::string::npos);

        caps->gles.buggyTexCubeLodGrad = caps->gles.isPvrGpu;
        caps->gles.buggyDisableColorWrite = caps->gles.isPvrGpu && caps->rendererString.find("SGX") != std::string::npos;
        caps->gles.buggyFloatRenderTarget = caps->gles.isAdrenoGpu && IsGfxLevelES2(level) && (isAdreno4 || isAdreno5 || isAdreno6);

        caps->gles.buggyTexStorageETC = IsGfxLevelES3(level) && caps->gles.isAdrenoGpu && android::systeminfo::ApiLevel() < android::apiLollipop;
        caps->gles.buggyTexStorage3DASTC = caps->gles.isAdrenoGpu;

        // The workaround (original cases 777617, 735299, 776827, 784146) controlled by UNITY_ADRENO_ES3
        // breaks shaders using clip() on many Adreno 4xx drivers (case 966038) and on many Adreno 3xx devices on 5.1 or newer (cases 808817, 805086, 801150)
        // Based on the information in bugreports enable the workaround only on Adreno3x devices and on Android versions older than 5.1
        caps->gles.requiresAdrenoShaderWorkarounds = isAdreno3 && (android::systeminfo::ApiLevel() < android::apiLollipopMR1);

        // So far we only saw this on Google Pixel with Android 9.0 (case 1071297)
        caps->gles.buggyReadbackFromAutoResolveFramebuffer = caps->gles.isAdrenoGpu && android::systeminfo::ApiLevel() == android::apiPie && ::strcasecmp(android::systeminfo::Manufacturer(), "Google") == 0;
#endif

        caps->gles.buggyTexStorageDXT = caps->gles.isVivanteGpu;

        if (PLATFORM_ANDROID)
            caps->gles.buggyDetachShader = (caps->rendererString.find("Tegra 3") != std::string::npos);
        else if (PLATFORM_WEBGL)
            caps->gles.buggyDetachShader = (caps->rendererString == "WebKit WebGL"); // glDetachShader causes problems on Safari
        else
            caps->gles.buggyDetachShader = false;

        caps->gles.maxFlexibleArrayBatchSize = caps->gles.isAdrenoGpu ? 128 : 0xffffffff;
        caps->gles.defaultFlexibleArrayBatchSize = caps->gles.isAdrenoGpu ? 128 : 2;

//        // both gles2 and gles3 have issue with non-full mipchains (up to some ios9.x)
//        // though we can do workaround ONLY if there is texture_max_level
//        if (UNITY_APPLE_PVR)
//            caps->gles.buggyTextureStorageWithNonFullMipChain = HasMipMaxLevel(*api, level, clamped);

        // Fill the shader language table
        caps->gles.supportedShaderTagsCount = 0;
        if (IsGfxLevelCore(GetGraphicsCaps().gles.featureLevel))
        {
            if (IsGfxLevelCore(GetGraphicsCaps().gles.featureLevel, kGfxLevelCore32))
                caps->gles.supportedShaderTags[caps->gles.supportedShaderTagsCount++] = kShaderGpuProgramGLCore32;
            if (IsGfxLevelCore(GetGraphicsCaps().gles.featureLevel, kGfxLevelCore41))
                caps->gles.supportedShaderTags[caps->gles.supportedShaderTagsCount++] = kShaderGpuProgramGLCore41;
            if (IsGfxLevelCore(GetGraphicsCaps().gles.featureLevel, kGfxLevelCore43))
                caps->gles.supportedShaderTags[caps->gles.supportedShaderTagsCount++] = kShaderGpuProgramGLCore43;
        }
        else if (IsGfxLevelES2(GetGraphicsCaps().gles.featureLevel))
        {
            caps->gles.supportedShaderTags[caps->gles.supportedShaderTagsCount++] = kShaderGpuProgramGLES;
        }
        else if (IsGfxLevelES3(GetGraphicsCaps().gles.featureLevel))
        {
            caps->gles.supportedShaderTags[caps->gles.supportedShaderTagsCount++] = kShaderGpuProgramGLES3;
            if (IsGfxLevelES3(GetGraphicsCaps().gles.featureLevel, kGfxLevelES31))
                caps->gles.supportedShaderTags[caps->gles.supportedShaderTagsCount++] = kShaderGpuProgramGLES31;
            if (IsGfxLevelES3(GetGraphicsCaps().gles.featureLevel, kGfxLevelES31AEP))
                caps->gles.supportedShaderTags[caps->gles.supportedShaderTagsCount++] = kShaderGpuProgramGLES31AEP;
        }

        caps->gles.hasBinaryShader = ::HasBinaryProgram(*api, *caps, level, clamped);
        caps->gles.hasBinaryShaderRetrievableHint = ::HasBinaryProgramRetrievableHint(*api, *caps, level, clamped);
        if (hasGeometry)
            caps->shaderCaps |= kShaderRequireGeometry;
        caps->gles.hasProgramPointSizeEnable            = ::HasProgramPointSizeEnable(*api, level, clamped);
        if (hasTessellation)
            caps->shaderCaps |= kShaderRequireTessellation | kShaderRequireTessHW;
        caps->gles.maxTransformFeedbackBufferBindings   = ::GetTransformFeedbackBufferBindings(*api, level, clamped);

        if (PLATFORM_ANDROID && caps->gles.isAdrenoGpu)
            caps->gles.minBufferSizeBytes = 4096;
        else
            caps->gles.minBufferSizeBytes = 64; // unknown, so pick some small value

        caps->gles.maxVertexUniforms                    = ::GetMaxVertexUniforms(*api, level);

        caps->gles.hasUniformBuffer                     = (!caps->gles.buggyUniformBuffers) && ::HasUniformBuffer(*api, level, clamped);
        if (caps->gles.hasUniformBuffer)
        {
            caps->gles.maxUniformBlockSize              = api->Get(GL_MAX_UNIFORM_BLOCK_SIZE);
            caps->gles.maxUniformBufferBindings         = std::min<int>(api->Get(GL_MAX_UNIFORM_BUFFER_BINDINGS), gl::kMaxUniformBufferBindings);

            caps->minConstantBufferOffsetAlignment      = api->Get(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT);
            caps->hasSetConstantBuffer = ::HasSRPBatcherSupport(*api, level);

            if (caps->gles.isMaliGpu)
                caps->gles.mapWholeUBOForWritingFlags = GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT;   // Mali drivers go panic mode when both INVALIDATE and UNSYNCHRONIZED are set annd does a full shadow copy
            else if (caps->gles.isAdrenoGpu)
                caps->gles.mapWholeUBOForWritingFlags = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;    // Adreno ignores GL_MAP_UNSYNCHRONIZED_BIT and issues a performance warning
            else
                caps->gles.mapWholeUBOForWritingFlags = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT;
        }

        caps->maxConstantBufferSize                     = static_cast<UInt32>(caps->gles.maxUniformBlockSize);

        // Our instancing implementation requires working uniform buffers
        caps->shaderCaps = SetOrClearFlags(caps->shaderCaps, kShaderRequireInstancing, ::HasInstancedDraw(*api, level, clamped) && caps->gles.hasUniformBuffer);

        caps->gles.maxAttributes                        = std::min<int>(api->Get(GL_MAX_VERTEX_ATTRIBS), gl::kMaxVertexAttrCount);

#if PLATFORM_ANDROID
        if (::HasComputeShader(*api, level, clamped))
        {
            caps->gpuSkinningCaps = kGPUSkinningSupported | kGPUSkinningAllBoneCountCompute | kGPUSkinningHasOnlyComputeSkinning | kGPUSkinningSupportsBlendShapes;
            caps->bufferCaps |= kGfxBufferCapsComputeBuffersAsVertexInput;
        }
        else
            caps->gpuSkinningCaps = kGPUSkinningCapsNone;
#endif

        caps->gles.useActualBufferTargetForUploads = PLATFORM_WEBGL || caps->gles.isMaliGpu;

        // -- Blend --

        caps->hasSeparateMRTBlend               = ::HasBlendSeparateMRT(*api, level, clamped);
        caps->hasBlendMinMax                    = IsGfxLevelES2(level) ? HasExtension(GLExt::kGL_EXT_blend_minmax) : true;
        caps->hasBlendAdvanced                  = ::HasBlendAdvanced(*api, level, clamped);
        caps->hasBlendAdvancedCoherent          = ::HasBlendAdvancedCoherent(*api, level, clamped);

        // -- Line --

        caps->gles.hasWireframe                 = IsGfxLevelCore(level);

        // -- Timer queries, update later in Init --
        caps->hasTimerQuery                     = ::HasTimerQuery(*api, level, clamped);
        caps->gles.hasDisjointTimerQuery        = ::HasDisjointTimerQuery(*api, level, clamped);


        // -- Clip --
        caps->gles.hasClipDistance              = ::HasClipDistance(*api, level, clamped);
        caps->gles.hasDepthClamp                = ::HasDepthClamp(*api, level, clamped);
        caps->conservativeRasterSupport         = ::HasConservativeRaster();

        // -- Multisampled textures
        caps->gles.hasSampler2dMS               = ::HasSampler2dMS(*api, level);
        caps->hasTexture2DMS                    = caps->gles.hasSampler2dMS;
        if (caps->hasTexture2DMS)
            caps->shaderCaps |= kShaderRequireMSAATex;
        caps->gles.hasTexStorageMultisample     = ::HasTexStorageMultisample(*api, level);

        caps->gles.hasASTCDecodeMode            = ::HasASTCDecodeMode(*api, level, clamped);
        caps->gles.hasASTCDecodeModeRGB9E5      = ::HasASTCDecodeModeRGB9E5(*api, level, clamped);
        caps->gles.hasASTCSliced3D              = ::HasASTCSliced3D(*api, level, clamped);

        // We use BufferManager fence sync wherever available, and just current + 4 frames elsewhere.
        caps->supportsGPUFence                  = true;

        // -- Initialize supported texture formats --

        InitFormatCaps(*api, caps, caps->gles.featureLevel, caps->gles.featureClamped);
        InitDefaultFormat(*api, caps, caps->gles.featureLevel);

        if (1)
        {
            if (caps->IsFormatSupported(kFormatR16G16B16A16_SFloat, kUsageSample))
                caps->attenuationFormat = kFormatR16G16B16A16_SFloat;
        }
        else
        {
            if (caps->IsFormatSupported(kFormatR16_UNorm, kUsageSample))
                caps->attenuationFormat = kFormatR16_UNorm;
            else if (caps->IsFormatSupported(kFormatR16_SFloat, kUsageSample))
                caps->attenuationFormat = kFormatR16_SFloat;
            else
                caps->attenuationFormat = kFormatR8_UNorm;
        }

        caps->srpBatcherSupported = ::HasSRPBatcherSupport(*api, level);
        caps->hasNoSeparateFragmentShaderStage = true;

        if (PLATFORM_ANDROID && caps->gles.isMaliGpu)
        {
            if (caps->rendererString == "Mali-G78" || caps->rendererString == "Mali-G77" || caps->rendererString == "Mali-G68" || caps->rendererString == "Mali-G57")
            {
                const char prefix[] = "OpenGL ES 3.2 v1.r";
                const size_t pos = caps->driverVersionString.find(prefix);
                if (pos != std::string::npos)
                {
                    std::string versionStr(caps->driverVersionString.substr(pos + sizeof(prefix) - 1));
                    const int version = StringToInt(versionStr);
                    if (version < 27)
                        caps->gles.buggyCopyBufferDependencyHandling = true;
                }
            }
        }

//        if (PLATFORM_ANDROID && caps->gles.isMaliGpu)
//        {
//            if (BeginsWith(caps->rendererString, "Mali-G7") || BeginsWith(caps->rendererString, "Mali-5") || BeginsWith(caps->rendererString, "Mali-G3"))
//            {
//                const char prefix[] = "OpenGL ES 3.2 v1.r";
//                const size_t pos = caps->driverVersionString.find(prefix);
//                if (pos != std::string::npos)
//                {
//                    const int version = StringToInt(caps->driverVersionString.substr(pos + sizeof(prefix) - 1));
//                    if (version < 19)
//                        caps->gles.buggyMultiPassAutoresolve = true;
//                }
//            }
//        }

#if PLATFORM_ANDROID
        if ((isAdreno4 || isAdreno5) && (caps->gpuSkinningCaps & kGPUSkinningSupportsBlendShapes))
        {
            // Blend shapes with compute skinning don't work properly on older Adreno drivers
            // glFlush works as a workaround but it destroys any performance gains

            // known bad version is 145 (e.g. Pixel, Android 7.1)
            // known good version is 225 (e.g. Pixel, Android 8.0)
            // But there are some devices with 145 driver and Android 8
            // case 1253349, 1260887

            bool disableComputeblendShapes = false;
            if (android::systeminfo::ApiLevel() < android::apiOreo)
            {
                disableComputeblendShapes = true;
            }
            else
            {
                if (adrenoDriverVersionES3x > 0 && adrenoDriverVersionES3x < 225)
                    disableComputeblendShapes = true;
            }

            if (disableComputeblendShapes)
                caps->gpuSkinningCaps &= ~kGPUSkinningSupportsBlendShapes;
        }
#endif

#if PLATFORM_ANDROID
        if (isAdreno4 || isAdreno5 || isAdreno6)
        {
            if (adrenoDriverVersionES3x > 0 && adrenoDriverVersionES3x < 384)
                caps->gles.buggyDrawAfterDrawIndirect = true;
        }
#endif

#if GFX_SUPPORTS_SINGLE_PASS_STEREO
        if (IsGfxLevelCore(level))
        {
            caps->singlePassStereo = kSinglePassStereoSideBySide;
        }
        else if (caps->gles.hasMultiview)
        {
            caps->singlePassStereo = kSinglePassStereoMultiview;
        }
        else if (caps->hasRenderTargetArrayIndexFromAnyShader)
        {
            caps->singlePassStereo = kSinglePassStereoInstancing;
        }
        else
        {
            caps->singlePassStereo = kSinglePassStereoNone;
        }
#endif
        caps->canCreateUninitializedTextures = false; // this is currently disabled but AFAIK we could get it to work if needed

        caps->gles.buggyShaderReflectionSSBO = caps->gles.isAdrenoGpu;
    }
}//namespace gles
