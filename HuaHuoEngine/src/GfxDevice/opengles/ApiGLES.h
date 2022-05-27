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

private:
    // -- All initialization time code --

    gl::ContextHandle                               m_Context;

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
