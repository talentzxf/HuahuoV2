//
// Created by VincentZhang on 5/25/2022.
//

#ifndef HUAHUOENGINE_APIGLES_H
#define HUAHUOENGINE_APIGLES_H
#include "Utilities/NonCopyable.h"
#include "ApiFuncGLES.h"
#include "Shaders/GraphicsCaps.h"
#include "GfxDevice/GfxDeviceTypes.h"
#include "HandleContextGLES.h"
#include "ApiEnumGLES.h"
#include "HandleObjectGLES.h"

class TranslateGLES;
class GfxContextGLES;

struct FramebufferInfoGLES
{
    GLint redBits;
    GLint greenBits;
    GLint blueBits;
    GLint alphaBits;
    GLint depthBits;
    GLint stencilBits;
    GLint samples;
    GLint sampleBuffers;
    GLint coverageSamples;
    GLint coverageBuffers;
};

class ApiGLES: public ApiFuncGLES, private NonCopyable
{
public:
    ApiGLES();
    ~ApiGLES();

    void Init(const GfxContextGLES& context, GfxDeviceLevelGL &deviceLevel);
    // Return the OpenGL strings
    const char* GetDriverString(gl::DriverQuery query) const;

    // Query OpenGL capabilities
    GLint Get(GLenum cap) const;
    GLint Get(GLenum cap, GLuint index) const;

    // -- Vertex Array API --

    gl::VertexArrayHandle CreateVertexArray();
    void DeleteVertexArray(gl::VertexArrayHandle& vertexArrayName);

    void BindVertexArray(gl::VertexArrayHandle vertexArrayName);
    bool IsVertexArray(gl::VertexArrayHandle vertexArrayName);

    FramebufferInfoGLES GetFramebufferInfo() const;

    // Return the current tracked context
    gl::ContextHandle GetContext() const;

    // Conversions from Unity enums to OpenGL enums
    const TranslateGLES & translate;

    // Fills samples with a descending list of sample counts that are support for the given internFormat and target
    void QuerySampleCounts(GLenum target, GLenum internalFormat, std::vector<GLint>& samples) const;

    void GenerateMipmap(GLuint texture, GLenum textureTarget);

    GLenum InitGetTextureTargetFunc(TextureDimension textureDimension, GLuint texture);

    void ActiveTextureUnit(GLenum unit);
    void BindTexture(GLuint texture, GLenum textureTarget);

private:
    void BindTexture(GLenum unit, GLuint texture, GLenum textureTarget);
    TranslateGLES* m_Translate;

    typedef GLenum (ApiGLES::*GetTextureTargetFunc)(TextureDimension textureDimension, GLuint texture);
    struct UBOBinding
    {
        GLuint name;
        GLintptr offset;
        GLsizeiptr size;

        UBOBinding() :
                name(0xffffffff), offset(0), size(0)
        {}

        explicit UBOBinding(GLuint _name) :
                name(_name), offset(0), size(0)
        {}

        UBOBinding(GLuint _name, GLintptr _offset, GLsizeiptr _size) :
                name(_name), offset(_offset), size(_size)
        {}

        bool operator==(const UBOBinding& other) const
        {
            return name == other.name && offset == other.offset && size == other.size;
        }
    };

    // -- All initialization time code --

    gl::ContextHandle                               m_Context;

    fixed_array<GLuint, gl::kBufferTargetSingleBindingCount>        m_CurrentBufferBindings; // Store the state caching for all the buffer bindings that have only a single binding per target.

    GLuint                                                  m_CurrentTextureUnit;
    fixed_array<GLuint, gl::kMaxTextureBindings>            m_CurrentTextureBindings;
    fixed_array<GLenum, gl::kMaxTextureBindings>            m_CurrentTextureTargets;

    fixed_array<UBOBinding, gl::kMaxUniformBufferBindings>          m_CurrentUniformBufferBindings;
    fixed_array<GLuint, gl::kMaxTransformFeedbackBufferBindings>    m_CurrentTransformBufferBindings;
    fixed_array<GLuint, gl::kMaxShaderStorageBufferBindings>        m_CurrentStorageBufferBindings;
    fixed_array<GLuint, gl::kMaxAtomicCounterBufferBindings>        m_CurrentAtomicCounterBufferBindings;

    fixed_array<gl::FramebufferHandle, gl::kFramebufferTargetCount> m_CurrentFramebufferBindings;

    fixed_array<GetTextureTargetFunc, gl::kTexDimActiveCountGL> m_TestTextureTargetFunc;

    // -- Sampler --

    fixed_array<GLuint, gl::kMaxTextureBindings>    m_CurrentSamplerBindings;

    bool                                            m_Caching;

    void Load(GfxDeviceLevelGL contextLevel);
    void LoadExtensionQuerying(GfxDeviceLevelGL level);
    void FillExtensions(std::vector<std::string>& allExtensions);
};

// This is a simple counter type for tracking order of resource accesses that need memory barriers.
typedef unsigned long long BarrierTime;
struct GLESTexture
{
    GLuint texture;
    BarrierTime imageWriteTime;
    GLenum target;
    GraphicsFormat format;
    int width;
    int height;
    int layers;
    int mipCount;
    bool immutable;
    GLenum internalFormat; // Needed for binding as image (random write access)
    GLuint viewTexture; // Needed when binding srgb texture as image
};

namespace gles
{
    void InitCaps(ApiGLES* api, GraphicsCaps* caps, GfxDeviceLevelGL &level, const std::vector<std::string>& allExtensions);
    void InitRenderTextureAACaps(ApiGLES* api, GraphicsCaps* caps);
}//namespace gles

extern ApiGLES* gGL;

#endif //HUAHUOENGINE_APIGLES_H
