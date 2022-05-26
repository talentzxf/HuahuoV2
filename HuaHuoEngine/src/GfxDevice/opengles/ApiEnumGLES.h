#pragma once

#include "GfxDevice/GfxDeviceTypes.h"

// This define allows building the OpenGL back-end as if we were building WebGL. Debugging helper should be used with -force-gles20 and -force-clamped
#define FORCE_DEBUG_BUILD_WEBGL 0

namespace gl
{
    // GL specific texture dimension for buffer backed textures & couple of helpers
    enum
    {
        kTexDimBuffer = kTexDimLast + 1,
        kTexDimLastGL = kTexDimBuffer,
        kTexDimActiveCountGL = kTexDimActiveCount + 1
    };

    enum DriverQuery // Do not reorder, it would break the code!
    {
        kDriverQueryVendor,
        kDriverQueryRenderer,
        kDriverQueryVersion
    };

    enum SubmitMode
    {
        SUBMIT_FLUSH,
        SUBMIT_FINISH
    };

    enum EnabledCap
    {
        kBlend, kEnabledCapFirst = kBlend,
        kCullFace,
        kDebugOutput,
        kDebugOutputSynchronous,
        kDepthTest,
        kDither,
        kFramebufferSRGB,
        kLineSmooth,
        kMultisample,
        kPolygonOffsetFill,
        kPolygonOffsetLine,
        kRasterizerDiscard,
        kSampleAlphaToCoverage,
        kScissorTest,
        kStencilTest,
        kTextureCubeMapSeamless,
        kProgramPointSize,
        kDepthClamp,
        kConservativeRaster, kEnabledCapLast = kConservativeRaster,
    };

    enum
    {
        kEnabledCapCount = kEnabledCapLast - kEnabledCapFirst + 1
    };

    enum ObjectType
    {
        kObjectTypeInvalid = 0xDEADDEAD,
        kBuffer = 0, kObjectTypeFirst = kBuffer,
        kShader,
        kProgram,
        kVertexArray,
        kQuery,
        kTransformFeedback,
        kSampler,
        kTexture,
        kRenderbuffer,
        kFramebuffer, kObjectTypeLast = kFramebuffer
    };

    enum
    {
        kObjectTypeCount = kObjectTypeLast - kObjectTypeFirst + 1
    };

    enum BufferTarget
    {
        kBufferTargetInvalid = 0xDEADDEAD,
        kElementArrayBuffer = 0, kBufferTargetSingleBindingFirst = kElementArrayBuffer, kBufferTargetFirst = kElementArrayBuffer,
        kArrayBuffer,
        kCopyWriteBuffer,
        kCopyReadBuffer,
        kDispatchIndirectBuffer,
        kDrawIndirectBuffer, kBufferTargetSingleBindingLast = kDrawIndirectBuffer, // All the buffer targets before this point has a single binding point
        kUniformBuffer,
        kTransformFeedbackBuffer,
        kShaderStorageBuffer,
        kAtomicCounterBuffer, kBufferTargetLast = kAtomicCounterBuffer
    };

    enum
    {
        kBufferTargetCount = kBufferTargetLast - kBufferTargetFirst + 1,
        kBufferTargetSingleBindingCount = kBufferTargetSingleBindingLast - kBufferTargetSingleBindingFirst + 1
    };

    enum FramebufferRead
    {
        // We use kFramebufferReadDefault to not updated FBO glReadBuffer state and rely on the current object state
        kFramebufferReadNone = 0, kFramebufferReadFirst = kFramebufferReadNone,
        kFramebufferReadDefault,
        kFramebufferReadBack,
        kFramebufferReadColor0,
        kFramebufferReadColor1,
        kFramebufferReadColor2,
        kFramebufferReadColor3,
        kFramebufferReadColor4,
        kFramebufferReadColor5,
        kFramebufferReadColor6,
        kFramebufferReadColor7, kFramebufferReadLast = kFramebufferReadColor7
    };

    enum
    {
        kFramebufferReadCount = kFramebufferReadLast - kFramebufferReadFirst + 1
    };

    enum FramebufferTarget
    {
        kDrawFramebuffer = 0, kFramebufferTargetFirst = kDrawFramebuffer,
        kReadFramebuffer, kFramebufferTargetLast = kReadFramebuffer
    };

    enum
    {
        kFramebufferTargetCount = kFramebufferTargetLast - kFramebufferTargetFirst + 1
    };

    enum FramebufferType
    {
        kFramebufferTypeColor, kFramebufferTypeFirst = kFramebufferTypeColor,
        kFramebufferTypeDepth,
        kFramebufferTypeStencil,
        kFramebufferTypeColorDepth, kFramebufferTypeLast = kFramebufferTypeColorDepth
    };

    enum
    {
        kFramebufferTypeCount = kFramebufferTypeLast + 1
    };

    enum MaxCaps
    {
        kMaxUniformBufferBindings               = 64,
        kMaxTransformFeedbackBufferBindings     = 4,
        kMaxShaderStorageBufferBindings         = 24, // Biggest known value supported (Adreno 420 on April 2015)
        kMaxAtomicCounterBufferBindings         = 8,
        kMaxTextureBindings                     = 32
    };

    enum
    {
        kMaxVertexAttrCount = kVertexCompAttrib15 - kVertexCompAttrib0
    };

    enum VertexArrayFlags
    {
        kVertexArrayFlagNone        = 0,
        kVertexArrayFlagInteger     = 1 << 0,
        kVertexArrayFlagNormalized  = 1 << 1
    };
    ENUM_FLAGS(VertexArrayFlags);

    enum ShaderStage
    {
        kVertexShaderStage = 0, kShaderStageFirst = kVertexShaderStage,
        kControlShaderStage,
        kEvalShaderStage,
        kGeometryShaderStage,
        kFragmentShaderStage,
        kComputeShaderStage, kShaderStageLast = kComputeShaderStage
    };

    enum
    {
        kShaderStageCount = kShaderStageLast - kShaderStageFirst + 1
    };

    enum MemoryBarrierType
    {
        kBarrierVertexAttribArray = 0, kBarrierTypeFirst = kBarrierVertexAttribArray,
        kBarrierElementArray,
        kBarrierUniform,
        kBarrierTextureFetch,
        kBarrierShaderImageAccess,
        kBarrierCommand,
        kBarrierPixelBuffer,
        kBarrierTextureUpdate,
        kBarrierBufferUpdate,
        kBarrierFramebuffer,
        kBarrierTransformFeedback,
        kBarrierAtomicCounter,
        kBarrierShaderStorage, kBarrierTypeLast = kBarrierShaderStorage
    };

    enum
    {
        kBarrierTypeCount = kBarrierTypeLast - kBarrierTypeFirst + 1
    };

    enum TextureCap
    {
        kTextureCapStorageBit = (1 << 0),
    };

    enum ASTCDecodeMode
    {
        kASTCDecodeMode_R16G16B16_SFloat = 0x881A,      // GL_RGBA16F
        kASTCDecodeMode_R8G8B8A8_UNorm = 0x8058,        // GL_RGBA8
        kASTCDecodeMode_E5B9G9R9_UFloatPack32 = 0x8C3D, // GL_RGB9_E5
        kASTCDecodeMode_Default = 0 // Don't use decode_mode and rely on GL's default (unorm 8 for sRGB, otherwise fp16)
    };
}//namespace gl
