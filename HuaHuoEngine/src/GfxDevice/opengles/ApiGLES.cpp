//
// Created by VincentZhang on 5/25/2022.
//

#include "ApiGLES.h"
#include "ExtensionsGLES.h"
#include "ApiEnumGLES.h"
#include "AssertGLES.h"
#include "Utilities/Word.h"
#include "GLES3/gl3.h"

namespace gl
{
    ContextHandle GetCurrentContext();
}//namespace gl

ApiGLES * gGL = NULL;

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