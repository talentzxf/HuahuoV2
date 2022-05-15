//
// Created by VincentZhang on 5/14/2022.
//

#include "ShaderPassContext.h"
#include "Memory/MemoryMacros.h"

ShaderPassContext* g_SharedPassContext = NULL;

void ShaderPassContextInitialize()
{
    Assert(g_SharedPassContext == NULL);
    // g_SharedPassContext = UNITY_NEW(ShaderPassContext, kMemShader);
    g_SharedPassContext = NEW(ShaderPassContext);//, kMemShader);
}

void ShaderPassContextCleanup()
{
    // UNITY_DELETE(g_SharedPassContext, kMemShader);
    DELETE(g_SharedPassContext)
}