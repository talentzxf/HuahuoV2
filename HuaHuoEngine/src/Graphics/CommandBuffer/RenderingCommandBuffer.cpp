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