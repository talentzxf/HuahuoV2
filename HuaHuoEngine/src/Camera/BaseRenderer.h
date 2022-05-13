//
// Created by VincentZhang on 5/13/2022.
//

#ifndef HUAHUOENGINE_BASERENDERER_H
#define HUAHUOENGINE_BASERENDERER_H

#include "Graphics/RendererType.h"
#include "Modules/ExportModules.h"

class EXPORT_COREMODULE BaseRenderer {
public:
    BaseRenderer(RendererType type);
    virtual ~BaseRenderer();
};


#endif //HUAHUOENGINE_BASERENDERER_H
