#pragma once

// On other platforms, functions are resolved at runtime using glGetProcAddress. On WebGL this technique has performance
// and build size penalties so we need to resolve all gl calls at compile time.

// note that webgl2.0 does not actually support all gles3.2 features but we need GLES3/gl32.h anyway because
// our codebase relies on them (although they are not used at runtime)
#include <GLES3/gl32.h>
#include <GLES3/gl2ext.h>

extern "C"
{
    // GL/glext.h enums / funcs
    void glTextureView(GLuint texture, GLenum target, GLuint origtexture, GLenum internalformat, GLuint minlevel, GLuint numlevels, GLuint minlayer, GLuint numlayers);
    void glGetQueryObjectui64v(GLuint id, GLenum pname, GLuint64 *params);
    void glFramebufferTexture2DMultisampleEXT(GLenum, GLenum, GLenum, GLuint, GLint, GLsizei);
    // platform-specific
    void glResolveMultisampleFramebufferAPPLE();
}
