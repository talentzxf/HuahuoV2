//
// Created by VincentZhang on 5/16/2022.
//

#include "RenderPipelineManager.h"
#include "Utilities/RegisterRuntimeInitializeAndCleanup.h"

RenderPipelineManager* g_RenderPipelineMgr = NULL;
void CreateRenderPipelineManager(void*){
    g_RenderPipelineMgr = HUAHUO_NEW(RenderPipelineManager, kMemRenderer);
}

void DestroyRenderPipelineManager(void*){
    Assert(g_RenderPipelineMgr != NULL);
    HUAHUO_DELETE(g_RenderPipelineMgr, kMemRenderer);
    g_RenderPipelineMgr = NULL;
}

RenderPipelineManager* GetRenderPipelineManager(){
    Assert(g_RenderPipelineMgr != NULL);
    return g_RenderPipelineMgr;
}

static RegisterRuntimeInitializeAndCleanup sInit_MessageIdentifier(CreateRenderPipelineManager, DestroyRenderPipelineManager, -1);