//
// Created by VincentZhang on 5/22/2022.
//

#ifndef HUAHUOENGINE_TEXTURE_H
#define HUAHUOENGINE_TEXTURE_H
#include "Modules/ExportModules.h"
#include "BaseClasses/NamedObject.h"

class EXPORT_COREMODULE Texture : public NamedObject{
    REGISTER_CLASS_TRAITS(kTypeIsAbstract);
    REGISTER_CLASS(Texture);
    DECLARE_OBJECT_SERIALIZE();
public:
    // Global texture anisotropy levels
    enum AnisotropicFiltering
    {
        kAnisoDisable = 0,
        kAnisoPerTexture,
        kAnisoForceEnable,
        kAnisoCount
    };

public:
    Texture(MemLabelId label, ObjectCreationMode mode);
    virtual void        MainThreadCleanup() override;
};


#endif //HUAHUOENGINE_TEXTURE_H
