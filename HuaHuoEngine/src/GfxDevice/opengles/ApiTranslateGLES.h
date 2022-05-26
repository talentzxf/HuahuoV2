#pragma once

#include "Utilities/NonCopyable.h"
#include "Utilities/fixed_array.h"
#include "Utilities/vector_map.h"
#include "GfxDevice/GfxDeviceTypes.h"
#include "ApiFuncGLES.h"
#include "ApiEnumGLES.h"
#include "ApiConstantsGLES.h"
#include "Utilities/StaticAssert.h"

struct GraphicsCaps;

struct FormatDescGLES
{
    GLenum internalFormat;                  // internal format
    GLenum externalFormat;
    GLenum type;
    GLuint flags;                           // is compressed, is ETC, is float, is integer, etc. Flags from TextureCap enum
};

inline bool operator==(const FormatDescGLES& A, const FormatDescGLES& B)
{
    return A.internalFormat == B.internalFormat && A.externalFormat == B.externalFormat && A.type == B.type && A.flags == B.flags;
}

struct VertexFormatDescGLES
{
    GLenum type;
    gl::VertexArrayFlags flags;
};

class TranslateGLES : NonCopyable
{
public:
    void Init(const GraphicsCaps & caps, GfxDeviceLevelGL level);

    const FormatDescGLES& GetFormatDesc(GraphicsFormat format, bool assertOnAutoFormats = true) const;

    TextureDimension GetTextureTargetDimension(GLenum target) const;
    GLenum GetTextureTargetCount(TextureDimension textureDimension) const;
    GLenum GetTextureTarget(TextureDimension textureDimension, unsigned int index) const;
    TextureDimension GetTextureSamplerDimension(GLenum samplerType) const;
    bool GetTextureSamplerIsMultisampled(GLenum samplerType) const;
    bool AddExtendedTextureDefinition(TextureDimension textureDimension, GLenum target, GLenum samplerType);

    VertexFormatDescGLES GetVertexFormatDesc(VertexFormat format) const;
    GLenum ObjectType(gl::ObjectType type) const;

    GLenum FramebufferTarget(gl::FramebufferTarget target) const;

private:
    static const GLenum kInvalidEnum;

    TextureDimension GetTextureTargetDimensionNoAssert(GLenum target) const;

    void InitFramebufferTarget(const GraphicsCaps & caps);
    fixed_array<GLenum, gl::kFramebufferTargetCount> m_FramebufferTarget;

    vector_map<GLenum, TextureDimension> m_TextureTargetExt;
    fixed_array<std::vector<GLenum>, gl::kTexDimActiveCountGL> m_TextureTargetExtByDim;

    void InitTextureSampler(const GraphicsCaps & caps);
    vector_map<GLenum, TextureDimension> m_TextureSampler;

    void InitFormat(const GraphicsCaps & caps);
    fixed_array<FormatDescGLES, kGraphicsFormatCount> m_FormatGLES;

    void InitVertexType(const GraphicsCaps & caps, GfxDeviceLevelGL level);
    fixed_array<VertexFormatDescGLES, kVertexFormatCount> m_VertexFormat;

    void InitObjectType(const GraphicsCaps & caps);
    fixed_array<GLenum, gl::kObjectTypeCount> m_ObjectType;
};

namespace gl
{
    GraphicsFormat GetGraphicsFormatFromGL(const TranslateGLES& translate, GLenum glesFormat);

    inline GLenum GetTopology(GfxPrimitiveType type)
    {
        static const GLenum table[] =
        {
            GL_TRIANGLES,           // kPrimitiveTriangles
            GL_TRIANGLE_STRIP,      // kPrimitiveTriangleStrip
            GL_NONE,                // kPrimitiveQuads - Not supported
            GL_LINES,               // kPrimitiveLines
            GL_LINE_STRIP,          // kPrimitiveLineStrip
            GL_POINTS               // kPrimitivePoints
        };
        CompileTimeAssertArraySize(table, kPrimitiveTypeCount);

        return table[type];
    }

    inline GLenum GetShaderStage(ShaderStage stage)
    {
        static const GLenum table[] =               // gl::kShaderStageCount
        {
            GL_VERTEX_SHADER,                       // gl::kVertexShaderStage
            GL_TESS_CONTROL_SHADER,                 // gl::kControlShaderStage
            GL_TESS_EVALUATION_SHADER,              // gl::kEvalShaderStage
            GL_GEOMETRY_SHADER,                     // gl::kGeometryShaderStage
            GL_FRAGMENT_SHADER,                     // gl::kFragmentShaderStage
            GL_COMPUTE_SHADER,                      // gl::kComputeShaderStage
        };
        CompileTimeAssertArraySize(table, kShaderStageCount);

        return table[stage];
    }

    inline ShaderStage GetShaderStageFromGL(GLenum stage)
    {
        for (int stageStageIndex = 0; stageStageIndex < kShaderStageCount; ++stageStageIndex)
            if (GetShaderStage(static_cast<ShaderStage>(stageStageIndex)) == stage)
                return static_cast<ShaderStage>(stageStageIndex);

        DebugAssertMsg(0, "OPENGL ERROR: GetShaderStage - Invalid input enum");
        return static_cast<ShaderStage>(-1);
    }

    inline const char* GetShaderTitle(ShaderStage stage)
    {
        static const char* table[] =                    // gl::kShaderStageCount
        {
            "vertex shader",                            // gl::kVertexShaderStage
            "tessellation control shader",              // gl::kControlShaderStage
            "tessellation evaluation shader",           // gl::kEvalShaderStage
            "geometry evaluation shader",               // gl::kGeometryShaderStage
            "fragment evaluation shader",               // gl::kFragmentShaderStage
            "compute evaluation shader"                 // gl::kComputeShaderStage
        };
        CompileTimeAssertArraySize(table, kShaderStageCount);

        return table[stage];
    }

    inline GLenum GetCompareFunction(UInt8 func)
    {
        static const GLenum table[] =   // kFuncCount
        {
            GL_ALWAYS,                  // kFuncDisabled
            GL_NEVER,                   // kFuncNever
            GL_LESS,                    // kFuncLess
            GL_EQUAL,                   // kFuncEqual
            GL_LEQUAL,                  // kFuncLEqual
            GL_GREATER,                 // kFuncGreater
            GL_NOTEQUAL,                // kFuncNotEqual
            GL_GEQUAL,                  // kFuncGEqual
            GL_ALWAYS                   // kFuncAlways
        };
        CompileTimeAssertArraySize(table, kFuncCount);

        const CompareFunction compareFunction = static_cast<CompareFunction>(func);
        Assert(compareFunction >= kFuncFirst && compareFunction < kFuncCount);

        return table[compareFunction];
    }

    inline GLenum GetStencilOperation(UInt8 op)
    {
        static const GLenum table[] =   // kStencilOpCount
        {
            GL_KEEP,                    // kStencilOpKeep
            GL_ZERO,                    // kStencilOpZero
            GL_REPLACE,                 // kStencilOpReplace
            GL_INCR,                    // kStencilOpIncrSat
            GL_DECR,                    // kStencilOpDecrSat
            GL_INVERT,                  // kStencilOpInvert
            GL_INCR_WRAP,               // kStencilOpIncrWrap
            GL_DECR_WRAP                // kStencilOpDecrWrap
        };
        CompileTimeAssertArraySize(table, kStencilOpCount);

        const StencilOp stencilOp = static_cast<StencilOp>(op);
        Assert(stencilOp >= kStencilOpFirst && stencilOp < kStencilOpCount);

        return table[stencilOp];
    }

    struct colorMask { GLboolean r, g, b, a; };

    inline colorMask GetColorMask(const UInt32 colorMaskBitfield)
    {
        colorMask mask;
        mask.r = (colorMaskBitfield & kColorWriteR) != 0 ? GL_TRUE : GL_FALSE;
        mask.g = (colorMaskBitfield & kColorWriteG) != 0 ? GL_TRUE : GL_FALSE;
        mask.b = (colorMaskBitfield & kColorWriteB) != 0 ? GL_TRUE : GL_FALSE;
        mask.a = (colorMaskBitfield & kColorWriteA) != 0 ? GL_TRUE : GL_FALSE;
        return mask;
    }

    inline GLenum GetBlendFactor(UInt8 mode)
    {
        static const GLenum table[] =
        {
            GL_ZERO,                    // kBlendZero
            GL_ONE,                     // kBlendOne
            GL_DST_COLOR,               // kBlendDstColor
            GL_SRC_COLOR,               // kBlendSrcColor
            GL_ONE_MINUS_DST_COLOR,     // kBlendOneMinusDstColor
            GL_SRC_ALPHA,               // kBlendSrcAlpha
            GL_ONE_MINUS_SRC_COLOR,     // kBlendOneMinusSrcColor
            GL_DST_ALPHA,               // kBlendDstAlpha
            GL_ONE_MINUS_DST_ALPHA,     // kBlendOneMinusDstAlpha
            GL_SRC_ALPHA_SATURATE,      // kBlendSrcAlphaSaturate
            GL_ONE_MINUS_SRC_ALPHA      // kBlendOneMinusSrcAlpha
        };
        CompileTimeAssertArraySize(table, kBlendCount);

        const BlendMode blendMode = static_cast<BlendMode>(mode);
        Assert(blendMode >= kBlendFirst && blendMode < kBlendCount);

        return table[blendMode];
    }

    inline GLenum GetBlendEquation(UInt8 equation)
    {
        static const GLenum table[] =
        {
            GL_FUNC_ADD,                    // kBlendOpAdd
            GL_FUNC_SUBTRACT,               // kBlendOpSub
            GL_FUNC_REVERSE_SUBTRACT,       // kBlendOpRevSub
            GL_MIN,                         // kBlendOpMin, GL 1.1, ES 3.0,  GL_EXT_blend_minmax
            GL_MAX,                         // kBlendOpMax, GL 1.1, ES 3.0, GL_EXT_blend_minmax
            GL_CLEAR,                       // kBlendOpLogicalClear
            GL_SET,                         // kBlendOpLogicalSet,
            GL_COPY,                        // kBlendOpLogicalCopy,
            GL_COPY_INVERTED,               // kBlendOpLogicalCopyInverted,
            GL_NOOP,                        // kBlendOpLogicalNoop,
            GL_INVERT,                      // kBlendOpLogicalInvert,
            GL_AND,                         // kBlendOpLogicalAnd,
            GL_NAND,                        // kBlendOpLogicalNand,
            GL_OR,                          // kBlendOpLogicalOr,
            GL_NOR,                         // kBlendOpLogicalNor,
            GL_XOR,                         // kBlendOpLogicalXor,
            GL_EQUIV,                       // kBlendOpLogicalEquiv,
            GL_AND_REVERSE,                 // kBlendOpLogicalAndReverse,
            GL_AND_INVERTED,                // kBlendOpLogicalAndInverted,
            GL_OR_REVERSE,                  // kBlendOpLogicalOrReverse,
            GL_OR_INVERTED,                 // kBlendOpLogicalOrInverted,
            GL_MULTIPLY_KHR,                // kBlendOpMultiply
            GL_SCREEN_KHR,                  // kBlendOpScreen
            GL_OVERLAY_KHR,                 // kBlendOpOverlay
            GL_DARKEN_KHR,                  // kBlendOpDarken
            GL_LIGHTEN_KHR,                 // kBlendOpLighten
            GL_COLORDODGE_KHR,              // kBlendOpColorDodge
            GL_COLORBURN_KHR,               // kBlendOpColorBurn
            GL_HARDLIGHT_KHR,               // kBlendOpHardLight
            GL_SOFTLIGHT_KHR,               // kBlendOpSoftLight
            GL_DIFFERENCE_KHR,              // kBlendOpDifference
            GL_EXCLUSION_KHR,               // kBlendOpExclusion
            GL_HSL_HUE_KHR,                 // kBlendOpHSLHue
            GL_HSL_SATURATION_KHR,          // kBlendOpHSLSaturation
            GL_HSL_COLOR_KHR,               // kBlendOpHSLColor
            GL_HSL_LUMINOSITY_KHR           // kBlendOpHSLLuminosity
        };
        CompileTimeAssertArraySize(table, kBlendOpCount);

        const BlendOp blendequation = static_cast<BlendOp>(equation);
        Assert(blendequation >= kBlendOpFirst && blendequation < kBlendOpCount);

        return table[blendequation];
    }

    inline GLenum GetBufferTarget(BufferTarget target)
    {
        static const GLenum table[] =       // kBufferTargetCount
        {
            GL_ELEMENT_ARRAY_BUFFER,        // kElementArrayBuffer
            GL_ARRAY_BUFFER,                // kArrayBuffer
            GL_COPY_WRITE_BUFFER,           // kCopyWriteBuffer
            GL_COPY_READ_BUFFER,            // kCopyReadBuffer
            GL_DISPATCH_INDIRECT_BUFFER,    // kDispatchIndirectBuffer
            GL_DRAW_INDIRECT_BUFFER,        // kDrawIndirectBuffer
            GL_UNIFORM_BUFFER,              // kUniformBuffer
            GL_TRANSFORM_FEEDBACK_BUFFER,   // kTransformFeedbackBuffer
            GL_SHADER_STORAGE_BUFFER,       // kShaderStorageBuffer
            GL_ATOMIC_COUNTER_BUFFER        // kAtomicCounterBuffer
        };
        CompileTimeAssertArraySize(table, kBufferTargetCount);

        return table[target];
    }

    inline GLenum GetMemoryBarrierBits(MemoryBarrierType type)
    {
        static const GLenum table[] =
        {
            GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT, //kBarrierVertexAttribArray
            GL_ELEMENT_ARRAY_BARRIER_BIT,       //kBarrierElementArray
            GL_UNIFORM_BARRIER_BIT,             //kBarrierUniform
            GL_TEXTURE_FETCH_BARRIER_BIT,       //kBarrierTextureFetch
            GL_SHADER_IMAGE_ACCESS_BARRIER_BIT, //kBarrierShaderImageAccess
            GL_COMMAND_BARRIER_BIT,             //kBarrierCommand
            GL_PIXEL_BUFFER_BARRIER_BIT,        //kBarrierPixelBuffer
            GL_TEXTURE_UPDATE_BARRIER_BIT,      //kBarrierTextureUpdate
            GL_BUFFER_UPDATE_BARRIER_BIT,       //kBarrierBufferUpdate
            GL_FRAMEBUFFER_BARRIER_BIT,         //kBarrierFramebuffer
            GL_TRANSFORM_FEEDBACK_BARRIER_BIT,  //kBarrierTransformFeedback
            GL_ATOMIC_COUNTER_BARRIER_BIT,      //kBarrierAtomicCounter
            GL_SHADER_STORAGE_BARRIER_BIT       //kBarrierShaderStorage
        };
        CompileTimeAssertArraySize(table, kBarrierTypeCount);

        return table[type];
    }

    inline GLenum GetEnable(EnabledCap cap)
    {
        static const GLenum table[] =
        {
            GL_BLEND,                           //kBlend,
            GL_CULL_FACE,                       //kCullFace,
            GL_DEBUG_OUTPUT,                    //kDebugOutput
            GL_DEBUG_OUTPUT_SYNCHRONOUS,        //kDebugOutputSynchronous
            GL_DEPTH_TEST,                      //kDepthTest,
            GL_DITHER,                          //kDither,
            GL_FRAMEBUFFER_SRGB,                //kFramebufferSRGB,
            GL_LINE_SMOOTH,                     //kLineSmooth,
            GL_MULTISAMPLE,                     //kMultisample,
            GL_POLYGON_OFFSET_FILL,             //kPolygonOffsetFill,
            GL_POLYGON_OFFSET_LINE,             //kPolygonOffsetLine,
            GL_RASTERIZER_DISCARD,              //kRasterizerDiscard,
            GL_SAMPLE_ALPHA_TO_COVERAGE,        //kSampleAlphaToCoverage,
            GL_SCISSOR_TEST,                    //kScissorTest,
            GL_STENCIL_TEST,                    //kStencilTest,
            GL_TEXTURE_CUBE_MAP_SEAMLESS,       //kTextureCubeMapSeamless,
            GL_PROGRAM_POINT_SIZE,              //kProgramPointSize,
            GL_DEPTH_CLAMP,                     //kDepthClamp,
            GL_CONSERVATIVE_RASTER,             //kConservativeRaster,
        };
        CompileTimeAssertArraySize(table, kEnabledCapCount);

        return table[cap];
    }

    inline GLenum GetFramebufferType(FramebufferType type)
    {
        static const GLenum table[] =
        {
            GL_COLOR_BUFFER_BIT,                        // kFramebufferTypeColor
            GL_DEPTH_BUFFER_BIT,                        // kFramebufferTypeDepth
            GL_STENCIL_BUFFER_BIT,                      // kFramebufferTypeStencil
            GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,  // kFramebufferTypeColorDepth
        };
        CompileTimeAssertArraySize(table, kFramebufferTypeCount);

        return table[type];
    }

    inline GLenum GetFramebufferRead(FramebufferRead read)
    {
        Assert(read >= kFramebufferReadFirst && read <= kFramebufferReadLast && read != kFramebufferReadDefault);

        static const GLenum translation[] = // gl::kFramebufferTargetCount
        {
            GL_NONE,                    // kFramebufferReadNone
            GL_NONE,                    // kFramebufferReadDefault
            GL_BACK,                    // kFramebufferReadBack
            GL_COLOR_ATTACHMENT0 + 0,   // kFramebufferReadColor0
            GL_COLOR_ATTACHMENT0 + 1,   // kFramebufferReadColor1
            GL_COLOR_ATTACHMENT0 + 2,   // kFramebufferReadColor2
            GL_COLOR_ATTACHMENT0 + 3,   // kFramebufferReadColor3
            GL_COLOR_ATTACHMENT0 + 4,   // kFramebufferReadColor4
            GL_COLOR_ATTACHMENT0 + 5,   // kFramebufferReadColor5
            GL_COLOR_ATTACHMENT0 + 6,   // kFramebufferReadColor6
            GL_COLOR_ATTACHMENT0 + 7    // kFramebufferReadColor7
        };
        CompileTimeAssertArraySize(translation, kFramebufferReadCount);

        return translation[read];
    }

    inline GLenum GetTextureTarget(TextureDimension dim, bool multiSampled = false)
    {
        AssertFormatMsg(dim >= kTexDimFirst && dim <= static_cast<TextureDimension>(kTexDimLastGL), "OPENGL ERROR: GetTextureTargetGLES - Invalid input: %d", dim);

        AssertMsg(!multiSampled || (dim == kTexDim2D || dim == kTexDim2DArray), "OPENGL ERROR: GetTextureTargetGLES - binding multisampled textures only supported for 2D textures and 2D texture arrays.");

        if (multiSampled)
        {
            if (dim == kTexDim2D)
            {
                return GL_TEXTURE_2D_MULTISAMPLE;
            }
            else if (dim == kTexDim2DArray)
            {
                return GL_TEXTURE_2D_MULTISAMPLE_ARRAY;
            }
        }

        static const GLenum table[] =
        {
            GL_TEXTURE_2D,                              // kTexDim2D
            GL_TEXTURE_3D,                              // kTexDim3D
            GL_TEXTURE_CUBE_MAP,                        // kTexDimCUBE
            GL_TEXTURE_2D_ARRAY,                        // kTexDim2DArray
            GL_TEXTURE_CUBE_MAP_ARRAY,                  // kTexDimCubeArray
            GL_TEXTURE_BUFFER,                          // gl::kTexDimBuffer
        };
        CompileTimeAssertArraySize(table, kTexDimLastGL - kTexDimFirst + 1);

        return table[dim - kTexDimFirst];
    }

    inline TextureDimension GetTextureDimensionFromGL(GLenum target)
    {
        if (target == GL_TEXTURE_2D_MULTISAMPLE)
            return kTexDim2D;

        if (target == GL_TEXTURE_2D_MULTISAMPLE_ARRAY)
            return kTexDim2DArray;

        for (int i = kTexDimFirst; i <= kTexDimLastGL; ++i)
        {
            const TextureDimension dim = static_cast<TextureDimension>(i);
            if (GetTextureTarget(dim) == target)
                return dim;
        }
        return kTexDimUnknown;
    }

    inline bool IsMultisampledTextureFormat(GLenum target)
    {
        return (target == GL_TEXTURE_2D_MULTISAMPLE) || (target == GL_TEXTURE_2D_MULTISAMPLE_ARRAY);
    }

    inline GLenum GetFilterMin(GLenum textureTarget, TextureFilterMode filter, bool hasMipMaps)
    {
        static const GLenum table[] =
        {
            GL_NEAREST,                     // kTexFilterNearest
            GL_LINEAR,                      // kTexFilterBilinear
            GL_NONE                         // kTexFilterTrilinear requires mipmaps, not supported
        };
        CompileTimeAssertArraySize(table, kTexFilterCount);

        static const GLenum tableMipMaps[] =
        {
            GL_NEAREST_MIPMAP_NEAREST,      // kTexFilterNearest
            GL_LINEAR_MIPMAP_NEAREST,       // kTexFilterBilinear
            GL_LINEAR_MIPMAP_LINEAR         // kTexFilterTrilinear
        };
        CompileTimeAssertArraySize(tableMipMaps, kTexFilterCount);

        // OES_EGL_image_external does not support mipmaps.
        static const GLenum externalTable[] =
        {
            GL_NEAREST,                     // kTexFilterNearest
            GL_LINEAR,                      // kTexFilterBilinear
            GL_LINEAR                       // kTexFilterTrilinear
        };
        CompileTimeAssertArraySize(externalTable, kTexFilterCount);

        if (textureTarget == GL_TEXTURE_EXTERNAL_OES)
        {
            return externalTable[filter];
        }

        return hasMipMaps ? tableMipMaps[filter] : table[filter];
    }

    inline GLenum GetFilterMag(TextureFilterMode filter)
    {
        static const GLenum table[] =
        {
            GL_NEAREST,                     // kTexFilterNearest
            GL_LINEAR,                      // kTexFilterBilinear
            GL_LINEAR                       // kTexFilterTrilinear
        };
        CompileTimeAssertArraySize(table, kTexFilterCount);

        return table[filter];
    }

    inline GLenum GetWrap(TextureWrapMode wrap)
    {
        static const GLenum table[] =
        {
            GL_REPEAT,                  // kTexWrapRepeat
            GL_CLAMP_TO_EDGE,           // kTexWrapClamp
            GL_MIRRORED_REPEAT,         // kTexWrapMirror
            GL_MIRROR_CLAMP_TO_EDGE,    // kTexWrapMirrorOnce
        };
        CompileTimeAssertArraySize(table, kTexWrapCount);

        return table[wrap];
    }

    struct LevelDesc
    {
        int major;
        int minor;
        ShaderRequirements shaderCaps;
    };

    // Access OpenGL/ES level specific properties
    const LevelDesc& GetLevelDesc(GfxDeviceLevelGL level);

    // Retrieve the Unity GfxDeviceLevelGL from OpenGL profile, major and minor version.
    GfxDeviceLevelGL GetDeviceLevel(int major, int minor, bool isES = false);
}//namespace gl
