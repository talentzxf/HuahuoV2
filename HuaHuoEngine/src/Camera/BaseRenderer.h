//
// Created by VincentZhang on 5/13/2022.
//

#ifndef HUAHUOENGINE_BASERENDERER_H
#define HUAHUOENGINE_BASERENDERER_H

#include "Graphics/RendererType.h"
#include "Modules/ExportModules.h"
#include "Shaders/ShaderPropertySheet.h"
#include "SharedRenderData.h"
#include "Shaders/Material.h"
#include "BaseClasses/PPtr.h"

class EXPORT_COREMODULE BaseRenderer {
public:
    BaseRenderer(RendererType type);
    virtual ~BaseRenderer();

    virtual int GetLayer() const = 0;

    TransformInfo& GetWritableTransformInfo() { return m_RendererData.m_TransformInfo; }

    virtual int GetMaterialCount() const = 0;
    virtual PPtr<Material> GetMaterial(int i) const = 0;
    virtual int GetStaticBatchIndex() const { return 0; }

protected:
    SharedRendererData      m_RendererData;
    ShaderPropertySheet*    m_RendererProperties;
};


#endif //HUAHUOENGINE_BASERENDERER_H
