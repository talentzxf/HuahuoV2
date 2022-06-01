//
// Created by VincentZhang on 5/14/2022.
//

#ifndef HUAHUOENGINE_SHADERPASSCONTEXT_H
#define HUAHUOENGINE_SHADERPASSCONTEXT_H
#include "Utilities/NonCopyable.h"
#include "ShaderKeywordSet.h"
#include "ShaderPropertySheet.h"

#define CAN_DO_ASYNC_COMPILATION UNITY_EDITOR || ENABLE_PLAYERCONNECTION

struct ShaderPassContext : public NonCopyable
{
    ShaderPassContext()
            : maxKeywordCount(0)
            , properties(kMemShader)
            , allowShaderStencil(true)
#if CAN_DO_ASYNC_COMPILATION
    , allowAsyncShaderCompilation(false)
#endif
    {}

    void CopyFrom(const ShaderPassContext& other);

    bool GetAllowAsyncShaderCompilation() const
    {
#if CAN_DO_ASYNC_COMPILATION
        return allowAsyncShaderCompilation;
#else
        return false;
#endif
    }

    int                 maxKeywordCount;
    ShaderKeywordSet    keywords;
    ShaderPropertySheet properties;

    // Says when it's ok for a pass to set it's stencil state
    // (otherwise the deferred render loop might be using it).
    bool allowShaderStencil;
#if CAN_DO_ASYNC_COMPILATION
    bool allowAsyncShaderCompilation;
#endif
};

extern ShaderPassContext* g_SharedPassContext;
void ShaderPassContextInitialize();
void ShaderPassContextCleanup();

inline ShaderPassContext& GetDefaultPassContext()
{
    // ASSERT_RUNNING_ON_MAIN_THREAD;
    DebugAssert(g_SharedPassContext);
    return *g_SharedPassContext;
}

#if UNITY_EDITOR
class AutoAsyncShaderCompilationScope : NonCopyable
{
public:
    AutoAsyncShaderCompilationScope(ShaderPassContext& passContext, bool allowAsync)
        : m_passContext(passContext)
    {
        m_oldValue = passContext.allowAsyncShaderCompilation;
        passContext.allowAsyncShaderCompilation = allowAsync;
    }

    ~AutoAsyncShaderCompilationScope()
    {
        m_passContext.allowAsyncShaderCompilation = m_oldValue;
    }

private:
    ShaderPassContext& m_passContext;
    bool m_oldValue;
};
#endif

#endif //HUAHUOENGINE_SHADERPASSCONTEXT_H
