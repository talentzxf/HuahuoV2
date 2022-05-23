//
// Created by VincentZhang on 5/22/2022.
//

#include "RenderTexture.h"

IMPLEMENT_REGISTER_CLASS(RenderTexture, 84);
IMPLEMENT_OBJECT_SERIALIZE(RenderTexture);

RenderTexture::RenderTexture(MemLabelId label, ObjectCreationMode mode)
        :   Super(label, mode)
//        ,   m_RenderTexturesNode(this)
//        ,   m_BindMultisampledOnSecondaryTex(false)
//        ,   m_AllocateColorWithVRDevice(false)
//        ,   m_AllocateDepthWithVRDevice(false)
//        ,   m_RenderBufferManagerData(NULL)
//        ,   m_UVYIsFromTopToBottom(!GetGraphicsCaps().usesOpenGLTextureCoords)
{
//    GetSettings().SetWrap(kTexWrapClamp);
//
//    // We use unchecked version since we may not be on the main thread
//    // This means CreateTextureID() implementation must be thread safe!
//    m_SecondaryTexID = GetUncheckedRealGfxDevice().CreateTextureID();
//    m_SecondaryTexIDUsed = false;
}