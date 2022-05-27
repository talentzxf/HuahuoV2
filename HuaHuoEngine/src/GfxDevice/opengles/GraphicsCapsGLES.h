//
// Created by VincentZhang on 5/24/2022.
//

#ifndef HUAHUOENGINE_GRAPHICSCAPSGLES_H
#define HUAHUOENGINE_GRAPHICSCAPSGLES_H

#include "ApiTypeGLES.h"
#include "ApiEnumGLES.h"
#include "GfxDevice/GfxDeviceTypes.h"
#include "Utilities/fixed_array.h"

enum
{
    kMaxShaderTags = 10
};

struct GraphicsCapsGLES {
public:
    GraphicsCapsGLES();

    int     maxAASamples;
    bool featureClamped;
    bool    isVideoCoreGpu;
    bool    isPvrGpu;
    bool    isMaliGpu;
    bool    isAdrenoGpu;
    bool    isTegraGpu;
    bool    isIntelGpu;
    bool    isNvidiaGpu;
    bool    isAMDGpu;
    bool    isAMDVegaGpu;
    bool    isVivanteGpu;
    bool    isES2Gpu;       // A GPU that supports only OpenGL ES 2.0
    int     maxAttributes;

    //      If set, uses actual buffer target for data uploads instead of COPY_WRITE_BUFFER etc.
    bool    useActualBufferTargetForUploads;

    bool    hasWireframe; // GL

    bool    hasDisjointTimerQuery; // GL_EXT_disjoint_timer_query

    int     maxTransformFeedbackBufferBindings;

    size_t  minBufferSizeBytes; // minimum size the driver allocates for buffers (glBufferData)
    int     maxVertexUniforms;

    // Adreno 3xx's are super slow when using uniform buffers. Also they crash (at least pre-lollipop) when trying to query
    // the bound uniform buffers with glGetIntegeri_v. If this is set, no checks will be made.
    bool    buggyUniformBuffers;

    int     maxUniformBlockSize;

    int     maxUniformBufferBindings;
    int     hasVertexShaderTexUnits;

    bool    hasClipDistance;                    // has gl_ClipDistance in shader. all desktops do, ES requires GL_EXT_clip_cull_distances

    bool    hasDepthClamp;                      // GL_ARB_depth_clamp

    bool    hasSampler2dMS;                     // GL1.5 / ES3.1

    bool    hasTexStorageMultisample;           // GL 4.3 / ARB_texture_storage_multisample

    bool    hasASTCDecodeMode;                  // GL_EXT_texture_compression_astc_decode_mode
    bool    hasASTCDecodeModeRGB9E5;            // GL_EXT_texture_compression_astc_decode_mode_rgb9e5
    bool    hasASTCSliced3D;                    // GL_KHR_texture_compression_astc_sliced_3d or GL_KHR_texture_compression_astc_hdr (implies sliced 3d)

    // Different drivers like different sets of flags for mapping an entire UBO for writing
    GLenum mapWholeUBOForWritingFlags;

    // Adreno 2xx: buggy depth texture & stencil attachment at once.
    bool    buggyRenderTargetDepthAndStencil;

    bool    hasMultisampleBlitScaled;       // EXT_framebuffer_multisample_blit_scaled (GL)
    bool    hasInvalidateFramebuffer;       // GL 4.2 / GL_ARB_invalidate_subdata  / ES 3.0 / GL_EXT_discard_framebuffer
    bool    hasDrawBuffers;                 // GL 2.0 / ES3 / ES2 / GL_NV_draw_buffers / GL_EXT_draw_buffers

    // glBlitFramebuffer support
    bool    hasBlitFramebuffer; // GL 3.0 / GL_ARB_framebuffer_object / GL_NV_framebuffer_blit / ES 3.0

    // OpenGL desktop require to call glDrawBuffer(GL_NONE) on depth only framebuffer. This retriction was dropped in OpenGL 4.1 and ES2_compatibility
    bool    requireDrawBufferNone;

    // Requires ES2_compatibility on desktop. Also Intel drivers (May 2015) have a bug and generates an invalid operation error on glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_FORMAT, ...)
    bool    hasFramebufferColorRead;

    // The platform blend the framebuffer so we need to clear the Alpha channel that we don't typically keep clean.
    bool    requireClearAlpha;

    // Should be passed as target to glFramebufferTexture2D, glFramebufferRenderbuffer etc. when binding the framebuffer using ApiGLES::BindFramebuffer and kDrawFramebuffer.
    GLenum  framebufferTargetForBindingAttachments;

    bool    hasClearDepthFloat; // ES has glClearDepthf, GL < 4.1 has glClearDepth

    bool    hasClearBuffer; // glClearBuffer, in ES 3.0 and all GL core, not in WebGL 1.0

    bool    hasDebug;           // KHR_debug: Debug output + Debug Marker + Debug Label + Notification + Debug enable

    // GL_DRAW_FRAMEBUFFER and GL_READ_FRAMEBUFFER support (glBlitFramebuffer support or GL_APPLE_framebuffer_multisample)
    bool    hasReadDrawFramebuffer;

    // macOS High Sierra + Intel Iris 6100 has troubles with MSAA when certain features are switched on
    bool    buggyMSAA;

    int     driverGLESVersion;
    GfxDeviceLevelGL featureLevel;

    bool    hasMapbuffer;                       // GL 1.5 / OES_mapbuffer / ES 3.0 droped it. We typically try not to use it.
    bool    hasMapbufferRange;                  // GL 3.0 / ARB_map_buffer_range / ES 3.0 / EXT_map_buffer_range
    bool    hasBufferCopy;                      // GL 3.1 / ARB_copy_buffer /  ES 3.0

    bool    hasCircularBuffer;                  // Unity implementation of circular buffers. Depends on GL feature (mapBufferRange). Only enabled on drivers where it's actually fast

    bool    hasVertexArrayObject;               // GL 3.0 / ARB_vertex_array_object / ES 3.0 / OES_vertex_array_object
    bool    hasSeparateShaderObject;            // GL 4.1 / GL_ARB_separate_shader_objects / ES 3.1

    bool    hasES2Compatibility;            // To run GLSL ES 100 shader on desktop
    bool    hasES3Compatibility;            // To run GLSL ES 300 shader on desktop
    bool    hasES31Compatibility;           // To run GLSL ES 310 shader on desktop
    bool    hasES32Compatibility;           // To run GLSL ES 320 shader on desktop

    // True if the driver supports highp in fragment shader
    bool    useHighpDefaultFSPrec;

    // True if the device has texture sampler with explicit LOD.
    bool    hasTexLodSamplers;

    bool    hasFenceSync;                       // GL 3.2, ES3.0 onwards / ARB_sync

    bool    hasMultiview;                       // GL_OVR_multiview

    // Nvidia tegra GPU on Android < 6.0 fails to get information about active uniform variables
    // after shader object detachement glGetProgramiv returns correct GL_ACTIVE_UNIFORMS count,
    // but glGetActiveUniform always results in GL_INVALID_VALUE: Numeric argument out of range.
    // glDetachShader breaks shader reflection on Tegra 3
    bool    buggyDetachShader;

    // Adreno driver has problems with regular draws after draw indirect (case 1271258)
    bool    buggyDrawAfterDrawIndirect;

    // Adreno driver that Google ships with Android 9.0 for Pixel phones doesn't support glReadPixels from auto-resolve FBO (MSAA backbuffer is fine though)
    bool    buggyReadbackFromAutoResolveFramebuffer;

    // Mali driver may deadlock when we use a CopyBufferSubData on a buffer that was previously written to from compute without forcing flush
    bool    buggyCopyBufferDependencyHandling;

    // Mali driver generates artifacts with multiple passes to a floating point MSAA auto-resolve rendertarget without clear between the passes (case 1246823)
    bool    buggyMultiPassAutoresolve;

    // Adreno driver reports some invalid SSBO bindings (case 1251305)
    bool    buggyShaderReflectionSSBO;

    // Adreno GPUs behave badly in some cases when the batch is over the limit (case 1053324 and duplicates)
    // (this is a bug in the shader compiler in the driver, it doesn't like array sizes that use a define instead of a constant)
    // maxFlexibleArrayBatchSize        speicifies how many instances can there be in total
    // defaultFlexibleArrayBatchSize    is what is being patched to the shader by default (having 2 as it was initially
    //                                  doesn't work on Adreno as well, we need to set it to the same value as maxFlexibleArrayBatchSize)
    UInt32  maxFlexibleArrayBatchSize;
    UInt32  defaultFlexibleArrayBatchSize;

    fixed_array<ShaderGpuProgramType, kMaxShaderTags> supportedShaderTags;
    int     supportedShaderTagsCount;
    // -- sRGB --

    // gl from GL_EXT_texture_sRGB_decode (Todo: could be done with texture views too from 4.3 or GL_ARB_texture_view
    bool    hasTexSRGBDecode; // Read sRGB textures directly without decoding: http://www.opengl.org/registry/specs/EXT/texture_sRGB_decode.txt
    bool    hasFramebufferSRGBEnable;       // GL_ARB_framebuffer_sRGB || GL_EXT_sRGB_write_control

    bool    hasTextureView; // 4.3 / GL_ARB_texture_view / GL_EXT_texture_view / GL_OES_texture_view

    bool    hasMipBaseLevel;                    // GL 3.2 / ES 3.0: GL_TEXTURE_BASE_LEVEL is supported
    bool    hasTextureSwizzle;                  // ES3 / GL3.3 / ARB_texture_swizzle / Not available on WebGL
    bool    hasTextureStorage;                  // GL 4.2 / ES 3.0 / GL_ARB_texture_storage / GL_EXT_texture_storage
    bool    hasTextureRG;                       // Not in GLES without EXT_texture_rg
    bool    hasTextureBuffer;                   // GL 4.3 / GL_EXT_texture_buffer

    bool    hasAlphaLumTexStorage;              // ES 3.0 drops alpha/alpha_lum tex storage support, while ES 2.0 have it

    bool    hasTextureAlpha;                    // Not in core profile. We need to used GL_R8 + texture swizzle

    bool    hasDirectTextureAccess;             // GL 4.5 / GL_ARB_direct_state_access

    // ES 2.0 doesn't have seamless cubemap filtering and can't be enable
    // ES 3.0 has seamless cubemap filtering but it doesn't need to be enabled
    bool    hasSeamlessCubemapEnable;           // GL 3.2 / ARB_seamless_cube_map

    bool    hasSamplerObject;                   // GL 3.3 / ARB_sampler_objects / ES 3.0

    bool    hasInternalformat;      // GL 4.1: ARB_internalformat_query2

    // Vivante ES 3.0 driver does not support tex storage with DXT compressed textures.
    bool    buggyTexStorageDXT;

    bool    hasDepth24;                         // GL 3.2 / ES 3.0 / GL_OES_depth24

    bool    hasPackedDepthStencil;              // GL 3.0 / GL_OES_packed_depth_stencil / GL_EXT_packed_depth_stencil / WebGL / ES 3.0
    bool    useDiscardToAvoidRestore;   // if discard/invalidate are preferred to avoid restore (usually means clear is fullscreen quad)
    bool    useClearToAvoidRestore;     // if clear is preferred to avoid restore

    bool    hasDebugMarker;     // KHR_debug / EXT_debug_marker (ES)
    bool    hasDebugLabel;      // KHR_debug / EXT_debug_label (ES)

    int     maxShaderStorageBufferBindings;
    int     maxAtomicCounterBufferBindings;

    bool    hasIndirectDraw;                    // GL 4.0 / ARB_draw_indirect / ES 3.1
    bool    hasDrawBaseVertex;                  // GL 3.2 / ARB_draw_elements_base_vertex / ES 3.2 / OES_draw_elements_base_vertex / EXT_draw_elements_base_vertex

    // handling of different constants in gles2/gles3

    gl::BufferTarget memoryBufferTargetConst;   // If copy buffer isn't supported, we use a different constant

    // vendor-specific extensions
    // we separate them simply for clarity

    bool    hasNVNLZ;
    bool    hasNVMRT;               // gles2: special attachment points
    bool    hasNVCSAA;              // gles2: Tegra 3 has coverage sampling anti-aliasing


    bool    hasBinaryShader;                // GL 4.1 / ES3 / GL_ARB_get_program_binary / GL_OES_get_program_binary
    bool    hasBinaryShaderRetrievableHint; // GL 4.1 / ES3 / GL_ARB_get_program_binary
    bool    hasUniformBuffer;               // GL 3.2 / GL_ARB_uniform_buffer_object / ES 3.0 / GL_IMG_uniform_buffer_object

    bool    hasProgramPointSizeEnable;          // GL 3.2

    bool    supportsManualMipmaps;  // WebGL does not support manually initializing mipmap levels other than 0.
    int     majorVersion;           // Major OpenGL version, eg OpenGL 4.2 major version is 4
    int     minorVersion;           // Minor OpenGL version, eg OpenGL 4.2 minor version is 2
};

extern GraphicsCapsGLES* g_GraphicsCapsGLES;

#endif //HUAHUOENGINE_GRAPHICSCAPSGLES_H
