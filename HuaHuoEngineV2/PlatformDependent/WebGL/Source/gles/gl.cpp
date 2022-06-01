#include "UnityPrefix.h"
#include "Runtime/GfxDevice/opengles/ApiTypeGLES.h"
#include <emscripten/emscripten.h>

#define NOT_IMPLEMENTED                                            \
    {                                                              \
        static bool printedOnce = false;                           \
        if (!printedOnce)                                          \
        {                                                          \
            emscripten_log(EM_LOG_ERROR | EM_LOG_JS_STACK, "Internal Unity error: function %s is not supported on WebGL. Please report a bug.\n", __FUNCTION__);    \
            printedOnce = true;                                    \
        }                                                          \
    }

extern "C"
{
    // glext.h
    void glTextureView(GLuint texture, GLenum target, GLuint origtexture, GLenum internalformat, GLuint minlevel, GLuint numlevels, GLuint minlayer, GLuint numlayers) NOT_IMPLEMENTED;
    void glGetQueryObjectui64v(GLuint id, GLenum pname, GLuint64 *params) NOT_IMPLEMENTED;
    void glFramebufferTexture2DMultisampleEXT(GLenum, GLenum, GLenum, GLuint, GLint, GLsizei) NOT_IMPLEMENTED;
    // gles3.1 / 3.2
    void glBindImageTexture(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format) NOT_IMPLEMENTED;
    void glBlendBarrier(void) NOT_IMPLEMENTED;
    void glBlendEquationSeparatei(GLuint buf, GLenum modeRGB, GLenum modeAlpha) NOT_IMPLEMENTED;
    void glBlendEquationi(GLuint buf, GLenum mode) NOT_IMPLEMENTED;
    void glBlendFuncSeparatei(GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha) NOT_IMPLEMENTED;
    void glColorMaski(GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a) NOT_IMPLEMENTED;
    void glDebugMessageInsert(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf) NOT_IMPLEMENTED;
    void glFramebufferTexture(GLenum target, GLenum attachment, GLuint texture, GLint level) NOT_IMPLEMENTED;
    void glGetProgramInterfaceiv(GLuint program, GLenum programInterface, GLenum pname, GLint *params) NOT_IMPLEMENTED;
    void glGetProgramResourceName(GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name) NOT_IMPLEMENTED;
    void glGetProgramResourceiv(GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei bufSize, GLsizei *length, GLint *params) NOT_IMPLEMENTED;
    void glMemoryBarrier(GLbitfield barriers) NOT_IMPLEMENTED;
    void glProgramUniform1fv(GLuint program, GLint location, GLsizei count, GLfloat* v) NOT_IMPLEMENTED;
    void glProgramUniform2fv(GLuint program, GLint location, GLsizei count, GLfloat* v) NOT_IMPLEMENTED;
    void glProgramUniform3fv(GLuint program, GLint location, GLsizei count, GLfloat* v) NOT_IMPLEMENTED;
    void glProgramUniform4fv(GLuint program, GLint location, GLsizei count, GLfloat* v) NOT_IMPLEMENTED;
    void glProgramUniform1iv(GLuint program, GLint location, GLsizei count, GLint* v) NOT_IMPLEMENTED;
    void glProgramUniform2iv(GLuint program, GLint location, GLsizei count, GLint* v) NOT_IMPLEMENTED;
    void glProgramUniform3iv(GLuint program, GLint location, GLsizei count, GLint* v) NOT_IMPLEMENTED;
    void glProgramUniform4iv(GLuint program, GLint location, GLsizei count, GLint* v) NOT_IMPLEMENTED;
    void glProgramUniform1uiv(GLuint program, GLint location, GLsizei count, GLuint* v) NOT_IMPLEMENTED;
    void glProgramUniform2uiv(GLuint program, GLint location, GLsizei count, GLuint* v) NOT_IMPLEMENTED;
    void glProgramUniform3uiv(GLuint program, GLint location, GLsizei count, GLuint* v) NOT_IMPLEMENTED;
    void glProgramUniform4uiv(GLuint program, GLint location, GLsizei count, GLuint* v) NOT_IMPLEMENTED;
    void glProgramUniformMatrix2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, GLfloat* v) NOT_IMPLEMENTED;
    void glProgramUniformMatrix3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, GLfloat* v) NOT_IMPLEMENTED;
    void glProgramUniformMatrix4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, GLfloat* v) NOT_IMPLEMENTED;
    void glProgramUniformMatrix2x3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, GLfloat* v) NOT_IMPLEMENTED;
    void glProgramUniformMatrix3x2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, GLfloat* v) NOT_IMPLEMENTED;
    void glProgramUniformMatrix2x4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, GLfloat* v) NOT_IMPLEMENTED;
    void glProgramUniformMatrix4x2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, GLfloat* v) NOT_IMPLEMENTED;
    void glProgramUniformMatrix3x4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, GLfloat* v) NOT_IMPLEMENTED;
    void glProgramUniformMatrix4x3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, GLfloat* v) NOT_IMPLEMENTED;
}
