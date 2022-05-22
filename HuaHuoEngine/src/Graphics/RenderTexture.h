//
// Created by VincentZhang on 5/22/2022.
//

#ifndef HUAHUOENGINE_RENDERTEXTURE_H
#define HUAHUOENGINE_RENDERTEXTURE_H
#include "Texture.h"

class EXPORT_COREMODULE RenderTexture : public Texture{
    REGISTER_CLASS(RenderTexture);
    DECLARE_OBJECT_SERIALIZE();
public:
    RenderTexture(MemLabelId label, ObjectCreationMode mode);
    // virtual ~RenderTexture (); declared-by-macro
};


#endif //HUAHUOENGINE_RENDERTEXTURE_H
