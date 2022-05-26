//
// Created by VincentZhang on 5/25/2022.
//

#include "ApiFuncGLES.h"
#include "ApiGLES.h"
#include "GfxGetProcAddressGLES.h"
#include "ExtensionsGLES.h"

#if PLATFORM_WEBGL
#   define HAS_GLES30_HEADER 1
#   define HAS_GLES31_HEADER 0
#   define HAS_GL_AEP_HEADER 0
#   define HAS_GLCORE_HEADER 0

#elif PLATFORM_ANDROID
#   define HAS_GLES30_HEADER 1
#   define HAS_GLES31_HEADER 1
#   define HAS_GL_AEP_HEADER 1
#   define HAS_GLES32_HEADER 1
#   define HAS_GLCORE_HEADER 0

#else // We load the functions so we rely on our own definitions.
#   define HAS_GLES30_HEADER 1
#   define HAS_GLES31_HEADER 1
#   define HAS_GLES32_HEADER 1
#   define HAS_GL_AEP_HEADER 1
#   define HAS_GLCORE_HEADER 1
#endif

#if FORCE_DEBUG_BUILD_WEBGL
#   undef PLATFORM_WEBGL
#   define PLATFORM_WEBGL 1
#endif//FORCE_DEBUG_BUILD_WEBGL

void ApiGLES::Load(GfxDeviceLevelGL level)
{
#   if UNITY_APPLE || PLATFORM_LINUX || PLATFORM_ANDROID || PLATFORM_WIN || PLATFORM_LUMIN
#       define GLESAPI_STRINGIFY(s) GLESAPI_STRINGIFY2(s)
#       define GLESAPI_STRINGIFY2(s) #s
#       define GLES_GET_FUNC(api, func)     do { api.gl ## func = reinterpret_cast<gl::func##Func>(gles::GetProcAddress_core("gl" GLESAPI_STRINGIFY(func))); } while(0)
#   else
#       define GLES_GET_FUNC(api, func)     do { api.gl ## func = reinterpret_cast<gl::func##Func>(::gl ## func); } while(0)
#   endif
    DebugAssertMsg(IsGfxLevelES(level) || IsGfxLevelCore(level), "OPENGL ERROR: Invalid device level");

    ApiGLES& api = *this;

    GLES_GET_FUNC(api, ActiveTexture);
    GLES_GET_FUNC(api, AttachShader);
    GLES_GET_FUNC(api, BindAttribLocation);
    GLES_GET_FUNC(api, BindBuffer);
    GLES_GET_FUNC(api, BindFramebuffer);
    GLES_GET_FUNC(api, BindRenderbuffer);
    GLES_GET_FUNC(api, BindTexture);
    GLES_GET_FUNC(api, BlendEquation);
    GLES_GET_FUNC(api, BlendEquationSeparate);
    GLES_GET_FUNC(api, BlendFuncSeparate);
    GLES_GET_FUNC(api, BufferData);
    GLES_GET_FUNC(api, BufferSubData);
    GLES_GET_FUNC(api, CheckFramebufferStatus);
    GLES_GET_FUNC(api, Clear);
    GLES_GET_FUNC(api, ClearColor);
    GLES_GET_FUNC(api, ClearDepthf);
    GLES_GET_FUNC(api, ClearStencil);
    GLES_GET_FUNC(api, ColorMask);
    GLES_GET_FUNC(api, CompileShader);
    GLES_GET_FUNC(api, CompressedTexImage2D);
    GLES_GET_FUNC(api, CompressedTexSubImage2D);
    GLES_GET_FUNC(api, CopyTexImage2D);
    GLES_GET_FUNC(api, CopyTexSubImage2D);
    GLES_GET_FUNC(api, CreateProgram);
    GLES_GET_FUNC(api, CreateShader);
    GLES_GET_FUNC(api, CullFace);
    GLES_GET_FUNC(api, DeleteBuffers);
    GLES_GET_FUNC(api, DeleteFramebuffers);
    GLES_GET_FUNC(api, DeleteProgram);
    GLES_GET_FUNC(api, DeleteRenderbuffers);
    GLES_GET_FUNC(api, DeleteShader);
    GLES_GET_FUNC(api, DeleteTextures);
    GLES_GET_FUNC(api, DepthFunc);
    GLES_GET_FUNC(api, DepthMask);
    GLES_GET_FUNC(api, DetachShader);
    GLES_GET_FUNC(api, Disable);
    GLES_GET_FUNC(api, DisableVertexAttribArray);
    GLES_GET_FUNC(api, DrawArrays);
    GLES_GET_FUNC(api, DrawElements);
    GLES_GET_FUNC(api, IsEnabled);
    GLES_GET_FUNC(api, Enable);
    GLES_GET_FUNC(api, EnableVertexAttribArray);
    GLES_GET_FUNC(api, Finish);
    GLES_GET_FUNC(api, Flush);
    GLES_GET_FUNC(api, FramebufferRenderbuffer);
    GLES_GET_FUNC(api, GetRenderbufferParameteriv);
    GLES_GET_FUNC(api, FramebufferTexture2D);
#if !PLATFORM_WEBGL
    GLES_GET_FUNC(api, FramebufferTexture3D);
#endif
    GLES_GET_FUNC(api, FrontFace);
    GLES_GET_FUNC(api, GenBuffers);
    GLES_GET_FUNC(api, GenerateMipmap);
    GLES_GET_FUNC(api, GenFramebuffers);
    GLES_GET_FUNC(api, GenRenderbuffers);
    GLES_GET_FUNC(api, GenTextures);
    GLES_GET_FUNC(api, GetActiveAttrib);
    GLES_GET_FUNC(api, GetActiveUniform);
    GLES_GET_FUNC(api, GetAttribLocation);
    GLES_GET_FUNC(api, GetError);
    GLES_GET_FUNC(api, GetFramebufferAttachmentParameteriv);
    GLES_GET_FUNC(api, GetIntegerv);
    GLES_GET_FUNC(api, GetProgramiv);
    GLES_GET_FUNC(api, GetProgramInfoLog);
    GLES_GET_FUNC(api, ValidateProgram);
    GLES_GET_FUNC(api, GetShaderiv);
    GLES_GET_FUNC(api, GetShaderSource);
    GLES_GET_FUNC(api, GetShaderInfoLog);
    GLES_GET_FUNC(api, GetShaderPrecisionFormat);
    GLES_GET_FUNC(api, GetString);
    GLES_GET_FUNC(api, GetTexParameteriv);
#if !PLATFORM_WEBGL
    GLES_GET_FUNC(api, GetTexLevelParameterfv);
    GLES_GET_FUNC(api, GetTexLevelParameteriv);
#endif
    GLES_GET_FUNC(api, GetUniformiv);
    GLES_GET_FUNC(api, GetUniformLocation);
    GLES_GET_FUNC(api, GetVertexAttribiv);
    GLES_GET_FUNC(api, IsEnabled);
    GLES_GET_FUNC(api, LinkProgram);
    GLES_GET_FUNC(api, PixelStorei);
    GLES_GET_FUNC(api, PolygonOffset);
    GLES_GET_FUNC(api, ReadPixels);
    GLES_GET_FUNC(api, RenderbufferStorage);
    GLES_GET_FUNC(api, Scissor);
    GLES_GET_FUNC(api, ShaderSource);
    GLES_GET_FUNC(api, StencilFuncSeparate);
    GLES_GET_FUNC(api, StencilMask);
    GLES_GET_FUNC(api, StencilOpSeparate);
    GLES_GET_FUNC(api, TexImage2D);
#if !PLATFORM_WEBGL
    GLES_GET_FUNC(api, TexImage2DMultisample);
#endif
    GLES_GET_FUNC(api, TexParameterf);
    GLES_GET_FUNC(api, TexParameteri);
    GLES_GET_FUNC(api, TexParameteriv);
    GLES_GET_FUNC(api, TexSubImage2D);
    GLES_GET_FUNC(api, Uniform1fv);
    GLES_GET_FUNC(api, Uniform1i);
    GLES_GET_FUNC(api, Uniform1iv);
    GLES_GET_FUNC(api, Uniform1uiv);
    GLES_GET_FUNC(api, Uniform2fv);
    GLES_GET_FUNC(api, Uniform2iv);
    GLES_GET_FUNC(api, Uniform2uiv);
    GLES_GET_FUNC(api, Uniform3fv);
    GLES_GET_FUNC(api, Uniform3iv);
    GLES_GET_FUNC(api, Uniform3uiv);
    GLES_GET_FUNC(api, Uniform4fv);
    GLES_GET_FUNC(api, Uniform4iv);
    GLES_GET_FUNC(api, Uniform4uiv);
    GLES_GET_FUNC(api, UniformMatrix3fv);
    GLES_GET_FUNC(api, UniformMatrix4fv);

    GLES_GET_FUNC(api, UseProgram);
    GLES_GET_FUNC(api, VertexAttrib4f);
    GLES_GET_FUNC(api, VertexAttrib4fv);
    GLES_GET_FUNC(api, VertexAttribPointer);
    GLES_GET_FUNC(api, Viewport);

#   if HAS_GLES30_HEADER || HAS_GLCORE_HEADER
    if (IsGfxLevelES3(level) || HAS_GLCORE_HEADER)
    {
        GLES_GET_FUNC(api, GenQueries);
        GLES_GET_FUNC(api, DeleteQueries);
        GLES_GET_FUNC(api, BeginQuery);
        GLES_GET_FUNC(api, EndQuery);

        GLES_GET_FUNC(api, BindVertexArray);
        GLES_GET_FUNC(api, IsVertexArray);
        GLES_GET_FUNC(api, DeleteVertexArrays);
        GLES_GET_FUNC(api, GenVertexArrays);

        GLES_GET_FUNC(api, BeginTransformFeedback);
        GLES_GET_FUNC(api, EndTransformFeedback);
        GLES_GET_FUNC(api, TransformFeedbackVaryings);
        GLES_GET_FUNC(api, BindTransformFeedback);
        GLES_GET_FUNC(api, DeleteTransformFeedbacks);
        GLES_GET_FUNC(api, GenTransformFeedbacks);

        GLES_GET_FUNC(api, TexImage3D);
        GLES_GET_FUNC(api, TexSubImage3D);
        GLES_GET_FUNC(api, CompressedTexSubImage3D);
        GLES_GET_FUNC(api, CompressedTexImage3D);
        GLES_GET_FUNC(api, TexStorage2D);
        GLES_GET_FUNC(api, TexStorage3D);
        GLES_GET_FUNC(api, BlitFramebuffer);
        GLES_GET_FUNC(api, RenderbufferStorageMultisample);
        GLES_GET_FUNC(api, GetStringi);
        GLES_GET_FUNC(api, GetIntegeri_v);
        GLES_GET_FUNC(api, MapBufferRange);
        GLES_GET_FUNC(api, UnmapBuffer);
        GLES_GET_FUNC(api, FlushMappedBufferRange);
        GLES_GET_FUNC(api, InvalidateFramebuffer);
        GLES_GET_FUNC(api, DrawArraysInstanced);
        GLES_GET_FUNC(api, DrawElementsInstanced);
        GLES_GET_FUNC(api, CopyBufferSubData);

        GLES_GET_FUNC(api, DrawBuffers);
        GLES_GET_FUNC(api, ReadBuffer);

        GLES_GET_FUNC(api, FramebufferTextureLayer);
#       if HAS_GL_AEP_HEADER
        GLES_GET_FUNC(api, FramebufferTexture);
#       endif

        GLES_GET_FUNC(api, BindBufferBase);
        GLES_GET_FUNC(api, BindBufferRange);
        GLES_GET_FUNC(api, GetActiveUniformsiv);
        GLES_GET_FUNC(api, GetUniformBlockIndex);
        GLES_GET_FUNC(api, GetUniformIndices);
        GLES_GET_FUNC(api, GetActiveUniformBlockiv);
        GLES_GET_FUNC(api, GetActiveUniformBlockName);
        GLES_GET_FUNC(api, UniformBlockBinding);
        GLES_GET_FUNC(api, VertexAttribIPointer);

        GLES_GET_FUNC(api, GetProgramBinary);
        GLES_GET_FUNC(api, ProgramBinary);
        GLES_GET_FUNC(api, ProgramParameteri);

        GLES_GET_FUNC(api, GenSamplers);
        GLES_GET_FUNC(api, DeleteSamplers);
        GLES_GET_FUNC(api, BindSampler);
        GLES_GET_FUNC(api, SamplerParameteri);

        GLES_GET_FUNC(api, GetInternalformativ);

        GLES_GET_FUNC(api, FenceSync);
        GLES_GET_FUNC(api, ClientWaitSync);
        GLES_GET_FUNC(api, DeleteSync);

        GLES_GET_FUNC(api, ClearBufferuiv);
        GLES_GET_FUNC(api, ClearBufferfv);
        GLES_GET_FUNC(api, ClearBufferfi);
    }
#   endif

#   if HAS_GLES31_HEADER || HAS_GLCORE_HEADER
    if (IsGfxLevelES3(level, kGfxLevelES31) || HAS_GLCORE_HEADER)
    {
        GLES_GET_FUNC(api, ProgramUniform1fv);
        GLES_GET_FUNC(api, ProgramUniform1iv);
        GLES_GET_FUNC(api, ProgramUniform2fv);
        GLES_GET_FUNC(api, ProgramUniform2iv);
        GLES_GET_FUNC(api, ProgramUniform3fv);
        GLES_GET_FUNC(api, ProgramUniform3iv);
        GLES_GET_FUNC(api, ProgramUniform4fv);
        GLES_GET_FUNC(api, ProgramUniform4iv);
        GLES_GET_FUNC(api, ProgramUniformMatrix2fv);
        GLES_GET_FUNC(api, ProgramUniformMatrix3fv);
        GLES_GET_FUNC(api, ProgramUniformMatrix4fv);
        GLES_GET_FUNC(api, ProgramUniformMatrix2x3fv);
        GLES_GET_FUNC(api, ProgramUniformMatrix3x2fv);
        GLES_GET_FUNC(api, ProgramUniformMatrix2x4fv);
        GLES_GET_FUNC(api, ProgramUniformMatrix4x2fv);
        GLES_GET_FUNC(api, ProgramUniformMatrix3x4fv);
        GLES_GET_FUNC(api, ProgramUniformMatrix4x3fv);
        GLES_GET_FUNC(api, ProgramUniform1uiv);
        GLES_GET_FUNC(api, ProgramUniform2uiv);
        GLES_GET_FUNC(api, ProgramUniform3uiv);
        GLES_GET_FUNC(api, ProgramUniform4uiv);

        GLES_GET_FUNC(api, BindImageTexture);

        GLES_GET_FUNC(api, DispatchCompute);
        GLES_GET_FUNC(api, DispatchComputeIndirect);

        GLES_GET_FUNC(api, GetProgramInterfaceiv);
        GLES_GET_FUNC(api, GetProgramResourceName);
        GLES_GET_FUNC(api, GetProgramResourceiv);
        GLES_GET_FUNC(api, GetProgramResourceIndex);

        GLES_GET_FUNC(api, DrawArraysIndirect);
        GLES_GET_FUNC(api, DrawElementsIndirect);

        // Avoid name conflict with winnt.h
#       undef MemoryBarrier
        GLES_GET_FUNC(api, MemoryBarrier);
    }
#   endif

#define INTERNAL_GLES_GET_NAMED(api, func, name, force)                     \
    do                                                                      \
    {                                                                       \
        typedef gl::func##Func FuncPtrType;                                 \
        FuncPtrType& fptr = api.gl ## func;                                 \
        if (!fptr || force)                                                 \
        {                                                                   \
            if (void* const loadedFunc = gles::GetProcAddress(name))        \
            {                                                               \
                fptr = reinterpret_cast<FuncPtrType>(loadedFunc);           \
            }                                                               \
        }                                                                   \
    } while(0)

#define GLES_GET_NAMED(api, func, name) INTERNAL_GLES_GET_NAMED(api, func, name, false)
#define GLES_GET_NAMED_OVERRIDE(api, func, name) INTERNAL_GLES_GET_NAMED(api, func, name, true)

#   if HAS_GL_AEP_HEADER || HAS_GLCORE_HEADER
    if (HAS_GLCORE_HEADER || IsGfxLevelES3(level, kGfxLevelES31AEP))
    {
        GLES_GET_FUNC(api, PatchParameteri);
        GLES_GET_FUNC(api, CopyImageSubData);
        GLES_GET_FUNC(api, TexStorage3DMultisample);
    }
#   endif

#   if HAS_GLES32_HEADER || HAS_GLCORE_HEADER
    if (HAS_GLCORE_HEADER || IsGfxLevelES3(level, kGfxLevelES32))
    {
        GLES_GET_FUNC(api, DrawElementsBaseVertex);
        GLES_GET_FUNC(api, DrawElementsInstancedBaseVertex);
        GLES_GET_FUNC(api, BlendFuncSeparatei);
        GLES_GET_FUNC(api, BlendEquationi);
        GLES_GET_FUNC(api, BlendEquationSeparatei);
        GLES_GET_FUNC(api, ColorMaski);
    }
#   endif

    // ES 3.2 only
#   if HAS_GLES32_HEADER
    if (IsGfxLevelES3(level, kGfxLevelES32))
    {
        GLES_GET_FUNC(api, BlendBarrier);
    }
#   endif

    // Desktop GL helpers
#   if HAS_GLCORE_HEADER
    {
        GLES_GET_FUNC(api, GetQueryObjectui64v);
        GLES_GET_FUNC(api, DrawBuffer);
        GLES_GET_FUNC(api, PolygonMode);
        GLES_GET_FUNC(api, ClearDepth);
        // in case we're pre-GL4.0, fetch MRT blending entry points from the extension
        if (HasExtension(GLExt::kGL_ARB_draw_buffers_blend))
        {
            GLES_GET_NAMED(api, BlendFuncSeparatei, "glBlendFuncSeparateiARB");
            GLES_GET_NAMED(api, BlendEquationi, "glBlendEquationiARB");
            GLES_GET_NAMED(api, BlendEquationSeparatei, "glBlendEquationSeparateiARB");
        }
    }
    if (IsGfxLevelCore(level, kGfxLevelCore45) || HAS_GLCORE_HEADER)
    {
        GLES_GET_FUNC(api, GetTextureParameteriv);
        GLES_GET_FUNC(api, GetTextureLevelParameteriv);
    }
#   endif//HAS_GLCORE_HEADER

    if (HAS_GLCORE_HEADER || HasExtension(GLExt::kGL_EXT_multisampled_render_to_texture))
    {
        // In ARM Mali driver glRenderbufferStorageMultisample and glRenderbufferStorageMultisampleEXT are two distinct functions
        // and glRenderbufferStorageMultisample cannot be combined with glFramebufferTexture2DMultisampleEXT
        // so we override the ES3 function with the EXT function.
        GLES_GET_NAMED_OVERRIDE(api, RenderbufferStorageMultisample, "glRenderbufferStorageMultisampleEXT");
        GLES_GET_NAMED(api, FramebufferTexture2DMultisampleEXT, "glFramebufferTexture2DMultisampleEXT");
    }

    if (HAS_GLCORE_HEADER || HasExtension(GLExt::kGL_IMG_multisampled_render_to_texture))
    {
        GLES_GET_NAMED(api, RenderbufferStorageMultisample, "glRenderbufferStorageMultisampleIMG");
        GLES_GET_NAMED(api, FramebufferTexture2DMultisampleEXT, "glFramebufferTexture2DMultisampleIMG");
    }

    if (HAS_GLCORE_HEADER || HasExtension(GLExt::kGL_APPLE_framebuffer_multisample))
    {
        GLES_GET_NAMED(api, RenderbufferStorageMultisample, "glRenderbufferStorageMultisampleAPPLE");
        GLES_GET_NAMED(api, ResolveMultisampleFramebufferAPPLE, "glResolveMultisampleFramebufferAPPLE");
    }

    if (HAS_GLCORE_HEADER || (IsGfxLevelES2(level) && HasExtension(GLExt::kGL_NV_framebuffer_multisample) && HasExtension(GLExt::kGL_NV_framebuffer_blit)))
    {
        GLES_GET_NAMED(api, RenderbufferStorageMultisample, "glRenderbufferStorageMultisampleNV");
    }

    if (HAS_GLCORE_HEADER || IsGfxLevelES3(level, kGfxLevelES32) || HasExtension(GLExt::kGL_KHR_debug))
    {
        if (IsGfxLevelCore(level) || IsGfxLevelES3(level, kGfxLevelES32))
        {
            GLES_GET_NAMED(api, DebugMessageControl, "glDebugMessageControl");
            GLES_GET_NAMED(api, DebugMessageCallback, "glDebugMessageCallback");
            GLES_GET_NAMED(api, DebugMessageInsert, "glDebugMessageInsert");
            GLES_GET_NAMED(api, ObjectLabel, "glObjectLabel");
            GLES_GET_NAMED(api, GetObjectLabel, "glGetObjectLabel");
            GLES_GET_NAMED(api, PushDebugGroup, "glPushDebugGroup");
            GLES_GET_NAMED(api, PopDebugGroup, "glPopDebugGroup");
        }
        else
        {
            GLES_GET_NAMED(api, DebugMessageControl, "glDebugMessageControlKHR");
            GLES_GET_NAMED(api, DebugMessageCallback, "glDebugMessageCallbackKHR");
            GLES_GET_NAMED(api, DebugMessageInsert, "glDebugMessageInsertKHR");
            GLES_GET_NAMED(api, ObjectLabel, "glObjectLabelKHR");
            GLES_GET_NAMED(api, GetObjectLabel, "glGetObjectLabelKHR");
            GLES_GET_NAMED(api, PushDebugGroup, "glPushDebugGroupKHR");
            GLES_GET_NAMED(api, PopDebugGroup, "glPopDebugGroupKHR");
        }
    }

    // dedicated EXT entrypoint -> always fetch if available
    if (HAS_GLCORE_HEADER || HasExtension(GLExt::kGL_EXT_debug_marker))
    {
        GLES_GET_NAMED(api, PushGroupMarkerEXT, "glPushGroupMarkerEXT");
        GLES_GET_NAMED(api, PopGroupMarkerEXT, "glPopGroupMarkerEXT");
    }

    // dedicated EXT entrypoint -> always fetch if available
    if (HAS_GLCORE_HEADER || HasExtension(GLExt::kGL_EXT_debug_label))
    {
        GLES_GET_NAMED(api, LabelObjectEXT, "glLabelObjectEXT");
        GLES_GET_NAMED(api, GetObjectLabelEXT, "glGetObjectLabelEXT");
    }

#   if !PLATFORM_WEBGL  // re-enable when timer queries are suppported by emscripten
    if (HAS_GLCORE_HEADER || HasExtension(GLExt::kGL_EXT_disjoint_timer_query))
    {
        GLES_GET_NAMED(api, GenQueries, "glGenQueriesEXT");
        GLES_GET_NAMED(api, DeleteQueries, "glDeleteQueriesEXT");
        GLES_GET_NAMED(api, BeginQuery, "glBeginQueryEXT");
        GLES_GET_NAMED(api, EndQuery, "glEndQueryEXT");
        GLES_GET_NAMED(api, GetQueryObjectui64v, "glGetQueryObjectui64vEXT");
    }
#   endif

    if (HAS_GLCORE_HEADER || HasExtension(GLExt::kGL_NV_timer_query))
    {
        GLES_GET_NAMED(api, GenQueries, "glGenQueriesEXT");
        GLES_GET_NAMED(api, DeleteQueries, "glDeleteQueriesEXT");
        GLES_GET_NAMED(api, BeginQuery, "glBeginQueryEXT");
        GLES_GET_NAMED(api, EndQuery, "glEndQueryEXT");
        GLES_GET_NAMED(api, GetQueryObjectui64v, "glGetQueryObjectui64vNV");
    }

    if (HAS_GLCORE_HEADER || IsGfxLevelES3(level, kGfxLevelES31AEP) || HasExtension(GLExt::kGL_EXT_texture_buffer))
    {
        if (IsGfxLevelCore(level, kGfxLevelCore43) || IsGfxLevelES3(level, kGfxLevelES32))
            GLES_GET_NAMED(api, TexBuffer, "glTexBuffer");
        else if (IsGfxLevelES3(level, kGfxLevelES31))
            GLES_GET_NAMED(api, TexBuffer, "glTexBufferEXT");
    }

    if (HAS_GLCORE_HEADER || IsGfxLevelES3(level, kGfxLevelES31))
    {
        if (IsGfxLevelCore(level, kGfxLevelCore43) || HasExtension(GLExt::kGL_ARB_texture_view))
            GLES_GET_NAMED(api, TextureView, "glTextureView");
        else if (HasExtension(GLExt::kGL_OES_texture_view))
            GLES_GET_NAMED(api, TextureView, "glTextureViewOES");
        else if (HasExtension(GLExt::kGL_EXT_texture_view))
            GLES_GET_NAMED(api, TextureView, "glTextureViewEXT");
    }

    if (HAS_GLCORE_HEADER || IsGfxLevelES2(level))
    {
        if (HasExtension(GLExt::kGL_OES_texture_3D))
        {
            GLES_GET_NAMED(api, TexImage3D, "glTexImage3DOES");
            GLES_GET_NAMED(api, TexSubImage3D, "glTexSubImage3DOES");
        }

        if (!PLATFORM_WEBGL && HasExtension(GLExt::kGL_OES_vertex_array_object))
        {
            GLES_GET_NAMED(api, BindVertexArray, "glBindVertexArrayOES");
            GLES_GET_NAMED(api, DeleteVertexArrays, "glDeleteVertexArraysOES");
            GLES_GET_NAMED(api, GenVertexArrays, "glGenVertexArraysOES");
        }

        if (HasExtension(GLExt::kGL_EXT_draw_buffers))
        {
            GLES_GET_NAMED(api, DrawBuffers, "glDrawBuffersEXT");
        }
        else if (HasExtension(GLExt::kGL_NV_draw_buffers))
        {
            GLES_GET_NAMED(api, DrawBuffers, "glDrawBuffersNV");
        }

        if (HasExtension(GLExt::kGL_NV_read_buffer))
            GLES_GET_NAMED(api, ReadBuffer, "glReadBufferNV");

        if (HasExtension(GLExt::kGL_NV_framebuffer_blit))
            GLES_GET_NAMED(api, BlitFramebuffer, "glBlitFramebufferNV");

        if (HasExtension(GLExt::kGL_EXT_discard_framebuffer))
            GLES_GET_NAMED(api, InvalidateFramebuffer, "glDiscardFramebufferEXT");

        if (HasExtension(GLExt::kGL_EXT_map_buffer_range))
        {
            GLES_GET_NAMED(api, MapBufferRange, "glMapBufferRangeEXT");
            GLES_GET_NAMED(api, FlushMappedBufferRange, "glFlushMappedBufferRangeEXT");
            GLES_GET_NAMED(api, UnmapBuffer, "glUnmapBufferOES");
            GLES_GET_NAMED(api, UnmapBuffer, "glUnmapBufferEXT");
        }

        if (HasExtension(GLExt::kGL_OES_mapbuffer))
        {
            GLES_GET_NAMED(api, MapBuffer, "glMapBufferOES");
            GLES_GET_NAMED(api, UnmapBuffer, "glUnmapBufferOES");
        }

        if (HasExtension(GLExt::kGL_OES_get_program_binary))
        {
            GLES_GET_NAMED(api, GetProgramBinary, "glGetProgramBinaryOES");
            GLES_GET_NAMED(api, ProgramBinary, "glProgramBinaryOES");
        }

        if (HasExtension(GLExt::kGL_OES_EGL_image))
        {
            GLES_GET_NAMED(api, EGLImageTargetTexture2DOES, "glEGLImageTargetTexture2DOES");
        }
    }

    if (HAS_GLCORE_HEADER || IsGfxLevelES(level))
    {
        if (HasExtension(GLExt::kGL_OES_copy_image))
        {
            GLES_GET_NAMED(api, CopyImageSubData, "glCopyImageSubDataOES");
        }
        else if (HasExtension(GLExt::kGL_EXT_copy_image))
        {
            GLES_GET_NAMED(api, CopyImageSubData, "glCopyImageSubDataEXT");
        }

        if (HasExtension(GLExt::kGL_OES_tessellation_shader))
        {
            GLES_GET_NAMED(api, PatchParameteri, "glPatchParameteriOES");
        }
        else if (HasExtension(GLExt::kGL_EXT_tessellation_shader))
        {
            GLES_GET_NAMED(api, PatchParameteri, "glPatchParameteriEXT");
        }

        if (HasExtension(GLExt::kGL_OES_draw_elements_base_vertex))
        {
            GLES_GET_NAMED(api, DrawElementsBaseVertex, "glDrawElementsBaseVertexOES");
            GLES_GET_NAMED(api, DrawElementsInstancedBaseVertex, "glDrawElementsInstancedBaseVertexOES");
        }
        else if (HasExtension(GLExt::kGL_EXT_draw_elements_base_vertex))
        {
            GLES_GET_NAMED(api, DrawElementsBaseVertex, "glDrawElementsBaseVertexEXT");
            GLES_GET_NAMED(api, DrawElementsInstancedBaseVertex, "glDrawElementsInstancedBaseVertexEXT");
        }

        if (HasExtension(GLExt::kGL_ARB_sparse_texture))
        {
            GLES_GET_NAMED(api, TexPageCommitment, "glTexPageCommitmentARB");
        }
        else if (HasExtension(GLExt::kGL_EXT_sparse_texture))
        {
            GLES_GET_NAMED(api, TexPageCommitment, "glTexPageCommitmentEXT");
        }

        if (HasExtension(GLExt::kGL_EXT_texture_storage))
        {
            GLES_GET_NAMED(api, TexStorage2D, "glTexStorage2DEXT");
            GLES_GET_NAMED(api, TexStorage3D, "glTexStorage3DEXT");
        }

        if (HasExtension(GLExt::kGL_KHR_blend_equation_advanced))
        {
            GLES_GET_NAMED(api, BlendBarrier, "glBlendBarrierKHR");
        }
        else if (HasExtension(GLExt::kGL_NV_blend_equation_advanced))
        {
            GLES_GET_NAMED(api, BlendBarrier, "glBlendBarrierNV");
        }
    }

    if (IsGfxLevelES3(level) && HasExtension(GLExt::kGL_OVR_multiview))
    {
        GLES_GET_NAMED(api, FramebufferTextureMultiview, "glFramebufferTextureMultiviewOVR");

        if (HasExtension(GLExt::kGL_OVR_multiview_multisampled_render_to_texture))
        {
            GLES_GET_NAMED(api, FramebufferTextureMultisampleMultiview, "glFramebufferTextureMultisampleMultiviewOVR");
        }
    }

#if !PLATFORM_WEBGL
    if (HAS_GLCORE_HEADER || IsGfxLevelES3(level, kGfxLevelES31))
    {
        GLES_GET_NAMED(api, TexImage2DMultisample, "glTexImage2DMultisample");
        GLES_GET_NAMED(api, TexStorage2DMultisample, "glTexStorage2DMultisample");
    }
#endif

    if (HasExtension(GLExt::kGL_ARB_direct_state_access))
    {
        GLES_GET_NAMED(api, GetTextureParameteriv, "glGetTextureParameteriv");
    }

    if (PLATFORM_WEBGL && HasExtension(WebGLExt::kWEBGL_draw_buffers))
    {
        GLES_GET_FUNC(api, DrawBuffers);
    }

#   undef GLES_GET_NAMED
}

void ApiGLES::LoadExtensionQuerying(GfxDeviceLevelGL level)
{
    ApiGLES& api = *this;

    GLES_GET_FUNC(api, GetString);
    GLES_GET_FUNC(api, GetIntegerv);

#   if HAS_GLES30_HEADER || HAS_GLCORE_HEADER
    if (IsGfxLevelES3(level) || HAS_GLCORE_HEADER)
        GLES_GET_FUNC(api, GetStringi);
#endif
}