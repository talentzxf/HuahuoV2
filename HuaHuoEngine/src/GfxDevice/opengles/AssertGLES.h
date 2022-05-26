#pragma once

#include "ApiGLES.h"
#include "Logging/LogAssert.h"

// To enable OpenGL asserts, set ENABLE_GL_DEBUG_ASSERT to 1
#define ENABLE_GL_DEBUG_ASSERT 0

#ifndef PLATFORM_SUPPORTS_GL_DEBUG
#   define PLATFORM_SUPPORTS_GL_DEBUG 0
#endif

// Debug only helper: to run OpenGL asserts on Katana: replace with #if 1.
#define DEBUG_GL_ERROR_CHECKS (!UNITY_RELEASE && (ENABLE_GL_DEBUG_ASSERT || PLATFORM_SUPPORTS_GL_DEBUG))

class ApiGLES;
bool CheckErrorGLES(const ApiGLES * const, const char *prefix, const char* file, long line);
void PreApiCallChecksGLES(const ApiGLES * const, const char *funcname, const char* file, long line);

#if defined(_MSC_VER)
#   define GLES_FUNCTION __FUNCTION__
#elif defined(__clang__) || defined(__GNUC__)
#   if __cplusplus >= 201103L
#       define GLES_FUNCTION __func__
#   else
#       define GLES_FUNCTION __FUNCTION__
#   endif
#else
#   define GLES_FUNCTION ""
#endif

#if __GNUC__
#   define SUPPORT_VARIADIC_TEMPLATE (__cplusplus >= 201103L || (__GXX_EXPERIMENTAL_CXX0X__ && (__GNUC__ > 4 || __GNUC__ == 4 && __GNUC_MINOR__ >= 4)))
#else
#   define SUPPORT_VARIADIC_TEMPLATE (__cplusplus >= 201103L)
#endif

#if DEBUG_GL_ERROR_CHECKS && SUPPORT_VARIADIC_TEMPLATE

#include <sstream>
#include "Runtime/Utilities/Word.h"

namespace gl
{
namespace detail
{
    class ArgsToString
    {
    public:
        template<typename ... Ts>
        core::string operator()(Ts const& ... args)
        {
            PrintAll(args ...);
            return m_Out.str();
        }

    private:
        template<typename T>
        void Print(T arg)
        {
            m_Out << arg;
        }

        void PrintAll()
        {
        }

        template<typename Head>
        void PrintAll(Head const& h)
        {
            Print(h);
        }

        template<typename Head, typename ... Tail>
        void PrintAll(Head const& h, Tail const& ... tail)
        {
            Print(h);
            m_Out << ", ";
            PrintAll(tail ...);
        }

        std::stringstream m_Out;
    };

    template<typename ... Ts>
    core::string ToString(Ts const& ... args)
    {
        return ArgsToString()(args ...);
    }
}     // namespace detail
} // namespace gl

#   define GLES_ARGS_TO_STRING(...) gl::detail::ToString(__VA_ARGS__)
#else
#   define GLES_ARGS_TO_STRING(...) core::string()
#endif

#if DEBUG_GL_ERROR_CHECKS

// Check that OpenGL calls are made on the correct thread
#   define GLES_CHECK(api, name) \
        do                                                                                              \
        {                                                                                               \
            (api)->Check(name, GLES_FUNCTION, __LINE__, __FILE__);                                      \
        } while(0)

// Normal AssertMsg followed by glGetError but only in debug build. These checks are completely discarded in release.
#   define GLES_ASSERT(api, x, message)                                                                 \
        do                                                                                              \
        {                                                                                               \
            AssertMsg(x, (core::string("OPENGL ERROR: ") + message).c_str());                           \
            { CheckErrorGLES((api), "OPENGL ERROR", __FILE__, __LINE__); }                              \
        } while(0)

#   define GLES_ASSIGN_RET_VOID(r)
#   define GLES_ASSIGN_RET_VALUE(r) r=

#   define GLES_CALL_IMPL_(api, assignret, retval, funcname, ...) \
        do                                                                                              \
        {                                                                                               \
            PreApiCallChecksGLES(api, #funcname, __FILE__, __LINE__);                                   \
            Assert((api)->funcname);                                                                    \
            { assignret(retval) (api)->funcname(__VA_ARGS__); }                                         \
            AssertFormatMsg(CheckErrorGLES((api), #funcname, __FILE__, __LINE__),                       \
                            "OpenGL Error after calling %s(%s)",                                        \
                            #funcname, GLES_ARGS_TO_STRING(__VA_ARGS__).c_str());                       \
        } while(0)

// Call OpenGL functions with full checking
#   define GLES_CALL(api, funcname, ...)                                                                \
        GLES_CALL_IMPL_(api, GLES_ASSIGN_RET_VOID, NULL, funcname, __VA_ARGS__)

// Call OpenGL functions that return a value with full checking
#   define GLES_CALL_RET(api, retval, funcname, ...)                                                    \
        GLES_CALL_IMPL_(api, GLES_ASSIGN_RET_VALUE, retval, funcname, __VA_ARGS__)
#else
#   define GLES_CHECK(api, name)                        do {} while (0)
#   define GLES_ASSERT(api, x, message)                 do {} while (0)

#   if PLATFORM_WEBGL
#       define GLES_CALL(api, funcname, ...)                funcname(__VA_ARGS__)
#       define GLES_CALL_RET(api, retval, funcname, ...)    retval = funcname(__VA_ARGS__)
#   else
#   define GLES_CALL(api, funcname, ...)                (api)->funcname(__VA_ARGS__)
#   define GLES_CALL_RET(api, retval, funcname, ...)    retval = (api)->funcname(__VA_ARGS__)
#endif
#endif
