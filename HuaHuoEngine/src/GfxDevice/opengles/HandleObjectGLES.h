#pragma once

#include "ApiTypeGLES.h"
#include "ApiEnumGLES.h"
#include "HandleContextGLES.h"

#define UNITY_GLES_HANDLE_CONTEXT_CHECKS (!UNITY_RELEASE)

namespace gl
{
    // Return a string that represent an object type
    const char* GetLabel(ObjectType type);

    // Is an object type shared across context.
    inline bool IsPerContext(ObjectType type)
    {
        static int const bitField =
            (1 << kVertexArray) |
            (1 << kQuery) |
            (1 << kTransformFeedback) |
            (1 << kFramebuffer);

        return (bitField & (1 << type)) != 0;
    }

    // Query whether an object type can support a default value. Eg: default vertex array object not available with OpenGL core but available with OpenGL ES.
    inline bool CanHaveDefault(ObjectType type)
    {
        static int const bitField =
            (1 << kVertexArray) |
            (1 << kTransformFeedback) |
            (1 << kFramebuffer);

        return (bitField & (1 << type)) != 0;
    }

    // Handles are per OpenGL object instance identifier that bring type safety (All OpenGL names are GLuint)
    // and store the context handle to create the OpenGL object.
    // This allows to check whether we are using the OpenGL object in a different context which is an error.
    template<ObjectType type>
    class Handle
    {
    public:
        Handle()
            : name(0)
            , contextHandle(ContextHandle::Discard())
        {}

        Handle(ContextHandle contextHandle, GLuint name)
            : name(name)
            , contextHandle(contextHandle)
        {}

        // Access the OpenGL object name. Only for interaction with the OpenGL API
        GLuint Get(ContextHandle contextHandle, const char* function, unsigned int line, const char* file) const
        {
            AssertMsg(this->name != kInvalidName, "OPENGL ERROR: Accessing an invalid handle, no OpenGL name associated with this handle");

            if (IsPerContext(type) && *this != Default())
            {
#               if UNITY_GLES_HANDLE_CONTEXT_CHECKS
                AssertFormatMsg(
                    this->contextHandle == contextHandle,
                    "OPENGL ERROR (%s - line %d %s): Accessing a per context OpenGL object from a context different than the context used for the creation of the object. %s Name %d ; context creation: %p ; context accessing: %p",
                    function, line, file,
                    this->Label(), this->name, this->contextHandle.Get(), contextHandle.Get());
#               endif

                if (this->contextHandle != contextHandle)
                    return kInvalidName;
            }

            return this->name;
        }

        bool operator==(const Handle<type>& handle) const
        {
            // Default objects are independent from the OpenGL context.
            if (handle.name == 0 && this->name == 0)
                return true;

            return handle.contextHandle == this->contextHandle && handle.name == this->name;
        }

        bool operator!=(const Handle<type> & handle) const
        {
            return !(*this == handle);
        }

        // Returns a string representing the type of the handle. eg "Buffer" for a Handle<kBuffer>.
        const char* Label() const
        {
            return GetLabel(type);
        }

        // Invalid Handles represent objects that are neither created or destroyed.
        static Handle<type> const Invalid()
        {
            return Handle<type>(ContextHandle(reinterpret_cast<void*>(0xDEADDEADDEADDEAD)), kInvalidName);
        }

        // Default object are available on all contexts and are not OpenGL generated.
        // We use ContextHandle::Discard() to avoid checking
        static Handle<type> const Default()
        {
            return Handle<type>(ContextHandle::Discard(), 0);
        }

        ContextHandle context() const
        {
            return contextHandle;
        }

    private:
        static const GLuint kInvalidName = static_cast<GLuint>(-1);

        GLuint name;
        ContextHandle contextHandle;
    };

    typedef Handle<kBuffer> BufferHandle;
    typedef Handle<kShader> ShaderHandle;
    typedef Handle<kProgram> ProgramHandle;
    typedef Handle<kVertexArray> VertexArrayHandle;
    typedef Handle<kQuery> QueryHandle;
    typedef Handle<kTransformFeedback> TransformFeedbackHandle;
    typedef Handle<kSampler> SamplerHandle;
    typedef Handle<kTexture> TextureHandle;
    typedef Handle<kRenderbuffer> RenderbufferHandle;
    typedef Handle<kFramebuffer> FramebufferHandle;
}//namespace gl

#if UNITY_GLES_HANDLE_CONTEXT_CHECKS
#   define GLES_GET_NAME(context, handle) (handle).Get((context), __FUNCTION__, __LINE__, __FILE__)
#else
#   define GLES_GET_NAME(context, handle) (handle).Get((context), NULL, 0, NULL)
#endif
