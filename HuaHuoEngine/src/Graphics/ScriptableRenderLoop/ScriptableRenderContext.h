//
// Created by VincentZhang on 5/14/2022.
//

#ifndef HUAHUOENGINE_SCRIPTABLERENDERCONTEXT_H
#define HUAHUOENGINE_SCRIPTABLERENDERCONTEXT_H
#include <vector>
#include "Camera/CullingParameters.h"
#include "ScriptableDrawRenderers.h"
#include "ScritableCulling.h"

enum ScriptableRenderCommandType
{
    kScriptRenderCommand_DrawRenderers = 0,
    kScriptRenderCommand_DrawShadows,
    kScriptRenderCommand_ExecuteCommandBuffer,
    kScriptRenderCommand_ExecuteCommandBufferAsync,
    kScriptRenderCommand_SetupCameraProperties,
    kScriptRenderCommand_DrawSkybox,
    kScriptRenderCommand_InvokeOnRenderObjectCallbacks,
    kScriptRenderCommand_BeginRenderPass,
    kScriptRenderCommand_BeginSubPass,
    kScriptRenderCommand_EndSubPass,
    kScriptRenderCommand_EndRenderPass,
    kScriptRenderCommand_StereoSetupCameraProperties,
    kScriptRenderCommand_StereoEndRender,
    kScriptRenderCommand_StartMultiEye,
    kScriptRenderCommand_StopMultiEye,
    kScriptRenderCommand_DrawPreImageEffectsGizmos,
    kScriptRenderCommand_DrawPostImageEffectsGizmos,
    kScriptRenderCommand_DrawUIOverlay,
    kScriptRenderCommand_DrawWireOverlay,
    kScriptRenderCommandCount
};

class Camera;
class RenderingCommandBuffer;

class ScriptableRenderContext {
public:
    void ExtractAndExecuteRenderPipeline(const std::vector<Camera*>& cameras, PostProcessCullResults* postProcessCullResults = NULL, void* postProcessCullResultsData = NULL);
    Camera* GetCamera(int index);
    int GetNumberOfCameras();

    void DrawRenderers(ScriptableCullResults *cullResults, const DrawingSettings& drawSettings, const FilteringSettings& filterSettings, ShaderTagID tagName, bool isPassTagName, ShaderTagID const *tagValues, RenderStateBlock const *stateBlocks, int stateCount);
    void ExecuteCommandBuffer(RenderingCommandBuffer& commandBuffer);
    void Submit();
private:
    void CleanupCommandBuffers();
    void ExecuteScriptableRenderLoop();
    void AddCommandWithIndex(ScriptableRenderCommandType type, UInt32 index);

    template<class T>
    void AddCommandWithIndex(ScriptableRenderCommandType type, T* data, UInt32 index);
private:
    struct Command
    {
        UInt32  type;
        UInt32  perTypeIndex; // Used for indexing commands put in external arrays. ~0 for commands allocated using the page allocator
        void*   data;
    };
    std::vector<Command>  m_Commands;
    std::vector<DrawRenderersCommand> m_DrawRenderersCommands;
    std::vector<RenderingCommandBuffer*> m_CommandBuffers;
    const std::vector<Camera*>* m_Cameras;
};


#endif //HUAHUOENGINE_SCRIPTABLERENDERCONTEXT_H
