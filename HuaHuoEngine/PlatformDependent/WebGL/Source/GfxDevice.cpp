#include "Runtime/GfxDevice/GfxDeviceTypes.h"
#include "PlatformDependent/WebGL/Source/JSBridge.h"

bool PlatformIsGfxDeviceRendererSupported(GfxDeviceRenderer renderer)
{
    // need to check if api is supported by the browser
    switch (renderer)
    {
        case kGfxRendererOpenGLES3x:
            return JS_SystemInfo_HasWebGL() >= 2;
        case kGfxRendererOpenGLES20:
            return JS_SystemInfo_HasWebGL() >= 1;
        default:
            return false;
    }
}
