//
// Created by VincentZhang on 5/16/2022.
//

#ifndef HUAHUOENGINE_RENDERPIPELINE_H
#define HUAHUOENGINE_RENDERPIPELINE_H
#include "Graphics/ScriptableRenderLoop/ScriptableRenderContext.h"
#include "Camera/Camera.h"

// The class for JS to implement
class RenderPipeline {
public:
    virtual void Render(ScriptableRenderContext* context) = 0;
};


#endif //HUAHUOENGINE_RENDERPIPELINE_H
