//
// Created by VincentZhang on 5/25/2022.
//

#include "GfxContextGLES.h"

namespace gl
{
    ContextHandle GetCurrentContext();
}//namespace gl

GfxContextGLES::Instance::Instance(ApiGLES & Api)
        : m_Framebuffer(Api, this)
        , m_DefaultVertexArrayName(GetGraphicsCaps().gles.hasVertexArrayObject ? gGL->CreateVertexArray() : gl::VertexArrayHandle())
{}

GfxContextGLES::Instance::~Instance()
{
    if (GetGraphicsCaps().gles.hasVertexArrayObject && m_DefaultVertexArrayName != gl::VertexArrayHandle::Default())
        gGL->DeleteVertexArray(m_DefaultVertexArrayName);
}

GfxFramebufferGLES& GfxContextGLES::Instance::GetFramebuffer()
{
    return m_Framebuffer;
}

const GfxFramebufferGLES& GfxContextGLES::Instance::GetFramebuffer() const
{
    return m_Framebuffer;
}

void GfxContextGLES::Instance::Invalidate()
{
    m_Framebuffer.Invalidate();
    gGL->BindVertexArray(m_DefaultVertexArrayName);
}