#pragma once

namespace gl
{
    class ContextHandle
    {
    public:
        ContextHandle()
            : context(reinterpret_cast<void*>(0xDEADDEADDEADDEAD))
        {}

        explicit ContextHandle(void* context)
            : context(context)
        {}

        bool operator==(ContextHandle const & contextHandle) const
        {
            return contextHandle.context == this->context;
        }

        bool operator!=(ContextHandle const & contextHandle) const
        {
            return contextHandle.context != this->context;
        }

        bool operator<(ContextHandle const & contextHandle) const
        {
            return contextHandle.context < this->context;
        }

        void* Get() const { return this->context; }

        // Identify an invalid OpenGL context. Used for assert essentially: Something went really wrong!
        static ContextHandle Invalid() { return ContextHandle(); }

        // Default OpenGL objects are independent from the OpenGL context. We use discard to skip the check.
        static ContextHandle Discard() { return ContextHandle(reinterpret_cast<void*>(-1)); }

        // On Android, we don't have a master context when calling GfxDeviceGLES::Init.
        // We use a dummy master context representation to go through the initialization
        static ContextHandle DummyMaster() { return ContextHandle(reinterpret_cast<void*>(1)); }

    private:
        void* context;
    };
}//namespace gl
