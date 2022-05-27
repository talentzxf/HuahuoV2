//
// Created by VincentZhang on 5/25/2022.
//


#include "GfxDevice/GfxDevice.h"
#include "GfxDeviceGLES.h"

//#if PLATFORM_WIN
#include <wingdi.h>
//#endif

namespace gl
{
    ContextHandle GetCurrentContext()
    {
        ContextHandle currentContextHandle = gl::ContextHandle::Invalid();

//#       if PLATFORM_WIN
        void* currentContext = reinterpret_cast<void*>(wglGetCurrentContext());
//#       elif PLATFORM_LINUX
//        void* currentContext = GetCurrentNativeGLContext();
//#       elif PLATFORM_ANDROID || PLATFORM_LUMIN
//        void* currentContext = reinterpret_cast<void*>(eglGetCurrentContext());
//#       elif PLATFORM_OSX
//        void* currentContext = reinterpret_cast<void*>(CGLGetCurrentContext());
//#       elif UNITY_APPLE_PVR
//        void* IOSGetCurrentContext(void);
//        void* currentContext = IOSGetCurrentContext();
//#       elif PLATFORM_WEBGL
//        void* WebGLGetCurrentContext(void);
//        void* currentContext = WebGLGetCurrentContext();
//#       else
//#           error "OPENGL ERROR: Unsupported platform"
//#       endif

        if (currentContext != NULL)
            currentContextHandle = ContextHandle(currentContext);

        DebugAssert(currentContextHandle != gl::ContextHandle::Invalid());
        return currentContextHandle;
    }
}//namespace gl

GfxDevice* CreateGLESGfxDevice(GfxDeviceRenderer renderer)
{
    GfxDeviceLevelGL requestedLevel = kGfxLevelUninitialized;
    switch (renderer)
    {
        case kGfxRendererOpenGLES20:
            requestedLevel = kGfxLevelES2;
            break;
        case kGfxRendererOpenGLES3x:
            requestedLevel = kGfxLevelESLast;
            break;
        case kGfxRendererOpenGLCore:
            requestedLevel = kGfxLevelCoreLast;
            break;
        default:
            Assert(0);
            break;
    }

    GfxDeviceGLES* device = HUAHUO_NEW_AS_ROOT(GfxDeviceGLES, kMemGfxDevice, "Rendering", "GfxDeviceGLES") ();
    if (device->Init(requestedLevel))
        return device;

    DebugAssertMsg(device, "ERROR: Failed to initialized a GfxDeviceGLES instance");
    HUAHUO_DELETE(device, kMemGfxDevice);
    return NULL;
}