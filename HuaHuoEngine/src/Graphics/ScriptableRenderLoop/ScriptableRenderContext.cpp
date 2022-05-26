//
// Created by VincentZhang on 5/14/2022.
//

#include "ScriptableRenderContext.h"
#include "Export/Rendering/RenderPipelineManager.h"
#include "Logging/LogAssert.h"
#include "Math/Color.h"
#include "GfxDevice/GfxDeviceTypes.h"
#include "Graphics/CommandBuffer/RenderingCommandBuffer.h"
#include "GfxDevice/GfxDevice.h"
#include "Camera/RenderNodeQueue.h"
#include "Camera/BaseRenderer.h"
#include "Camera/RendererScene.h"

Camera* ScriptableRenderContext::GetCamera(int index)
{
    if (m_Cameras == NULL)
    {
        LogString("Trying to access invalid active cameras pointer");
        return NULL;
    }

    if (index < 0 || index >= (*m_Cameras).size())
    {
        LogString(Format("Index %d is out of bounds", index));
        return NULL;
    }

    return (*m_Cameras)[index];
}

int ScriptableRenderContext::GetNumberOfCameras()
{
    if (m_Cameras == NULL)
    {
        LogString("Trying to access invalid active cameras pointer");
        return NULL;
    }

    return m_Cameras->size();
}

void ScriptableRenderContext::AddCommandWithIndex(ScriptableRenderCommandType type, UInt32 index)
{
    AddCommandWithIndex(type, (void*)NULL, index);
}

template<class T>
void ScriptableRenderContext::AddCommandWithIndex(ScriptableRenderCommandType type, T* data, UInt32 index)
{
    Command command;
    command.type = type;
    command.perTypeIndex = index;
    command.data = data;

    m_Commands.push_back(command);
}

void ScriptableRenderContext::ExecuteCommandBuffer(RenderingCommandBuffer& commandBuffer)
{
#if UNITY_EDITOR
    commandBuffer.ValidateForSRP(m_IsInsideRenderPassForValidation);
#endif

    // Copy the command buffer to allow the passed in one to be cleared and reused
    MemLabelRef memLabel = commandBuffer.GetMemLabel();
    RenderingCommandBuffer* clonedCommandBuffer = HUAHUO_NEW(RenderingCommandBuffer, memLabel)(memLabel, commandBuffer);

    UInt32 index = static_cast<UInt32>(m_CommandBuffers.size());
    m_CommandBuffers.push_back(clonedCommandBuffer);
    AddCommandWithIndex(kScriptRenderCommand_ExecuteCommandBuffer, index);
}

void ScriptableRenderContext::DrawRenderers(ScriptableCullResults *cullResults, const DrawingSettings& drawSettings, const FilteringSettings& filterSettings, ShaderTagID tagName, bool isPassTagName, ShaderTagID const *tagValues, RenderStateBlock const *stateBlocks, int stateCount){
    UInt32 index = static_cast<UInt32>(m_DrawRenderersCommands.size());
    DrawRenderersCommand& cmd = m_DrawRenderersCommands.emplace_back();
    cmd.drawSettings = drawSettings;
    //cmd.cullResults = cullResults;
    cmd.filterSettings = filterSettings;
    cmd.stateCount = stateCount;

    if (stateCount > 0)
    {
        cmd.tagName = tagName;
        cmd.isPassTagName = isPassTagName;

        // tagValues and stateBlocks ownership is transferred to ScriptableRenderContextArg
        cmd.stateTagValues = reinterpret_cast<ShaderTagID*>(HUAHUO_MALLOC_ALIGNED(kMemTempJobAlloc, sizeof(ShaderTagID) * stateCount, alignof(ShaderTagID)));
        cmd.stateBlocks = reinterpret_cast<RenderStateBlock*>(HUAHUO_MALLOC_ALIGNED(kMemTempJobAlloc, sizeof(RenderStateBlock) * stateCount, alignof(RenderStateBlock)));
        memcpy(cmd.stateTagValues, tagValues, sizeof(ShaderTagID) * stateCount);
        memcpy(cmd.stateBlocks, stateBlocks, sizeof(RenderStateBlock) * stateCount);
    }
    else
    {
        cmd.stateTagValues = nullptr;
        cmd.stateBlocks = nullptr;
    }

    AddCommandWithIndex(kScriptRenderCommand_DrawRenderers, index);
}

void HuaHuoRenderPipeline(ScriptableRenderContext* pContext, const std::vector<Camera *> &cameras){
    RendererScene& rendererScene = GetRendererScene();
    int rendererCount = rendererScene.GetNumRenderers();

    ShaderPassContext& passContext = GetDefaultPassContext();

    for(Camera* camera:cameras){
        // 1. Setup the camera
        camera->SetupRender(passContext);

        // 2. Render objects
        for(int i = 0; i < rendererCount; i++ ){
            const SceneNode& renderNode = rendererScene.GetRendererNode(i);
            BaseRenderer* renderer = renderNode.renderer;
            renderer->executeCallBack(renderer);
        }
    }
}

void ScriptableRenderContext::ExtractAndExecuteRenderPipeline(const std::vector<Camera *> &cameras,
                                                              PostProcessCullResults *postProcessCullResults,
                                                              void *postProcessCullResultsData) {

    m_Cameras = &cameras;
    RenderPipeline* renderPipeline = GetRenderPipelineManager()->GetRenderPipeline();
    if(renderPipeline != NULL){
        renderPipeline->Render(this);
    } else {// VZ: Really have to write own renderer, the original implementation is really really complicated ......
        HuaHuoRenderPipeline(this, cameras);
    }
}

void ScriptableRenderContext::Submit()
{
    ExecuteScriptableRenderLoop();
}

void ScriptableRenderContext::CleanupCommandBuffers()
{
    for (size_t i = 0; i < m_CommandBuffers.size(); ++i)
    {
        // We release the command buffer here and since we are the only one holding on to it it will be deleted at this point.
        m_CommandBuffers[i]->Release();
    }
}

// Three steps: 1. Prepare. 2. Schedule jobs.  3. Cleanup.
void ScriptableRenderContext::ExecuteScriptableRenderLoop()
{
    ShaderPassContext& defaultContext = GetDefaultPassContext();

    JobBatchDispatcher dispatcher;

    // Batch schedule prepare draw renderer jobs
    // Overlap with Shadow render node queue flattening as we don't access Renderers at all when doing PrepareDrawRenderers.
    std::vector<ScriptableRenderContextArg*> drawRenderersData;//(kMemTempAlloc);
    drawRenderersData.resize(m_DrawRenderersCommands.size());
    for (UInt32 i = 0; i != m_DrawRenderersCommands.size(); i++)
        drawRenderersData[i] = PrepareDrawRenderersCommand(m_DrawRenderersCommands[i], dispatcher);


    // execute commands
    for (UInt32 i = 0; i != m_Commands.size(); i++) {
        ScriptableRenderCommandType cmdType = (ScriptableRenderCommandType) m_Commands[i].type;
        switch (cmdType) {
            case kScriptRenderCommand_DrawRenderers: {
                ScriptableRenderContextArg *drawRenderers = drawRenderersData[m_Commands[i].perTypeIndex];
                ExecuteDrawRenderersCommand(drawRenderers);
                break;
            }
            case kScriptRenderCommand_ExecuteCommandBuffer:
            {
//                RenderingCommandBuffer* commandBuffer = m_CommandBuffers[m_Commands[i].perTypeIndex];
//                RenderNodeQueue tempEmptyQueue;
//                commandBuffer->ExecuteCommandBuffer(defaultContext, tempEmptyQueue,
//                                                    RenderingCommandBuffer::kFlagsKeepState, &m_tempRTs,
//                                                    kGPUQueueComputeOnGraphics,
//                                                    (m_ExtractedCameraProperties.m_CameraTargetValid ? m_ExtractedCameraProperties.m_CameraCurrentTarget : NULL));
//                tempEmptyQueue.SyncDependentJobs();
                break;
            }
            default:
                LogString(Format("Unknown RenderLoop command: %d", cmdType));
                break;
        }
    }


//    for (UInt32 i = 0; i != drawShadowData.size(); i++)
//        CleanupDrawShadowsCommand(drawShadowData[i]);

    for (UInt32 i = 0; i != m_DrawRenderersCommands.size(); i++)
        CleanupDrawRenderersCommand(m_DrawRenderersCommands[i]);

    m_Commands.clear();
    // m_DrawShadowCommands.clear_dealloc();
    m_DrawRenderersCommands.clear();
    CleanupCommandBuffers();                // the following m_CommandBuffers.clear(); does NOT release RenderingCommandBuffer memory and produces memory leak each frame in any SRP render loop (about 0.01GiB per second)
    m_CommandBuffers.clear();
//    m_PageAllocator.Clear();
//    m_Allocator.Initialize(m_PageAllocator);
}