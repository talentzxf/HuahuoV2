//
// Created by VincentZhang on 5/17/2022.
//

#ifndef HUAHUOENGINE_RENDERER_H
#define HUAHUOENGINE_RENDERER_H


#include "Components/BaseComponent.h"
#include "Camera/BaseRenderer.h"

class Renderer : public BaseComponent, public BaseRenderer{
    REGISTER_CLASS_TRAITS(kTypeIsAbstract);
    REGISTER_CLASS(Renderer);
    DECLARE_OBJECT_SERIALIZE();
public:
    Renderer(RendererType type, /*MemLabelId label,*/ ObjectCreationMode mode);
    virtual int GetLayer() const override;
private:
    bool                m_Enabled;
};


#endif //HUAHUOENGINE_RENDERER_H
