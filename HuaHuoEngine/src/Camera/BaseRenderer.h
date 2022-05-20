//
// Created by VincentZhang on 5/13/2022.
//

#ifndef HUAHUOENGINE_BASERENDERER_H
#define HUAHUOENGINE_BASERENDERER_H

#include "Graphics/RendererType.h"
#include "Modules/ExportModules.h"
#include "Shaders/ShaderPropertySheet.h"
#include "SharedRenderData.h"

class EXPORT_COREMODULE BaseRenderer {
public:
    BaseRenderer(RendererType type);
    virtual ~BaseRenderer();

    virtual int GetLayer() const = 0;

protected:
    SharedRendererData      m_RendererData;
    ShaderPropertySheet*    m_RendererProperties;
};


#endif //HUAHUOENGINE_BASERENDERER_H
