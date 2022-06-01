//
// Created by VincentZhang on 5/14/2022.
//

#include "ShaderPassContext.h"
#include "Memory/MemoryMacros.h"

ShaderPassContext* g_SharedPassContext = NULL;

void ShaderPassContextInitialize()
{
    Assert(g_SharedPassContext == NULL);
    g_SharedPassContext = HUAHUO_NEW(ShaderPassContext, kMemShader);
}

void ShaderPassContextCleanup()
{
    HUAHUO_DELETE(g_SharedPassContext, kMemShader);
}