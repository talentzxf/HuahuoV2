//
// Created by VincentZhang on 5/16/2022.
//

#include "RenderPipelineManager.h"
#include "Utilities/RegisterRuntimeInitializeAndCleanup.h"

RenderPipelineManager* g_RenderPipelineMgr = NULL;
void CreateRenderPipelineManager(void*){
    g_RenderPipelineMgr = NEW(RenderPipelineManager);
}

void DestroyRenderPipelineManager(void*){
    Assert(g_RenderPipelineMgr != NULL);
    DELETE(g_RenderPipelineMgr);
    g_RenderPipelineMgr = NULL;
}

static RegisterRuntimeInitializeAndCleanup sInit_MessageIdentifier(CreateRenderPipelineManager, DestroyRenderPipelineManager, -1);