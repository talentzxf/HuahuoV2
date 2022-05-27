//////////////////////////////////////////////////////////////////////////
// Design direction for the translation tables
//
// All translation tables which content may change because of different
// levels when recreating the GfxDeviceGLES needs to be define const
// instead of static const because static variables are initialized for
// life in C++.
//
// However, when initializing a variable/array const, compilers translate
// the code into a long series of instruction transforming a simple and
// fast table indexing into a costly execution prone for instruction cache
// evictions.
//
// As a result, it' ok-ish to declare translation tables in
// TranslateGLES::Init*** function const only because those are called only
// as GfxdeviceGLES creation but all the translation query functions
// should use static const tables or accessing initialized tables.
//////////////////////////////////////////////////////////////////////////

#include "ApiFuncGLES.h"
#include "ApiTranslateGLES.h"
#include "ApiConstantsGLES.h"
#include "GraphicsCapsGLES.h"
#include "Shaders/GraphicsCaps.h"
#include "Utilities/ArrayUtility.h"

#if FORCE_DEBUG_BUILD_WEBGL
#   undef PLATFORM_WEBGL
#   define PLATFORM_WEBGL 1
#endif//FORCE_DEBUG_BUILD_WEBGL

#define UNITY_DESKTOP (PLATFORM_WIN || PLATFORM_LINUX || PLATFORM_OSX)

void TranslateGLES::Init(const GraphicsCaps & caps, GfxDeviceLevelGL level)
{
    this->InitFormat(caps);
    this->InitTextureSampler(caps);
    this->InitVertexType(caps, level);
    this->InitObjectType(caps);
    this->InitFramebufferTarget(caps);

#if PLATFORM_OSX
    // OSX uses GL_TEXTURE_RECTANGLEs in its CoreVideo API.
    this->AddExtendedTextureDefinition(kTexDim2D, GL_TEXTURE_RECTANGLE, GL_SAMPLER_2D_RECT);
#endif

#if PLATFORM_ANDROID || PLATFORM_LUMIN
    // Android uses GL_TEXTURE_EXTERNAL_OESs in its SurfaceTexture API.
    this->AddExtendedTextureDefinition(kTexDim2D, GL_TEXTURE_EXTERNAL_OES, GL_SAMPLER_EXTERNAL_OES);
#endif
}

TextureDimension TranslateGLES::GetTextureTargetDimensionNoAssert(GLenum target) const
{
    // Linear search through the list of known texture targets. The assumption is that the
    // list is short and ordered by most frequently used texture type.
    const TextureDimension dim = gl::GetTextureDimensionFromGL(target);
    if (dim != kTexDimUnknown)
        return dim;

    const vector_map<GLenum, TextureDimension>::const_iterator it = m_TextureTargetExt.find(target);
    return (it == m_TextureTargetExt.end()) ? kTexDimUnknown : it->second;
}

TextureDimension TranslateGLES::GetTextureTargetDimension(GLenum target) const
{
    const TextureDimension ret = GetTextureTargetDimensionNoAssert(target);

    DebugAssertMsg(ret != kTexDimUnknown, "OPENGL ERROR: TranslateGLES::GetTextureTargetDimension - Invalid input enum");
    return ret;
}

GLenum TranslateGLES::GetTextureTargetCount(TextureDimension textureDimension) const
{
    AssertFormatMsg(textureDimension >= kTexDimFirst && textureDimension <= (TextureDimension)gl::kTexDimLastGL, "OPENGL ERROR: TranslateGLES::GetTextureTargetCount - Invalid input: %d", textureDimension);
    return 1 + m_TextureTargetExtByDim[textureDimension - kTexDimFirst].size();
}

GLenum TranslateGLES::GetTextureTarget(TextureDimension textureDimension, unsigned int index) const
{
    AssertFormatMsg(textureDimension >= kTexDimFirst && textureDimension <= (TextureDimension)gl::kTexDimLastGL, "OPENGL ERROR: TranslateGLES::GetTextureTarget - Invalid input: %d", textureDimension);
    const int dimIndex = textureDimension - kTexDimFirst;
    return index == 0 ? gl::GetTextureTarget(textureDimension) : m_TextureTargetExtByDim[dimIndex][index - 1];
}

void TranslateGLES::InitTextureSampler(const GraphicsCaps &)
{
    const GLenum samplerTypes[][7] =
    {
        { // kTexDim2D
            GL_SAMPLER_2D,
            GL_SAMPLER_2D_SHADOW,
            GL_SAMPLER_2D_MULTISAMPLE,
            GL_INT_SAMPLER_2D,
            GL_INT_SAMPLER_2D_MULTISAMPLE,
            GL_UNSIGNED_INT_SAMPLER_2D,
            GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE
        },
        { // kTexDim3D
            GL_SAMPLER_3D,
            GL_INT_SAMPLER_3D,
            GL_UNSIGNED_INT_SAMPLER_3D
        },
        { // kTexDimCUBE
            GL_SAMPLER_CUBE,
            GL_SAMPLER_CUBE_SHADOW,
            GL_INT_SAMPLER_CUBE,
            GL_UNSIGNED_INT_SAMPLER_CUBE
        },
        { // kTexDim2DArray
            GL_SAMPLER_2D_ARRAY,
            GL_SAMPLER_2D_ARRAY_SHADOW,
            GL_SAMPLER_2D_MULTISAMPLE_ARRAY,
            GL_INT_SAMPLER_2D_ARRAY,
            GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY,
            GL_UNSIGNED_INT_SAMPLER_2D_ARRAY,
            GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY
        },
        { // kTexDimCubeArray
            GL_SAMPLER_CUBE_MAP_ARRAY,
            GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW,
            GL_INT_SAMPLER_CUBE_MAP_ARRAY,
            GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY
        },
        { // gl::kTexDimBuffer
            GL_SAMPLER_BUFFER,
            GL_INT_SAMPLER_BUFFER,
            GL_UNSIGNED_INT_SAMPLER_BUFFER
        }
    };

    for (int index = 0; index < gl::kTexDimActiveCountGL; ++index)
    {
        const TextureDimension dim = static_cast<TextureDimension>(index + kTexDimFirst);
        for (int i = 0; i < ARRAY_SIZE(*samplerTypes) && samplerTypes[index][i] != 0; ++i)
            m_TextureSampler[samplerTypes[index][i]] = dim;
    }
}

TextureDimension TranslateGLES::GetTextureSamplerDimension(GLenum samplerType) const
{
    const vector_map<GLenum, TextureDimension>::const_iterator it = m_TextureSampler.find(samplerType);
    return it == m_TextureSampler.end() ? kTexDimUnknown : it->second;
}

bool TranslateGLES::GetTextureSamplerIsMultisampled(GLenum samplerType) const
{
    return (samplerType == GL_SAMPLER_2D_MULTISAMPLE) ||
        (samplerType == GL_SAMPLER_2D_MULTISAMPLE_ARRAY) ||
        (samplerType == GL_INT_SAMPLER_2D_MULTISAMPLE) ||
        (samplerType == GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY) ||
        (samplerType == GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE) ||
        (samplerType == GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY);
}

bool TranslateGLES::AddExtendedTextureDefinition(TextureDimension textureDimension, GLenum target, GLenum samplerType)
{
    if (textureDimension < kTexDimFirst || textureDimension > (TextureDimension)gl::kTexDimLastGL)
        return false;

    // Check we're not trying to register a sampler type to a different texture dimensionality.
    const TextureDimension samplerDim = GetTextureSamplerDimension(samplerType);
    if (samplerDim != kTexDimUnknown && samplerDim != textureDimension)
        return false;

    // Check we're not trying to register a target to a different texture dimensionality.
    const TextureDimension targetDim = GetTextureTargetDimensionNoAssert(target);
    if (targetDim != kTexDimUnknown && targetDim != textureDimension)
        return false;

    if (samplerDim == kTexDimUnknown)
        m_TextureSampler[samplerType] = textureDimension;

    if (targetDim == kTexDimUnknown)
    {
        m_TextureTargetExt[target] = textureDimension;
        m_TextureTargetExtByDim[textureDimension - kTexDimFirst].push_back(target);
    }

    return true;
}

GraphicsFormat gl::GetGraphicsFormatFromGL(const TranslateGLES& translate, GLenum glesFormat)
{
    for (int i = 0; i < kGraphicsFormatCount; ++i)
    {
        const GraphicsFormat format = static_cast<GraphicsFormat>(i);
        const FormatDescGLES& desc = translate.GetFormatDesc(format);

        if (desc.internalFormat == glesFormat)
            return format;
    }

    switch (glesFormat)
    {
        case GL_SRGB_ALPHA_EXT:
            return kFormatR8G8B8A8_SRGB;
        case GL_BGRA8_EXT:
            return kFormatB8G8R8A8_UNorm;
        case GL_ETC1_RGB8_OES:
            return kFormatRGB_ETC2_UNorm;
        default:
            Assert(false && "Unsupported OpenGL format");
            return kFormatR8G8B8A8_UNorm;
    }
}

void TranslateGLES::InitFormat(const GraphicsCaps& caps)
{
    const bool hasSRGB = caps.hasSRGBReadWrite;

    // ES2, when texture storage isn't supported, only supports unsized formats
    const bool sizedFormat = !IsGfxLevelES2(caps.gles.featureLevel) || caps.gles.hasTextureStorage;
    const bool hasUnsizedSRGB = hasSRGB && !sizedFormat;

    const GLenum internalRGBA8 = sizedFormat ? GL_RGBA8 : GL_RGBA;
    const GLenum internalSRGB8 = sizedFormat ? GL_SRGB8 : GL_SRGB;
    const GLenum internalSRGB8_A8 = sizedFormat ? GL_SRGB8_ALPHA8 : GL_SRGB_ALPHA_EXT;
    const GLenum externalSRGB8 = hasUnsizedSRGB ? GL_SRGB : GL_RGB;
    const GLenum externalSRGB8_A8 = hasUnsizedSRGB ? GL_SRGB_ALPHA_EXT : GL_RGBA;

    // Core profile doesn't support alpha texture.
    // With GL 3.3+ and ES3.0+ we use red texture and texture swizzle to fetch from the alpha channel in the shader
    // GL3.2 convert the alpha texture into RGBA data hence this format doesn't apply
    // ES2 doesn't support RED formats and texture storage doesn't support alpha format. Use GL_ALPHA for this case.
    const GLenum internalAlpha8 = caps.gles.hasTextureSwizzle ? GL_R8 : GL_ALPHA;
    const GLenum externalAlpha8 = caps.gles.hasTextureSwizzle ? GL_RED : GL_ALPHA;

    // use LUMINANCE instead of R8 on ES2.0 drivers that't don't support R8
    const GLenum internalR8 = GetGraphicsCaps().gles.hasTextureRG ? GL_R8 : GL_LUMINANCE;
    const GLenum externalR8 = GetGraphicsCaps().gles.hasTextureRG ? GL_RED : GL_LUMINANCE;

    // By default (ES 3.0 / GL 3.3) Swizzling is handled by texture swizzle to we use RGBA formats
    GLenum internalBGRA8 = internalRGBA8;
    GLenum internalSBGR8_A8 = internalSRGB8_A8;
    GLenum externalBGRA8 = GL_RGBA;

    // When texture swizzle isn't supported we fallback to swizzle formats (ES 2.0 and GL 3.2)
    if (!caps.gles.hasTextureSwizzle)
    {
        if (IsGfxLevelCore(caps.gles.featureLevel))
        {
            internalBGRA8 = GL_RGBA8;
            externalBGRA8 = GL_BGRA;
        }
        else
        {
            // EXT_texture_format_BGRA8888 and APPLE_texture_format_BGRA8888 dictates internal format for GL_BGRA_EXT to be GL_RGBA
            // BUT in case of texture storage support, it must be GL_BGRA8_EXT
            internalBGRA8 = caps.gles.hasTextureStorage ? GL_BGRA8_EXT : GL_BGRA;
            externalBGRA8 = GL_BGRA;
        }
    }

    // ES2 OES_texture_float defines a different enum value for half float type than ES and desktop
    const GLenum typeHalf = IsGfxLevelES2(caps.gles.featureLevel) ? GL_HALF_FLOAT_OES : GL_HALF_FLOAT;

    // RGB8 ETC2 is a superset of RGB8 ETC1 except that the first enum is only used for a ES2 extension
    const GLenum internalETC1 = IsGfxLevelES2(caps.gles.featureLevel) ? GL_ETC1_RGB8_OES : GL_COMPRESSED_RGB8_ETC2;

    // Initalize per texture format caps
    const GLuint capStorage = caps.gles.hasTextureStorage ? gl::kTextureCapStorageBit : 0;

    // See buggyTexStorageDXT declaration comment
    const GLuint capDXT = g_GraphicsCapsGLES->buggyTexStorageDXT ? 0 : capStorage;

    // Depth formats
    const GLenum typeDepth24 = caps.gles.hasDepth24 ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT;
    const GLenum typeDepthStencil = caps.gles.hasPackedDepthStencil ? GL_UNSIGNED_INT_24_8 : typeDepth24;

    const GLenum internalDepth16 = caps.gles.hasNVNLZ ? GL_DEPTH_COMPONENT16_NONLINEAR_NV : GL_DEPTH_COMPONENT16;
    const GLenum internalDepth24 = caps.gles.hasDepth24 ? GL_DEPTH_COMPONENT24 : internalDepth16;
    GLenum internalDepthStencilUnsized = /*PLATFORM_WEBGL &&*/ IsGfxLevelES2(caps.gles.featureLevel) ? GL_DEPTH_STENCIL : GL_DEPTH24_STENCIL8;
    const GLenum internalDepthStencil = caps.gles.hasPackedDepthStencil ? internalDepthStencilUnsized : internalDepth24;

    const GLenum externalDepthStencil = caps.gles.hasPackedDepthStencil ? GL_DEPTH_STENCIL : GL_DEPTH_COMPONENT;

    // Texture storage isn't supported with (ES2 only) alpha textures
    const GLuint capAlpha = !IsGfxLevelES2(caps.gles.featureLevel) && caps.gles.hasTextureStorage && caps.gles.hasTextureSwizzle ? gl::kTextureCapStorageBit : 0;
    const GLuint flagAlpha = caps.gles.hasTextureSwizzle ? capStorage : capAlpha;

    // See declaration of TextureFormatGLES for a description of each member
    const FormatDescGLES table[] = // kFormatCount
    {
        //GLenum internalFormat                         GLenum externalFormat       GLenum type                     GLuint flags            // GraphicsFormat
        {GL_NONE,                                       GL_NONE,                    GL_NONE,                        capStorage},            // kFormatNone

        {GL_SR8_EXT,                                    GL_RED,                     GL_UNSIGNED_BYTE,               capStorage},            // kFormatR8_SRGB
        {GL_SRG8_EXT,                                   GL_RG,                      GL_UNSIGNED_BYTE,               capStorage},            // kFormatR8G8_SRGB
        {internalSRGB8,                                 externalSRGB8,              GL_UNSIGNED_BYTE,               capStorage},            // kTexFormatRGB8srgb
        {internalSRGB8_A8,                              externalSRGB8_A8,           GL_UNSIGNED_BYTE,               capStorage},            // kTexFormatRGBA8srgb

        {internalR8,                                    externalR8,                 GL_UNSIGNED_BYTE,               capStorage},            // kFormatR8_UNorm
        {GL_RG8,                                        GL_RG,                      GL_UNSIGNED_BYTE,               capStorage},            // kFormatR8G8_UNorm
        {GL_RGB8,                                       GL_RGB,                     GL_UNSIGNED_BYTE,               capStorage},            // kFormatR8G8B8_UNorm
        {GL_RGBA8,                                      GL_RGBA,                    GL_UNSIGNED_BYTE,               capStorage},            // kFormatR8G8B8A8_UNorm
        {GL_R8_SNORM,                                   GL_RED,                     GL_BYTE,                        capStorage},            // kFormatR8_SNorm
        {GL_RG8_SNORM,                                  GL_RG,                      GL_BYTE,                        capStorage},            // kFormatR8G8_SNorm
        {GL_RGB8_SNORM,                                 GL_RGB,                     GL_BYTE,                        capStorage},            // kFormatR8G8B8_SNorm
        {GL_RGBA8_SNORM,                                GL_RGBA,                    GL_BYTE,                        capStorage},            // kFormatR8G8B8A8_SNorm
        {GL_R8UI,                                       GL_RED_INTEGER,             GL_UNSIGNED_BYTE,               capStorage},            // kFormatR8_UInt
        {GL_RG8UI,                                      GL_RG_INTEGER,              GL_UNSIGNED_BYTE,               capStorage},            // kFormatR8G8_UInt
        {GL_RGB8UI,                                     GL_RGB_INTEGER,             GL_UNSIGNED_BYTE,               capStorage},            // kFormatR8G8B8_UInt
        {GL_RGBA8UI,                                    GL_RGBA_INTEGER,            GL_UNSIGNED_BYTE,               capStorage},            // kFormatR8G8B8A8_UInt
        {GL_R8I,                                        GL_RED_INTEGER,             GL_BYTE,                        capStorage},            // kFormatR8_SInt
        {GL_RG8I,                                       GL_RG_INTEGER,              GL_BYTE,                        capStorage},            // kFormatR8G8_SInt
        {GL_RGB8I,                                      GL_RGB_INTEGER,             GL_BYTE,                        capStorage},            // kFormatR8G8B8_SInt
        {GL_RGBA8I,                                     GL_RGBA_INTEGER,            GL_BYTE,                        capStorage},            // kFormatR8G8B8A8_SInt

        {GL_R16,                                        GL_RED,                     GL_UNSIGNED_SHORT,              capStorage},            // kFormatR16_UNorm
        {GL_RG16,                                       GL_RG,                      GL_UNSIGNED_SHORT,              capStorage},            // kFormatR16G16_UNorm
        {GL_RGB16,                                      GL_RGB,                     GL_UNSIGNED_SHORT,              capStorage},            // kFormatR16G16B16_UNorm
        {GL_RGBA16,                                     GL_RGBA,                    GL_UNSIGNED_SHORT,              capStorage},            // kFormatR16G16B16A16_UNorm
        {GL_R16_SNORM,                                  GL_RED,                     GL_SHORT,                       capStorage},            // kFormatR16_SNorm
        {GL_RG16_SNORM,                                 GL_RG,                      GL_SHORT,                       capStorage},            // kFormatR16G16_SNorm
        {GL_RGB16_SNORM,                                GL_RGB,                     GL_SHORT,                       capStorage},            // kFormatRGB16_SNorm
        {GL_RGBA16_SNORM,                               GL_RGBA,                    GL_SHORT,                       capStorage},            // kFormatR16G16B16A16_SNorm
        {GL_R16UI,                                      GL_RED_INTEGER,             GL_UNSIGNED_SHORT,              capStorage},            // kFormatR16_UInt
        {GL_RG16UI,                                     GL_RG_INTEGER,              GL_UNSIGNED_SHORT,              capStorage},            // kFormatR16G16_UInt
        {GL_RGB16UI,                                    GL_RGB_INTEGER,             GL_UNSIGNED_SHORT,              capStorage},            // kFormatRGB16_UInt
        {GL_RGBA16UI,                                   GL_RGBA_INTEGER,            GL_UNSIGNED_SHORT,              capStorage},            // kFormatR16G16B16A16_UInt
        {GL_R16I,                                       GL_RED_INTEGER,             GL_SHORT,                       capStorage},            // kFormatR16_SInt
        {GL_RG16I,                                      GL_RG_INTEGER,              GL_SHORT,                       capStorage},            // kFormatR16G16_SInt
        {GL_RGB16I,                                     GL_RGB_INTEGER,             GL_SHORT,                       capStorage},            // kFormatRGB16_SInt
        {GL_RGBA16I,                                    GL_RGBA_INTEGER,            GL_SHORT,                       capStorage},            // kFormatR16G16B16A16_SInt

        {GL_R32UI,                                      GL_RED_INTEGER,             GL_UNSIGNED_INT,                capStorage},            // kFormatR32_UInt
        {GL_RG32UI,                                     GL_RG_INTEGER,              GL_UNSIGNED_INT,                capStorage},            // kFormatR32G32_UInt
        {GL_RGB32UI,                                    GL_RGB_INTEGER,             GL_UNSIGNED_INT,                capStorage},            // kFormatR32G32B32_UInt
        {GL_RGBA32UI,                                   GL_RGBA_INTEGER,            GL_UNSIGNED_INT,                capStorage},            // kFormatR32G32B32A32_UInt
        {GL_R32I,                                       GL_RED_INTEGER,             GL_INT,                         capStorage},            // kFormatR32_SInt
        {GL_RG32I,                                      GL_RG_INTEGER,              GL_INT,                         capStorage},            // kFormatR32G32_SInt
        {GL_RGB32I,                                     GL_RGB_INTEGER,             GL_INT,                         capStorage},            // kFormatR32G32B32_SInt
        {GL_RGBA32I,                                    GL_RGBA_INTEGER,            GL_INT,                         capStorage},            // kFormatR32G32B32A32_SInt

        {GL_R16F,                                       GL_RED,                     typeHalf,                       capStorage},            // kFormatR16_SFloat
        {GL_RG16F,                                      GL_RG,                      typeHalf,                       capStorage},            // kFormatR16G16_SFloat
        {GL_RGB16F,                                     GL_RGB,                     typeHalf,                       capStorage},            // kFormatR16G16B16_SFloat
        {GL_RGBA16F,                                    GL_RGBA,                    typeHalf,                       capStorage},            // kFormatR16G16B16A16_SFloat
        {GL_R32F,                                       GL_RED,                     GL_FLOAT,                       capStorage},            // kFormatR32_SFloat
        {GL_RG32F,                                      GL_RG,                      GL_FLOAT,                       capStorage},            // kFormatR32G32_SFloat
        {GL_RGB32F,                                     GL_RGB,                     GL_FLOAT,                       capStorage},            // kFormatR32G32B32_SFloat
        {GL_RGBA32F,                                    GL_RGBA,                    GL_FLOAT,                       capStorage},            // kFormatR32G32B32A32_SFloat

        {GL_LUMINANCE,                                  GL_LUMINANCE,               GL_UNSIGNED_BYTE,               flagAlpha},             // kFormatL8_UNorm
        {internalAlpha8,                                externalAlpha8,             GL_UNSIGNED_BYTE,               flagAlpha},             // kFormatA8_UNorm
        {GL_R16,                                        GL_RED,                     GL_UNSIGNED_SHORT,              capStorage},            // kFormatA16_UNorm

        {internalSRGB8,                                 externalSRGB8,              GL_UNSIGNED_BYTE,               capStorage},            // kFormatB8G8R8_SRGB
        {internalSBGR8_A8,                              externalBGRA8,              GL_UNSIGNED_BYTE,               capStorage},            // kFormatB8G8R8A8_SRGB
        {GL_RGB8,                                       GL_RGB,                     GL_UNSIGNED_BYTE,               capStorage},            // kFormatB8G8R8_UNorm
        {internalBGRA8,                                 externalBGRA8,              GL_UNSIGNED_BYTE,               capStorage},            // kFormatB8G8R8A8_UNorm
        {GL_RGB16_SNORM,                                GL_RGB,                     GL_BYTE,                        capStorage},            // kFormatBGR8_SNorm
        {GL_RGBA16_SNORM,                               GL_RGBA,                    GL_BYTE,                        capStorage},            // kFormatB8G8R8A8_SNorm
        {GL_RGB32UI,                                    GL_RGB_INTEGER,             GL_UNSIGNED_BYTE,               capStorage},            // kFormatBGR8_UInt
        {GL_RGBA32UI,                                   GL_RGBA_INTEGER,            GL_UNSIGNED_BYTE,               capStorage},            // kFormatB8G8R8A8_UInt
        {GL_RGB32I,                                     GL_RGB_INTEGER,             GL_BYTE,                        capStorage},            // kFormatBGR8_SInt
        {GL_RGBA32I,                                    GL_RGBA_INTEGER,            GL_BYTE,                        capStorage},            // kFormatB8G8R8A8_SInt

        {GL_RGBA4,                                      GL_RGBA,                    GL_UNSIGNED_SHORT_4_4_4_4,      capStorage},            // kFormatR4G4B4A4_UNormPack16
        {GL_RGBA4,                                      GL_RGBA,                    GL_UNSIGNED_SHORT_4_4_4_4,      capStorage},            // kFormatB4G4R4A4_UNormPack16
        {GL_RGB565,                                     GL_RGB,                     GL_UNSIGNED_SHORT_5_6_5,        capStorage},            // kFormatR5G6B5_UNormPack16
        {GL_RGB565,                                     GL_RGB,                     GL_UNSIGNED_SHORT_5_6_5,        capStorage},            // kFormatB5G6R5_UNormPack16
        {GL_RGB5_A1,                                    GL_RGBA,                    GL_UNSIGNED_SHORT_5_5_5_1,      capStorage},            // kFormatR5G5B5A1_UNormPack16
        {GL_RGB5_A1,                                    GL_RGBA,                    GL_UNSIGNED_SHORT_5_5_5_1,      capStorage},            // kFormatB5G5R5A1_UNormPack16
        {GL_RGB5_A1,                                    GL_BGRA,                    GL_UNSIGNED_SHORT_5_5_5_1,      capStorage},            // kFormatA1RGB5_UNorm

        {GL_RGB9_E5,                                    GL_RGB,                     GL_UNSIGNED_INT_5_9_9_9_REV,    capStorage},            // kFormatE5B9G9R9_UFloatPack32
        {GL_R11F_G11F_B10F,                             GL_RGB,                     GL_UNSIGNED_INT_10F_11F_11F_REV, capStorage},           // kFormatB10G11R11_UFloatPack32

        {GL_RGB10_A2,                                   GL_RGBA,                    GL_UNSIGNED_INT_2_10_10_10_REV, capStorage},            // kFormatA2B10G10R10_UNormPack32
        {GL_RGB10_A2UI,                                 GL_RGBA,                    GL_UNSIGNED_INT_2_10_10_10_REV, capStorage},            // kFormatA2B10G10R10_UIntPack32
        {GL_NONE,                                       GL_NONE,                    GL_NONE,                        capStorage},            // kFormatA2B10G10R10_SIntPack32
        {GL_RGB10_A2,                                   GL_RGBA,                    GL_UNSIGNED_INT_2_10_10_10_REV, capStorage},            // kFormatA2R10G10B10_UNormPack32
        {GL_RGB10_A2UI,                                 GL_RGBA,                    GL_UNSIGNED_INT_2_10_10_10_REV, capStorage},            // kFormatA2R10G10B10_UIntPack32
        {GL_NONE,                                       GL_NONE,                    GL_NONE,                        capStorage},            // kFormatA2R10G10B10_SIntPack32
        {GL_NONE,                                       GL_NONE,                    GL_NONE,                        capStorage},            // kFormatA2R10G10B10_XRSRGBPack32
        {GL_NONE,                                       GL_NONE,                    GL_NONE,                        capStorage},            // kFormatA2R10G10B10_XRUNormPack32
        {GL_NONE,                                       GL_NONE,                    GL_NONE,                        capStorage},            // kFormatR10G10B10_XRSRGBPack32
        {GL_NONE,                                       GL_NONE,                    GL_NONE,                        capStorage},            // kFormatR10G10B10_XRUNormPack32
        {GL_NONE,                                       GL_NONE,                    GL_NONE,                        capStorage},            // kFormatA10R10G10B10_XRSRGBPack32
        {GL_NONE,                                       GL_NONE,                    GL_NONE,                        capStorage},            // kFormatA10R10G10B10_XRUNormPack32

        {internalSRGB8_A8,                              GL_RGBA,                    GL_UNSIGNED_BYTE,               capStorage},            // kFormatA8R8G8B8_SRGB
        {GL_RGBA8,                                      GL_RGBA,                    GL_UNSIGNED_BYTE,               capStorage},            // kFormatA8R8G8B8_UNorm
        {GL_RGBA32F,                                    GL_RGBA,                    GL_FLOAT,                       capStorage},            // kFormatA32R32G32B32_SFloat

        {internalDepth16,                               GL_DEPTH_COMPONENT,         GL_UNSIGNED_SHORT,              capStorage},            // kFormatD16_UNorm
        {internalDepth24,                               GL_DEPTH_COMPONENT,         typeDepth24,                    capStorage},            // kFormatD24_UNorm
        {internalDepthStencil,                          externalDepthStencil,       typeDepthStencil,               capStorage},            // kFormatD24_UNorm_S8_UInt
        {GL_DEPTH_COMPONENT32F,                         GL_DEPTH_COMPONENT,         GL_FLOAT,                       capStorage},            // kFormatD32_SFloat
        {GL_DEPTH32F_STENCIL8,                          GL_DEPTH_STENCIL,           GL_FLOAT_32_UNSIGNED_INT_24_8_REV, capStorage},         // kFormatD32_SFloat_S8_Uint
        {GL_STENCIL_INDEX8,                             GL_STENCIL_INDEX,           GL_UNSIGNED_BYTE,               capStorage},            // kFormatS8_Uint

        {GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT,        GL_NONE,                    GL_NONE,                        capDXT},                // kFormatRGBA_DXT1_SRGB
        {GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,              GL_NONE,                    GL_NONE,                        capDXT},                // kFormatRGBA_DXT1_UNorm
        {GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT,        GL_NONE,                    GL_NONE,                        capDXT},                // kFormatRGBA_DXT3_SRGB
        {GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,              GL_NONE,                    GL_NONE,                        capDXT},                // kFormatRGBA_DXT3_UNorm
        {GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT,        GL_NONE,                    GL_NONE,                        capDXT},                // kFormatRGBA_DXT5_SRGB
        {GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,              GL_NONE,                    GL_NONE,                        capDXT},                // kFormatRGBA_DXT5_UNorm
        {GL_COMPRESSED_RED_RGTC1,                       GL_NONE,                    GL_NONE,                        capStorage},            // kFormatR_BC4_UNorm
        {GL_COMPRESSED_SIGNED_RED_RGTC1,                GL_NONE,                    GL_NONE,                        capStorage},            // kFormatR_BC4_SNorm
        {GL_COMPRESSED_RG_RGTC2,                        GL_NONE,                    GL_NONE,                        capStorage},            // kFormatRG_BC5_UNorm
        {GL_COMPRESSED_SIGNED_RG_RGTC2,                 GL_NONE,                    GL_NONE,                        capStorage},            // kFormatRG_BC5_SNorm
        {GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT,         GL_NONE,                    GL_NONE,                        capStorage},            // kFormatRGB_BC6H_UFloat
        {GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT,           GL_NONE,                    GL_NONE,                        capStorage},            // kFormatRGB_BC6H_SFloat
        {GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM,           GL_NONE,                    GL_NONE,                        capStorage},            // kFormatRGBA_BC7_SRGB
        {GL_COMPRESSED_RGBA_BPTC_UNORM,                 GL_NONE,                    GL_NONE,                        capStorage},            // kFormatRGBA_BC7_UNorm
        {GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT,           GL_NONE,                    GL_NONE,                        capStorage},            // kFormatRGB_PVRTC_2Bpp_SRGB
        {GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG,            GL_NONE,                    GL_NONE,                        capStorage},            // kFormatRGB_PVRTC_2Bpp_UNorm
        {GL_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT,           GL_NONE,                    GL_NONE,                        capStorage},            // kFormatRGB_PVRTC_4Bpp_SRGB
        {GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG,            GL_NONE,                    GL_NONE,                        capStorage},            // kFormatRGB_PVRTC_4Bpp_UNorm
        {GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT,     GL_NONE,                    GL_NONE,                        capStorage},            // kFormatRGBA_PVRTC_2Bpp_SRGB
        {GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG,           GL_NONE,                    GL_NONE,                        capStorage},            // kFormatRGBA_PVRTC_2Bpp_UNorm
        {GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT,     GL_NONE,                    GL_NONE,                        capStorage},            // kFormatRGBA_PVRTC_4Bpp_SRGB
        {GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG,           GL_NONE,                    GL_NONE,                        capStorage},            // kFormatRGBA_PVRTC_4Bpp_UNorm
        {internalETC1,                                  GL_NONE,                    GL_NONE,                        capStorage},            // kFormatRGB_ETC_UNorm
        {GL_COMPRESSED_SRGB8_ETC2,                      GL_NONE,                    GL_NONE,                        capStorage},            // kFormatRGB_ETC2_SRGB
        {GL_COMPRESSED_RGB8_ETC2,                       GL_NONE,                    GL_NONE,                        capStorage},            // kFormatRGB_ETC2_UNorm
        {GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2,  GL_NONE,                    GL_NONE,                        capStorage},            // kFormatRGB_A1_ETC2_SRGB
        {GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2,   GL_NONE,                    GL_NONE,                        capStorage},            // kFormatRGB_A1_ETC2_UNorm
        {GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC,           GL_NONE,                    GL_NONE,                        capStorage},            // kFormatRGBA_ETC2_SRGB
        {GL_COMPRESSED_RGBA8_ETC2_EAC,                  GL_NONE,                    GL_NONE,                        capStorage},            // kFormatRGBA_ETC2_UNorm
        {GL_COMPRESSED_R11_EAC,                         GL_NONE,                    GL_NONE,                        capStorage},            // kFormatR_EAC_UNorm
        {GL_COMPRESSED_SIGNED_R11_EAC,                  GL_NONE,                    GL_NONE,                        capStorage},            // kFormatR_EAC_SNorm
        {GL_COMPRESSED_RG11_EAC,                        GL_NONE,                    GL_NONE,                        capStorage},            // kFormatRG_EAC_UNorm
        {GL_COMPRESSED_SIGNED_RG11_EAC,                 GL_NONE,                    GL_NONE,                        capStorage},            // kFormatRG_EAC_SNorm
        {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4,           GL_NONE,                    GL_NONE,                        capStorage},            // kFormatRGBA_ASTC4X4_SRGB
        {GL_COMPRESSED_RGBA_ASTC_4x4,                   GL_NONE,                    GL_NONE,                        capStorage},            // kFormatRGBA_ASTC4X4_UNorm
        {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5,           GL_NONE,                    GL_NONE,                        capStorage},            // kFormatRGBA_ASTC5X5_SRGB
        {GL_COMPRESSED_RGBA_ASTC_5x5,                   GL_NONE,                    GL_NONE,                        capStorage},            // kFormatRGBA_ASTC5X5_UNorm
        {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6,           GL_NONE,                    GL_NONE,                        capStorage},            // kFormatRGBA_ASTC6X6_SRGB
        {GL_COMPRESSED_RGBA_ASTC_6x6,                   GL_NONE,                    GL_NONE,                        capStorage},            // kFormatRGBA_ASTC6X6_UNorm
        {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8,           GL_NONE,                    GL_NONE,                        capStorage},            // kFormatRGBA_ASTC8X8_SRGB
        {GL_COMPRESSED_RGBA_ASTC_8x8,                   GL_NONE,                    GL_NONE,                        capStorage},            // kFormatRGBA_ASTC8X8_UNorm
        {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10,         GL_NONE,                    GL_NONE,                        capStorage},            // kFormatRGBA_ASTC10X10_SRGB
        {GL_COMPRESSED_RGBA_ASTC_10x10,                 GL_NONE,                    GL_NONE,                        capStorage},            // kFormatRGBA_ASTC10X10_UNorm
        {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12,         GL_NONE,                    GL_NONE,                        capStorage},            // kFormatRGBA_ASTC12X12_SRGB
        {GL_COMPRESSED_RGBA_ASTC_12x12,                 GL_NONE,                    GL_NONE,                        capStorage},            // kFormatRGBA_ASTC12X12_UNorm

        {GL_NONE,                                       GL_NONE,                    GL_NONE,                        capStorage},            // kFormatYUV2

#if PLATFORM_WEBGL
        {GL_RGBA4,                                      GL_RGBA,                    GL_UNSIGNED_SHORT_4_4_4_4,      capStorage},            // kFormatDepthAuto is kFormatR4G4B4A4_UNormPack16 on WebGL and only used on WebGL
#else
        {internalDepthStencil,                          externalDepthStencil,       typeDepthStencil,               capStorage},            // kFormatDepthAuto is kFormatD24_UNorm_S8_UInt
#endif
        {GL_RGBA4,                                      GL_RGBA,                    GL_UNSIGNED_SHORT_4_4_4_4,      capStorage},            // kFormatShadowAuto is kFormatR4G4B4A4_UNormPack16 on WebGL and only used on WebGL
        {GL_NONE,                                       GL_NONE,                    GL_NONE,                        capStorage},            // kFormatVideoAuto
        {GL_COMPRESSED_RGBA_ASTC_4x4,                   GL_NONE,                    GL_NONE,                        capStorage},            // kFormatRGBA_ASTC4X4_UFloat
        {GL_COMPRESSED_RGBA_ASTC_5x5,                   GL_NONE,                    GL_NONE,                        capStorage},            // kFormatRGBA_ASTC5x5_UFloat
        {GL_COMPRESSED_RGBA_ASTC_6x6,                   GL_NONE,                    GL_NONE,                        capStorage},            // kFormatRGBA_ASTC6x6_UFloat
        {GL_COMPRESSED_RGBA_ASTC_8x8,                   GL_NONE,                    GL_NONE,                        capStorage},            // kFormatRGBA_ASTC8x8_UFloat
        {GL_COMPRESSED_RGBA_ASTC_10x10,                 GL_NONE,                    GL_NONE,                        capStorage},            // kFormatRGBA_ASTC10x10_UFloat
        {GL_COMPRESSED_RGBA_ASTC_12x12,                 GL_NONE,                    GL_NONE,                        capStorage},            // kFormatRGBA_ASTC12x12_UFloat
    };
    CompileTimeAssertArraySize(table, kGraphicsFormatCount);

    std::copy(&table[0], &table[0] + ARRAY_SIZE(table), m_FormatGLES.begin());
}

const FormatDescGLES& TranslateGLES::GetFormatDesc(GraphicsFormat format, bool assertOnAutoFormats) const
{
    AssertFormatMsg(format >= kFormatFirst && format <= kFormatLast, "OPENGL ERROR: TranslateGLES::GetFormatDesc - Invalid input: %d", format);

    if (assertOnAutoFormats)
    {
        Assert(format != kFormatDepthAuto);
        Assert(format != kFormatShadowAuto);
    }

    return this->m_FormatGLES[format];
}

void TranslateGLES::InitVertexType(const GraphicsCaps & caps, GfxDeviceLevelGL level)
{
    const GLenum halfFloatType = IsGfxLevelES2(level) ? GL_HALF_FLOAT_OES : GL_HALF_FLOAT;
    const gl::VertexArrayFlags integerFlag = (/*PLATFORM_WEBGL ||*/ IsGfxLevelES2(level)) ?
        gl::kVertexArrayFlagNone : gl::kVertexArrayFlagInteger; // glVertexAttribIPointer unsupported in ES 2.0 and WebGL

    const VertexFormatDescGLES translation[] =
    {
        { GL_FLOAT,             gl::kVertexArrayFlagNone        },  // kVertexFormatFloat
        { halfFloatType,        gl::kVertexArrayFlagNone        },  // kVertexFormatFloat16
        { GL_UNSIGNED_BYTE,     gl::kVertexArrayFlagNormalized  },  // kVertexFormatUNorm8
        { GL_BYTE,              gl::kVertexArrayFlagNormalized  },  // kVertexFormatSNorm8
        { GL_UNSIGNED_SHORT,    gl::kVertexArrayFlagNormalized  },  // kVertexFormatUNorm16
        { GL_SHORT,             gl::kVertexArrayFlagNormalized  },  // kVertexFormatSNorm16
        { GL_UNSIGNED_BYTE,     integerFlag                     },  // kVertexFormatUInt8
        { GL_BYTE,              integerFlag                     },  // kVertexFormatSInt8
        { GL_UNSIGNED_SHORT,    integerFlag                     },  // kVertexFormatUInt16
        { GL_SHORT,             integerFlag                     },  // kVertexFormatSInt16
        { GL_UNSIGNED_INT,      integerFlag                     },  // kVertexFormatUInt32
        { GL_INT,               integerFlag                     },  // kVertexFormatSInt32
    };
    CompileTimeAssertArraySize(translation, kVertexFormatCount);

    std::copy(&translation[0], &translation[0] + ARRAY_SIZE(translation), m_VertexFormat.begin());
}

VertexFormatDescGLES TranslateGLES::GetVertexFormatDesc(VertexFormat format) const
{
    DebugAssertMsg(format < kVertexFormatCount, "OPENGL ERROR: TranslateGLES::VertexType - Invalid input");

    return m_VertexFormat[format];
}

void TranslateGLES::InitObjectType(const GraphicsCaps & caps)
{
    const GLenum translation[] =
    {
        (GLenum)(caps.gles.hasDebug ? GL_BUFFER : GL_BUFFER_OBJECT_EXT),                          // kBuffer
        (GLenum)(caps.gles.hasDebug ? GL_SHADER : GL_SHADER_OBJECT_EXT),                          // kShader
        (GLenum)(caps.gles.hasDebug ? GL_PROGRAM : GL_PROGRAM_OBJECT_EXT),                        // kProgram
        (GLenum)(caps.gles.hasDebug ? GL_VERTEX_ARRAY : GL_VERTEX_ARRAY_OBJECT_EXT),              // kVertexArray
        (GLenum)(caps.gles.hasDebug ? GL_QUERY : GL_QUERY_OBJECT_EXT),                            // kQuery
        GL_TRANSFORM_FEEDBACK,                                                          // kTransformFeedback
        GL_SAMPLER,                                                                     // kSampler
        GL_TEXTURE,                                                                     // kTexture
        GL_RENDERBUFFER,                                                                // kRenderbuffer
        GL_FRAMEBUFFER                                                                  // kFramebuffer
    };
    CompileTimeAssertArraySize(translation, gl::kObjectTypeCount);

    std::copy(&translation[0], &translation[0] + ARRAY_SIZE(translation), m_ObjectType.begin());
}

GLenum TranslateGLES::ObjectType(gl::ObjectType type) const
{
    DebugAssertMsg(type >= gl::kObjectTypeFirst && type <= gl::kObjectTypeLast, "OPENGL ERROR: Unsupported gl::ObjectType input format translation");
    return m_ObjectType[type];
}

void TranslateGLES::InitFramebufferTarget(const GraphicsCaps & caps)
{
    const GLenum translation[] = // gl::kFramebufferTargetCount
    {
        (GLenum)(caps.gles.hasReadDrawFramebuffer ? GL_DRAW_FRAMEBUFFER : GL_FRAMEBUFFER),    // kDrawFramebuffer
        (GLenum)(caps.gles.hasReadDrawFramebuffer ? GL_READ_FRAMEBUFFER : GL_FRAMEBUFFER)     // kReadFramebuffer
    };
    CompileTimeAssertArraySize(translation, gl::kFramebufferTargetCount);

    std::copy(&translation[0], &translation[0] + ARRAY_SIZE(translation), m_FramebufferTarget.begin());
}

GLenum TranslateGLES::FramebufferTarget(gl::FramebufferTarget target) const
{
    Assert(target >= gl::kFramebufferTargetFirst && target <= gl::kFramebufferTargetLast);
    return m_FramebufferTarget[target];
}

namespace gl
{
    const LevelDesc& GetLevelDesc(GfxDeviceLevelGL level)
    {
        Assert(level != kGfxLevelUninitialized);

        static const LevelDesc table[] =
        {
            { 2, 0, kShaderRequireShaderModel30 },          // kGfxLevelES2
            { 3, 0, kShaderRequireShaderModel35_ES3 },      // kGfxLevelES3
            { 3, 1, kShaderRequireShaderModel45_ES31 },     // kGfxLevelES31
            { 3, 1, kShaderRequireShaderModel45_ES31 },     // kGfxLevelES31AEP
            { 3, 2, kShaderRequireShaderModel45_ES31 },     // kGfxLevelES32
            { 3, 2, kShaderRequireShaderModel40_PC },       // kGfxLevelCore32
            { 3, 3, kShaderRequireShaderModel40_PC },       // kGfxLevelCore33
            { 4, 0, kShaderRequireShaderModel40_PC },       // kGfxLevelCore40
            { 4, 1, kShaderRequireShaderModel46_GL41 | kShaderRequireInterpolators32 | kShaderRequireMRT8 | kShaderRequireCubeArray }, // kGfxLevelCore41
            { 4, 2, kShaderRequireShaderModel46_GL41 | kShaderRequireInterpolators32 | kShaderRequireMRT8 | kShaderRequireCubeArray }, // kGfxLevelCore42
            { 4, 3, kShaderRequireShaderModel50_PC },       // kGfxLevelCore43
            { 4, 4, kShaderRequireShaderModel50_PC },       // kGfxLevelCore44
            { 4, 5, kShaderRequireShaderModel50_PC },       // kGfxLevelCore45
        };
        CompileTimeAssertArraySize(table, kGfxLevelCount);

        return table[level - kGfxLevelFirst];
    }

    GfxDeviceLevelGL GetDeviceLevel(int major, int minor, bool isES)
    {
        for (int i = isES ? kGfxLevelESFirst : kGfxLevelCoreFirst, n = isES ? kGfxLevelESLast : kGfxLevelCoreLast; i <= n; ++i)
        {
            const GfxDeviceLevelGL level = static_cast<GfxDeviceLevelGL>(i);
            if (GetLevelDesc(level).major == major && GetLevelDesc(level).minor == minor)
                return level;
        }
        return kGfxLevelUninitialized;
    }
}//namespace gl
