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

class GfxContextGLES;

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

private:
    // -- All initialization time code --

    gl::ContextHandle                               m_Context;

    void Load(GfxDeviceLevelGL contextLevel);
    void LoadExtensionQuerying(GfxDeviceLevelGL level);
    void FillExtensions(std::vector<std::string>& allExtensions);
};

namespace gles
{
    void InitCaps(ApiGLES* api, GraphicsCaps* caps, GfxDeviceLevelGL &level, const std::vector<std::string>& allExtensions);
    void InitRenderTextureAACaps(ApiGLES* api, GraphicsCaps* caps);
}//namespace gles


#endif //HUAHUOENGINE_APIGLES_H
