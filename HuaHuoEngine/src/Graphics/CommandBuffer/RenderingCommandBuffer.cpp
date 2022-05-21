//
// Created by VincentZhang on 5/17/2022.
//

#include "RenderingCommandBuffer.h"
#include "RenderingCommandBufferTypes.h"

void RenderingCommandBuffer::AddClearRenderTarget(GfxClearFlags clearFlags, const ColorRGBAf& color, float depth, UInt32 stencil)
{
    RenderCommandClearRT cmd = { color, clearFlags, depth, stencil };
    m_Buffer.WriteValueType<RenderCommandType>(kRenderCommand_ClearRT);
    m_Buffer.WriteValueType(cmd);
}

void RenderingCommandBuffer::Release() {

}


RenderingCommandBuffer::RenderingCommandBuffer(MemLabelRef label)
        :   ThreadSharedObject<RenderingCommandBuffer>(label)
        ,   m_Buffer(label)
//        ,   m_PropertySheets(label)
//        ,   m_NodesToResolve(label)
//        ,   m_Textures(label)
//        ,   m_Materials(label)
//        ,   m_RenderTextures(label)
//        ,   m_CanRunOnJobs(true)
//        ,   m_Name("Unnamed command buffer")
//        ,   m_TargetQueue(kGPUQueueComputeOnGraphics)
//        ,   m_ExecutionFlags(kRCBExecutionFlagsNone)
//        ,   m_RequiresStereoTarget(false)
//        ,   m_PvtCompleteHandles(NULL)
{
}

RenderingCommandBuffer::RenderingCommandBuffer(MemLabelRef label, const RenderingCommandBuffer& other)
        : ThreadSharedObject<RenderingCommandBuffer>(label)
        , m_Buffer(other.m_Buffer)
//        , m_PropertySheets(other.m_PropertySheets)
//        , m_NodesToResolve(other.m_NodesToResolve)
//        , m_Textures(other.m_Textures)
//        , m_Materials(other.m_Materials)
//        , m_RenderTextures(other.m_RenderTextures)
//        , m_CanRunOnJobs(other.m_CanRunOnJobs)
//        , m_Name(other.m_Name)
//        , m_TargetQueue(kGPUQueueComputeOnGraphics)
//        , m_ExecutionFlags(other.m_ExecutionFlags)
//        , m_RequiresStereoTarget(false)
//        , m_PvtCompleteHandles(NULL)
{
//    for (size_t i = 0, n = m_PropertySheets.size(); i != n; ++i)
//    {
//        if (m_PropertySheets[i])
//            m_PropertySheets[i]->AddRef();
//    }
//
//    for (GPUFenceArray::const_iterator fenceItr = other.m_ReferencedFences.begin(); fenceItr != other.m_ReferencedFences.end(); ++fenceItr)
//    {
//        m_ReferencedFences.push_back(*fenceItr);
//        m_ReferencedFences.back()->AddRef();
//    }
//
//    // Give ownership of GCHandles for async readback callbacks to the new command buffer
//    const size_t handleCount = other.m_AsyncReadbackScriptingCallbacks.size();
//    m_AsyncReadbackScriptingCallbacks.resize_uninitialized(handleCount);
//    for (size_t i = 0; i < handleCount; ++i)
//        m_AsyncReadbackScriptingCallbacks[i] = ScriptingGCHandle(other.m_AsyncReadbackScriptingCallbacks[i].Resolve(), GCHANDLE_STRONG);
}

RenderingCommandBuffer::~RenderingCommandBuffer()
{
    // ClearCommands();
}