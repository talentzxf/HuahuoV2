//
// Created by VincentZhang on 5/16/2022.
//

#ifndef HUAHUOENGINE_RENDERPIPELINEMANAGER_H
#define HUAHUOENGINE_RENDERPIPELINEMANAGER_H
#include "RenderPipeline.h"

class RenderPipelineManager;
RenderPipelineManager* GetRenderPipelineManager();

class RenderPipelineManager {
private:
    RenderPipeline* m_RenderPipeline;
public:

    static RenderPipelineManager* GetManager() { return GetRenderPipelineManager(); }

    void SetRenderPipeline(RenderPipeline* pRenderPipeline){
        m_RenderPipeline = pRenderPipeline;
    }

    RenderPipeline* GetRenderPipeline(){
        return m_RenderPipeline;
    }
};


#endif //HUAHUOENGINE_RENDERPIPELINEMANAGER_H
