#include "GfxDevice/opengles/GfxGetProcAddressGLES.h"
#include "Logging/LogAssert.h"
#include <cstddef>

#if ENABLE_EGL
    #include "Runtime/GfxDevice/egl/IncludesEGL.h"
#endif

#if UNITY_APPLE || PLATFORM_ANDROID
    #include <dlfcn.h>
#endif

#if PLATFORM_WEBGL
    #include <emscripten.h>
#endif

#if PLATFORM_WIN
#include <windows.h>
#endif

#if PLATFORM_OSX
#include "External/baselib/builds/Include/PreExternalInclude.h"
#import <mach-o/dyld.h>
#include "External/baselib/builds/Include/PostExternalInclude.h"
#endif

#if PLATFORM_LINUX
#if ENABLE_SDL2_WINDOWING
#include "PlatformDependent/Linux/SDLWrapper.h"
#else
#include "Editor/Platform/Linux/GLXExtensions.h"
#endif
#endif

#if FORCE_DEBUG_BUILD_WEBGL
#   undef PLATFORM_WEBGL
#   define PLATFORM_WEBGL 1
#endif//FORCE_DEBUG_BUILD_WEBGL

namespace gles
{
    void *GetProcAddress_core(const char *name)
    {
        void *proc = NULL;
        // TODO Add similar workarounds for other platforms that need it
#if PLATFORM_ANDROID
        static void* selfHandle = NULL;
#if PLATFORM_ANDROID && UNITY_DEVELOPER_BUILD
        if (!selfHandle)
        {
            selfHandle = dlopen("libMGD.so", RTLD_LOCAL | RTLD_NOW);
        }
#endif
        if (!selfHandle)
            selfHandle = dlopen("libGLESv2.so", RTLD_LOCAL | RTLD_NOW);
        proc = dlsym(selfHandle, name);
#else
        return GetProcAddress(name);
#endif

//#if !UNITY_RELEASE
//        if (!proc)
//            printf_console("Warning: unable to load %s\n", name);
//#endif
        return proc;
    }

    void* GetProcAddress(const char* name)
    {
        void* proc = NULL;

#if ENABLE_EGL
        proc = (void*)eglGetProcAddress(name);
#elif PLATFORM_WIN
        proc = (void*)wglGetProcAddress(name);
        if (!proc)
            proc = (void*)GetProcAddress(GetModuleHandle("OpenGL32.dll"), name);

#elif PLATFORM_LINUX
        proc = (void*)GetNativeGLProcAddress(name);

#elif UNITY_APPLE
        // on apple platforms we link to framework, so symbols are already resolved
        static void* selfHandle = dlopen(0, RTLD_LOCAL | RTLD_LAZY);
        proc = dlsym(selfHandle, name);
#elif PLATFORM_WEBGL
        // GetProcAddress on webgl will incur in performance and build size penalties
        EM_ASM({throw new Error("Internal Unity error: gles::GetProcAddress(\"" + Pointer_stringify($0) + "\") was called but gles::GetProcAddress() is not implemented on Unity WebGL. Please report a bug.")}, name);
#else
        CompileTimeAssert(0, "OPENGL ERROR: Unsupported platform");
#endif

//#if !UNITY_RELEASE
//        if (!proc)
//            printf_console("Warning: unable to load %s\n", name);
//#endif

        return proc;
    }
}//namespace gles
