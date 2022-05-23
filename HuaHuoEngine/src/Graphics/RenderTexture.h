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

template<class TransferFunction>
void RenderTexture::Transfer(TransferFunction& transfer)
{
    Super::Transfer(transfer);
    transfer.SetVersion(3);

//    transfer.Transfer(m_Desc.width, "m_Width");
//    transfer.Transfer(m_Desc.height, "m_Height");
//    transfer.Transfer(m_Desc.antiAliasing, "m_AntiAliasing");
//    transfer.Transfer(m_Desc.mipCount, "m_MipCount");
//    int depthFormat = m_Desc.depthFormat;
//    int colorFormat = m_Desc.colorFormat;
//    transfer.Transfer(depthFormat, "m_DepthFormat");
//    transfer.Transfer(colorFormat, "m_ColorFormat");
//
//    bool mipmap = HasFlag(m_Desc.flags,  kRTFlagMipMap);
//    bool autoGenerateMips = HasFlag(m_Desc.flags, kRTFlagAutoGenerateMips);
//    bool srgb = HasFlag(m_Desc.flags, kRTFlagSRGB);
//    bool dynamicallyScale = HasFlag(m_Desc.flags, kRTFlagDynamicallyScalable);
//    bool bindMS = HasFlag(m_Desc.flags, kRTFlagBindMS);
//    bool enableCompatibleFormat = !HasFlag(m_Desc.flags, kRTFlagDisableCompatibleFormat);
//    transfer.Transfer(mipmap, "m_MipMap");
//    transfer.Transfer(autoGenerateMips, "m_GenerateMips");
//    transfer.Transfer(srgb, "m_SRGB");
//    transfer.Transfer(dynamicallyScale, "m_UseDynamicScale");
//    transfer.Transfer(bindMS, "m_BindMS");
//    transfer.Transfer(enableCompatibleFormat, "m_EnableCompatibleFormat");
//
//    if (transfer.IsReading())
//    {
//        m_Desc.flags = SetOrClearFlags(m_Desc.flags, kRTFlagMipMap, mipmap);
//        m_Desc.flags = SetOrClearFlags(m_Desc.flags, kRTFlagAutoGenerateMips, autoGenerateMips);
//        m_Desc.flags = SetOrClearFlags(m_Desc.flags, kRTFlagSRGB, srgb);
//        m_Desc.flags = SetOrClearFlags(m_Desc.flags, kRTFlagDynamicallyScalable, dynamicallyScale);
//        m_Desc.flags = SetOrClearFlags(m_Desc.flags, kRTFlagBindMS, bindMS);
//        m_Desc.flags = SetOrClearFlags(m_Desc.flags, kRTFlagDisableCompatibleFormat, !enableCompatibleFormat);
//
//        if (transfer.IsVersionSmallerOrEqual(1))
//            m_Desc.colorFormat = GetGraphicsFormat(static_cast<RenderTextureFormat>(colorFormat), srgb ? kRTReadWriteSRGB : kRTReadWriteLinear);
//        else
//            m_Desc.colorFormat = static_cast<GraphicsFormat>(colorFormat);
//
//        if (GetActiveColorSpace() == kLinearColorSpace && srgb)
//        {
//            m_Desc.colorFormat = GetSRGBFormat(m_Desc.colorFormat);
//            m_Desc.flags = SetOrClearFlags(m_Desc.flags, kRTFlagSRGB, true);
//        }
//        else
//            m_Desc.colorFormat = GetLinearFormat(m_Desc.colorFormat);
//
//        m_Desc.depthFormat = static_cast<DepthBufferFormat>(depthFormat);
//    }
//
//    transfer.Align();
//    TRANSFER(m_TextureSettings);
//    TRANSFER_ENUM_WITH_NAME(m_Desc.dimension, "m_Dimension");
//    TRANSFER_WITH_NAME(m_Desc.volumeDepth, "m_VolumeDepth");
}


#endif //HUAHUOENGINE_RENDERTEXTURE_H
