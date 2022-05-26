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

    virtual bool        HasMipMap() const                       { return m_MipCount != 1; }
    virtual int         GetMipmapCount() const                  { return m_MipCount; }

public:
    Texture(MemLabelId label, ObjectCreationMode mode);
    virtual void        MainThreadCleanup() override;

protected:
    int                     m_MipCount;
};


#endif //HUAHUOENGINE_TEXTURE_H
