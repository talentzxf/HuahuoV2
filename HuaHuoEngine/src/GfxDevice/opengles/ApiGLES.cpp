//
// Created by VincentZhang on 5/25/2022.
//

#include "ApiGLES.h"
#include "ExtensionsGLES.h"
#include "ApiEnumGLES.h"
#include "AssertGLES.h"
#include "Utilities/Word.h"
#include "ApiConstantsGLES.h"
#include "ApiTranslateGLES.h"

namespace gl
{
    ContextHandle GetCurrentContext();
}//namespace gl

ApiGLES * gGL = NULL;

ApiGLES::ApiGLES()
        : ApiFuncGLES()
        , m_Translate(new TranslateGLES)
//        , m_Debug(new DebugGLES(*this))
        , translate(*m_Translate)
//        , debug(*m_Debug)
//        , m_CurrentProgramBinding(0)
//        , m_CurrentProgramHasTessellation(false)
//        , m_TestTextureTargetFB(gl::FramebufferHandle::Invalid())
//        , m_CurrentVertexArrayBinding()
//        , m_DefaultVertexArrayName()
//        , m_CurrentDefaultVertexArrayEnabled(0)
//        , m_CurrentCullMode(kCullOff)
//        , m_CurrentPatchVertices(0)
//        , m_CurrentCapEnabled(0)
//        , m_CurrentPolygonModeWire(false)
        , m_CurrentTextureUnit(0)
//        , m_CurrentStencilState(nullptr)
//        , m_CurrentStencilRef(0)
//        , m_EnabledClipPlanes(0)
        , m_Context(gl::ContextHandle::Invalid())
#   if SUPPORT_THREADS
        , m_Thread(CurrentThread::GetID())
#   endif
        , m_Caching(false)
{
    this->m_CurrentTextureBindings.fill(0);
    this->m_CurrentTextureTargets.fill(GL_NONE);
    this->m_CurrentSamplerBindings.fill(0);
    this->m_CurrentFramebufferBindings.fill(gl::FramebufferHandle());

    this->m_CurrentBufferBindings.fill(0);
    this->m_CurrentUniformBufferBindings.fill(UBOBinding(0));
    this->m_CurrentTransformBufferBindings.fill(0);
    this->m_CurrentStorageBufferBindings.fill(0);
    this->m_CurrentAtomicCounterBufferBindings.fill(0);

    this->m_TestTextureTargetFunc.fill(&ApiGLES::InitGetTextureTargetFunc);

#   if !UNITY_RELEASE
    printf_console("OPENGL LOG: Created ApiGLES instance for context %d\n", m_Context.Get());
#   endif
}

ApiGLES::~ApiGLES()
{
    delete m_Translate;
    m_Translate = NULL;

//    delete m_Debug;
//    m_Debug = NULL;
}

void ApiGLES::InitDebug()
{
//    if (!GetGraphicsCaps().gles.hasDebug)
//        return;
//
//#   if !UNITY_RELEASE
//    Assert(this->m_Debug);
//    this->m_Debug->Init(*this);
//#   endif
}

bool ApiGLES::QueryExtensionSlow(const char * extension) const
{
    GLES_CHECK(this, -1);

//    if (HasARGV("no-extensions"))
//        return false;

    if (IsGfxLevelES2(GetGraphicsCaps().gles.featureLevel))
    {
        const char* extensions = reinterpret_cast<const char*>(this->glGetString(GL_EXTENSIONS));
        if (!extensions)
            return false;

        const char* match = strstr(extensions, extension);
        if (!match)
            return false;
        // we need an exact match, extensions string is a list of extensions separated by spaces, e.g. "GL_EXT_draw_buffers" should not match "GL_EXT_draw_buffers_indexed"
        const char* end = match + strlen(extension);
        return *end == ' ' || *end == '\0';
    }
    else
    {
        const GLint numExtensions = this->Get(GL_NUM_EXTENSIONS);
        for (GLint i = 0; i < numExtensions; ++i)
        {
            const char* Extension = reinterpret_cast<const char*>(this->glGetStringi(GL_EXTENSIONS, i));
            if (!strcmp(extension, Extension))
                return true;
        }
        return false;
    }
}


//GLenum ApiGLES::InitGetTextureTargetFunc(TextureDimension textureDimension, GLuint texture)
//{
//    m_TestTextureTargetFunc[textureDimension - kTexDimFirst] = &ApiGLES::GetTextureTargetDefault;
//
//    const int nbTargets = translate.GetTextureTargetCount(textureDimension);
//    if (nbTargets > 1)
//    {
//        // Create a texture of each known texture target type for the given dimension.
//        const GLuint restoreTextureName = m_CurrentTextureBindings[m_CurrentTextureUnit];
//        const GLenum restoreTextureTarget = m_CurrentTextureTargets[m_CurrentTextureUnit];
//        dynamic_array<GLuint> textures(nbTargets, kMemTempAlloc);
//        for (int i = 0; i < nbTargets; ++i)
//        {
//            const GLenum target = translate.GetTextureTarget(textureDimension, i);
//            bool immutable = false;
//            // External textures cannot be created by Unity, so we just create a texture name for it
//            // as we cannot ask our CreateTexture to create the storage for it. Still good enough
//            // for the target discovery tests.
//            textures[i] = target == GL_TEXTURE_EXTERNAL_OES ?
//                          GenTexture(target) : CreateTexture(target, kFormatR8G8B8_UNorm, 1, 1, 1, 1, 1, immutable);
//            // Note that a texture must be bound to finalize its type.
//            BindTexture(textures[i], target);
//        }
//
//        // For each possible function to test, check its ability to properly identify all known texture types.
//        const GetTextureTargetFunc testFuncs[] =
//                {
//                        &ApiGLES::GetTextureTargetViaDirectQuery,
//                        &ApiGLES::GetTextureTargetViaSuccessfulBinding,
//                        &ApiGLES::GetTextureTargetViaFrameBufferStatus
//                };
//        for (int i = 0; i < ARRAY_SIZE(testFuncs); ++i)
//        {
//            bool failed = false;
//            for (int j = 0; j < nbTargets && !failed; ++j)
//                failed = ((this->*testFuncs[i])(textureDimension, textures[j]) != translate.GetTextureTarget(textureDimension, j));
//            if (!failed)
//            {
//                m_TestTextureTargetFunc[textureDimension - kTexDimFirst] = testFuncs[i];
//                break;
//            }
//        }
//
//        for (int i = 0, e = nbTargets; i < e; ++i)
//        {
//            const GLenum target = translate.GetTextureTarget(textureDimension, i);
//            // External textures cannot be created by Unity, so we'll also delete them with the
//            // low-level API.
//            if (target == GL_TEXTURE_EXTERNAL_OES)
//                GLES_CALL(this, glDeleteTextures, 1, &(textures[i]));
//            else
//                DeleteTexture(textures[i]);
//        }
//
//        BindTexture(restoreTextureName, restoreTextureTarget);
//    }
//
//    return GetTextureTarget(textureDimension, texture);
//}

const char* ApiGLES::GetDriverString(gl::DriverQuery Query) const
{
    // GLES_CHECK(this, -1);

    const GLenum translatedQuery = GL_VENDOR + Query; // GL_VENDOR, GL_RENDERER and GL_VERSION are contiguous values, query is a zero based enum
    const GLubyte* stringPointer = 0;
    GLES_CALL_RET(this, stringPointer, glGetString, translatedQuery);

    return reinterpret_cast<const char*>(stringPointer);
}

gl::ContextHandle ApiGLES::GetContext() const
{
    DebugAssert(this->m_Context == gl::GetCurrentContext());
    return this->m_Context;
}

gl::VertexArrayHandle ApiGLES::CreateVertexArray()
{
    // FIXME: Enable this as soon as it's passing
    // GLES_CHECK(this, 0);
    GLES_ASSERT(this, GetGraphicsCaps().gles.hasVertexArrayObject, "Vertex array objects are not supported");

    GLuint vertexArrayName = 0;
    GLES_CALL(this, glGenVertexArrays, 1, &vertexArrayName);

    return gl::VertexArrayHandle(this->GetContext(), vertexArrayName);
}

GLint ApiGLES::Get(GLenum cap) const
{
    GLint result = 0;
    GLES_CALL(this, glGetIntegerv, cap, &result);
    return result;
}

void ApiGLES::FillExtensions(std::vector<std::string>& allExtensions)
{
//    GLES_CHECK(this, -1);
//
//    if (HasARGV("no-extensions"))
//        return;

    bool useLegacyExtensionString = false;
    if (IsGfxLevelES2(GetGraphicsCaps().gles.featureLevel))
    {
        useLegacyExtensionString = true;
    }
    else if (IsGfxLevelES(GetGraphicsCaps().gles.featureLevel))
    {
        // Early Adreno ES 3.0 drivers return the same pointer on every call to glGetStringi
        // So we cannot use it with string_ref
        // Note: GraphicsCaps are not initialized yet...
        const char* renderer = GetDriverString(gl::kDriverQueryRenderer);
        useLegacyExtensionString = BeginsWith(renderer, "Adreno (TM) 3");
    }

    if (useLegacyExtensionString)
    {
        const GLubyte* string = nullptr;
        GLES_CALL_RET(this, string, glGetString, GL_EXTENSIONS);
        std::string allExtensionString(reinterpret_cast<const char*>(string));
        core::Split(allExtensionString, ' ', allExtensions);
    }
    else
    {
        const GLint numExtensions = this->Get(GL_NUM_EXTENSIONS);
        allExtensions.reserve(numExtensions);
        for (GLint i = 0; i < numExtensions; ++i)
        {
            const GLubyte* string = 0;
            GLES_CALL_RET(this, string, glGetStringi, GL_EXTENSIONS, i);
            std::string ext(reinterpret_cast<const char*>(string));
            allExtensions.push_back(ext);
        }
    }
}

void ApiGLES::QuerySampleCounts(GLenum target, GLenum internalFormat, std::vector<GLint>& samples) const
{
    // Note: WebGL 2.0 doesn't support GL_NUM_SAMPLE_COUNTS with glGetInternalformativ
    const GLint invalidSampleCount = -1;
    const size_t maxSampleCount = 8; // large enough so that all supported sample counts will definitely fit
    samples.resize(maxSampleCount);
    std::fill_n(samples.begin(), maxSampleCount, invalidSampleCount);
    GLES_CALL(this, glGetInternalformativ, target, internalFormat, GL_SAMPLES, samples.size(), samples.data());
    samples.erase(std::remove(samples.begin(), samples.end(), invalidSampleCount), samples.end());
}

void ApiGLES::ActiveTextureUnit(GLenum unit)
{
    GLES_CHECK(this, unit);
    GLES_ASSERT(this, unit < GetGraphicsCaps().maxTextureBinds, "Trying to bind a texture to a unit not available");
    GLES_ASSERT(this, this->debug.ActiveTexture(), "The context has been modified outside of ApiGLES. States tracking is lost.");

    if (m_Caching && this->m_CurrentTextureUnit == unit)
        return;

    GLES_CALL(this, glActiveTexture, GL_TEXTURE0 + unit);
    this->m_CurrentTextureUnit = unit;
}

void ApiGLES::BindTexture(GLuint textureName, GLenum textureTarget)
{
    GLES_CHECK(this, textureName);
    GLES_ASSERT(this, this->debug.ActiveTexture(), "The context has been modified outside of ApiGLES. States tracking is lost.");

    if (m_Caching && this->m_CurrentTextureBindings[this->m_CurrentTextureUnit] == textureName)
        return;

    GLES_CALL(this, glBindTexture, textureTarget, textureName);

    m_CurrentTextureBindings[m_CurrentTextureUnit] = textureName;
    m_CurrentTextureTargets[m_CurrentTextureUnit] = textureTarget;
}

void ApiGLES::BindTexture(GLenum unit, GLuint textureName, GLenum textureTarget)
{
    GLES_CHECK(this, textureName);

    this->ActiveTextureUnit(unit);
    this->BindTexture(textureName, textureTarget);
}

void ApiGLES::GenerateMipmap(GLuint textureName, GLenum textureTarget)
{
    GLES_CHECK(this, textureName);

    if (textureTarget == GL_TEXTURE_EXTERNAL_OES)
    {
        // OES_EGL_image_external does not support mipmaps.
        return;
    }

    const GLuint prevTexture = m_CurrentTextureBindings[m_CurrentTextureUnit];
    const GLenum prevTarget = m_CurrentTextureTargets[m_CurrentTextureUnit];

    this->BindTexture(textureName, textureTarget);
    GLES_CALL(this, glGenerateMipmap, textureTarget);
    this->BindTexture(prevTexture, prevTarget);
}

FramebufferInfoGLES ApiGLES::GetFramebufferInfo() const
{
    const GraphicsCaps& caps = GetGraphicsCaps();
    GLES_CHECK(this, -1);
    GLES_ASSERT(this, !IsGfxLevelCore(caps.gles.featureLevel), "Core profile doesn't support context size queries");

    FramebufferInfoGLES info = {};

    GLES_CALL(this, glGetIntegerv, GL_RED_BITS, &info.redBits);
    GLES_CALL(this, glGetIntegerv, GL_GREEN_BITS, &info.greenBits);
    GLES_CALL(this, glGetIntegerv, GL_BLUE_BITS, &info.blueBits);
    GLES_CALL(this, glGetIntegerv, GL_ALPHA_BITS, &info.alphaBits);
    GLES_CALL(this, glGetIntegerv, GL_DEPTH_BITS, &info.depthBits);
    GLES_CALL(this, glGetIntegerv, GL_STENCIL_BITS, &info.stencilBits);
    if (caps.hasMultiSample)
    {
        GLES_CALL(this, glGetIntegerv, GL_SAMPLES, &info.samples);
        GLES_CALL(this, glGetIntegerv, GL_SAMPLE_BUFFERS, &info.sampleBuffers);
    }
    if (caps.gles.hasNVCSAA)
    {
        GLES_CALL(this, glGetIntegerv, GL_COVERAGE_SAMPLES_NV, &info.coverageSamples);
        GLES_CALL(this, glGetIntegerv, GL_COVERAGE_BUFFERS_NV, &info.coverageBuffers);
    }

    return info;
}


void ApiGLES::Init(const GfxContextGLES& context, GfxDeviceLevelGL &deviceLevel)
{
    GraphicsCaps& caps = GetGraphicsCaps();

    Assert(IsGfxLevelCore(deviceLevel) || IsGfxLevelES(deviceLevel));

    m_Context = gl::GetCurrentContext();

    // Set the current API
    gGL = this;

    // Caps are init by InitCaps but we need to initialize the featureLevel before calling Load which load OpenGL functions because it checks for extensions using the featureLevel
    Assert(caps.gles.featureLevel == kGfxLevelUninitialized);
    caps.gles.featureLevel = deviceLevel;

    // The order of these functions matters

    LoadExtensionQuerying(deviceLevel);
#if PLATFORM_ANDROID
    if (deviceLevel == kGfxLevelES31AEP)
    {
        if (!::QueryExtensionSlow(GLExt::kGL_ANDROID_extension_pack_es31a))
        {
            deviceLevel = kGfxLevelES31;
            caps.gles.featureLevel = deviceLevel;
        }
    }
#endif
    std::vector<std::string> allExtensions; //(kMemTempAlloc);
    FillExtensions(allExtensions);

    InitializeExtensions(allExtensions);

    // Load the OpenGL API pointers
    Load(deviceLevel);

    // Initialize the capabilities supported by the current platform
    gles::InitCaps(this, &caps, deviceLevel, allExtensions);

//    // Initialize the translation to OpenGL enums depending on the platform capability
//    Assert(this->m_Translate);
//    this->m_Translate->Init(caps, deviceLevel);
//
//    gles::InitRenderTextureAACaps(this, &caps);
//
//    // Now that the translations are ready, properly set the default for texture unit texture targets.
//    this->m_CurrentTextureTargets.fill(gl::GetTextureTarget(kTexDim2D));
//
//#   if PLATFORM_ANDROID
//    UpdateTextureFormatSupportETC2(this, deviceLevel);
//#   endif
//
//    // Initialize OpenGL debugging functionalities
//#   if !UNITY_RELEASE
//    this->InitDebug();
//#   endif
}