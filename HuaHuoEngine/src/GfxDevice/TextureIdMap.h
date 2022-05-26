//
// Created by VincentZhang on 5/26/2022.
//

#ifndef HUAHUOENGINE_TEXTUREIDMAP_H
#define HUAHUOENGINE_TEXTUREIDMAP_H


#include <cstdint>
#include "GfxDeviceTypes.h"

class TextureIdMap {
public:
    static intptr_t QueryNativeTexture(TextureID texid);
};

inline intptr_t TextureIdMap::QueryNativeTexture(TextureID texid)
{
//#if GFX_USE_POINTER_IN_RESOURCE_IDS
    return texid.m_ID;
//#else
//    return ms_IDMap.GetResource(texid.m_ID);
//#endif
}


#endif //HUAHUOENGINE_TEXTUREIDMAP_H
