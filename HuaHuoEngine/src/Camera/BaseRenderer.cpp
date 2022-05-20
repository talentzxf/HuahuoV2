//
// Created by VincentZhang on 5/13/2022.
//

#include "BaseRenderer.h"
#include <cstdlib>
#include "Logging/LogAssert.h"

BaseRenderer::BaseRenderer(RendererType type)
        : m_RendererData(type)
        , m_RendererProperties(NULL)
{
}

BaseRenderer::~BaseRenderer()
{
    DebugAssert(m_RendererProperties == NULL);
}