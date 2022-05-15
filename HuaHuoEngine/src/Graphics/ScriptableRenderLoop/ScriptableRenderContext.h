//
// Created by VincentZhang on 5/14/2022.
//

#ifndef HUAHUOENGINE_SCRIPTABLERENDERCONTEXT_H
#define HUAHUOENGINE_SCRIPTABLERENDERCONTEXT_H
#include <vector>
#include "Camera/CullingParameters.h"

class Camera;

class ScriptableRenderContext {
public:
    void ExtractAndExecuteRenderPipeline(const std::vector<Camera*>& cameras, PostProcessCullResults* postProcessCullResults = NULL, void* postProcessCullResultsData = NULL);
};


#endif //HUAHUOENGINE_SCRIPTABLERENDERCONTEXT_H
