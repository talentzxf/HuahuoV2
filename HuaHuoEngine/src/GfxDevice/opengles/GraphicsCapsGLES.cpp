//
// Created by VincentZhang on 5/24/2022.
//

#include "GraphicsCapsGLES.h"
#include "ApiGLES.h"
#include <vector>
#include <string>

GraphicsCapsGLES* g_GraphicsCapsGLES = 0;

namespace gles
{
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

        caps->gles.featureClamped = HasARGV("force-clamped");
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
        std::vector<core::string> parts;
        //int no = SplitString(parts, caps->driverVersionString, " ", 4);
        Split(caps->driverVersionString, ' ', parts, 4);
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

        caps->gles.isVideoCoreGpu = (caps->rendererString.find("VideoCore") != core::string::npos);
        // OpenGL is going away from Apple platforms, but treat it as PowerVR for the remaining time
        caps->gles.isPvrGpu = (caps->rendererString.find("PowerVR") != core::string::npos || caps->rendererString.find("Apple") != core::string::npos);
        caps->gles.isMaliGpu = (caps->rendererString.find("Mali") != core::string::npos);
        caps->gles.isAdrenoGpu = (caps->rendererString.find("Adreno") != core::string::npos);
        caps->gles.isTegraGpu = (caps->rendererString.find("Tegra") != core::string::npos);
        caps->gles.isIntelGpu = (caps->rendererString.find("Intel") != core::string::npos);
        caps->gles.isNvidiaGpu = (caps->rendererString.find("NVIDIA") != core::string::npos);
        caps->gles.isAMDGpu = (caps->rendererString.find("AMD") != core::string::npos) || (caps->rendererString.find("ATI") != core::string::npos);
        caps->gles.isAMDVegaGpu = ::isAMDVegaGpu(caps->rendererString);
        caps->gles.isVivanteGpu = (caps->vendorString.find("Vivante") != core::string::npos);
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
            isAdreno3 = caps->rendererString.find("Adreno (TM) 3") != core::string::npos;
            isAdreno4 = caps->rendererString.find("Adreno (TM) 4") != core::string::npos;
            isAdreno5 = caps->rendererString.find("Adreno (TM) 5") != core::string::npos;
            isAdreno6 = caps->rendererString.find("Adreno (TM) 6") != core::string::npos;
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
            const bool isMaliBifrost = caps->rendererString.find("Mali-G") != core::string::npos;
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

        const core::string& extensionString = GetExtensionsString(allExtensions);

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

        if (UNITY_APPLE_PVR)
        {
            // On iOS/tvOS, we only support native shadowmaps,
            // since everything (except iPhone4) supports it. And iPhone4 does not deserve to have realtime
            // shadows anyway.
            caps->hasNativeShadowMap = HasExtension(GLExt::kGL_EXT_shadow_samplers);
            caps->hasShadows = caps->hasNativeShadowMap;
        }
        else if (IsGfxLevelES2(level))
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
        caps->gles.buggyUniformBuffers = caps->gles.isAdrenoGpu && caps->driverVersionString.find("OpenGL ES 3.0") != core::string::npos;

        caps->gles.buggyNearPlaneTrianglesClipping = caps->gles.isVivanteGpu &&
            ((caps->rendererString.find("GC1000") != core::string::npos)
                || (caps->rendererString.find("GC2000") != core::string::npos));
        caps->gles.hasGLSLTransposeWithVersion100 = caps->gles.isIntelGpu && (caps->rendererString.find("BayTrail") != core::string::npos);

        caps->gles.buggyTexCubeLodGrad = caps->gles.isPvrGpu;
        caps->gles.buggyDisableColorWrite = caps->gles.isPvrGpu && caps->rendererString.find("SGX") != core::string::npos;
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
            caps->gles.buggyDetachShader = (caps->rendererString.find("Tegra 3") != core::string::npos);
        else if (PLATFORM_WEBGL)
            caps->gles.buggyDetachShader = (caps->rendererString == "WebKit WebGL"); // glDetachShader causes problems on Safari
        else
            caps->gles.buggyDetachShader = false;

        caps->gles.maxFlexibleArrayBatchSize = caps->gles.isAdrenoGpu ? 128 : 0xffffffff;
        caps->gles.defaultFlexibleArrayBatchSize = caps->gles.isAdrenoGpu ? 128 : 2;

        // both gles2 and gles3 have issue with non-full mipchains (up to some ios9.x)
        // though we can do workaround ONLY if there is texture_max_level
        if (UNITY_APPLE_PVR)
            caps->gles.buggyTextureStorageWithNonFullMipChain = HasMipMaxLevel(*api, level, clamped);

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

        if (UNITY_WEBGL)
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
                if (pos != core::string::npos)
                {
                    const int version = StringToInt(caps->driverVersionString.substr(pos + sizeof(prefix) - 1));
                    if (version < 27)
                        caps->gles.buggyCopyBufferDependencyHandling = true;
                }
            }
        }

        if (PLATFORM_ANDROID && caps->gles.isMaliGpu)
        {
            if (BeginsWith(caps->rendererString, "Mali-G7") || BeginsWith(caps->rendererString, "Mali-5") || BeginsWith(caps->rendererString, "Mali-G3"))
            {
                const char prefix[] = "OpenGL ES 3.2 v1.r";
                const size_t pos = caps->driverVersionString.find(prefix);
                if (pos != core::string::npos)
                {
                    const int version = StringToInt(caps->driverVersionString.substr(pos + sizeof(prefix) - 1));
                    if (version < 19)
                        caps->gles.buggyMultiPassAutoresolve = true;
                }
            }
        }

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
}