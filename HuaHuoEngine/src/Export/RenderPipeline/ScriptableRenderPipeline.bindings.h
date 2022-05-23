//
// Created by VincentZhang on 5/19/2022.
//

#ifndef HUAHUOENGINE_SCRIPTABLERENDERPIPELINE_BINDINGS_H
#define HUAHUOENGINE_SCRIPTABLERENDERPIPELINE_BINDINGS_H

#include "Camera/Camera.h"
#include "Graphics/ScriptableRenderLoop/ScriptableCulling.h"

namespace ScriptableRenderPipeline_Bindings
{
    inline bool GetCullingParameters_Internal(Camera *camera, bool stereoAware, ScriptableCullingParameters &cullingParameters, int managedCullingParametersSize)
    {
        AssertMsg(sizeof(ScriptableCullingParameters) == managedCullingParametersSize, Format("(sizeof(ScriptableCullingParameters) == %zu) != (managedCullingParametersSize == %d)", sizeof(ScriptableCullingParameters), managedCullingParametersSize));
        Camera& cullingCamera = GetCullingCameraAndSetCullingFlag(*camera, cullingParameters);
        return GetScriptableCullingParameters(cullingCamera, stereoAware, cullingParameters);
    }

//    inline void Internal_Cull(ScriptableCullingParameters &parameters, const ScriptableRenderContext* renderContext, ScriptableCullResults& results)
//    {
//        ScriptableCullResults* cullResults = CullScriptable(renderContext, parameters);
//
//        if (cullResults == NULL)
//            return;
//
//#if ENABLE_UNITY_COLLECTIONS_CHECKS
//        results->safety = renderLoop.safety;
//#endif
//        results->allocationInfo = &cullResults->allocationInfo;
//        renderContext->AddCullResultsCleanup(cullResults);
//    }
}
#endif //HUAHUOENGINE_SCRIPTABLERENDERPIPELINE_BINDINGS_H
