//
// Created by VincentZhang on 5/22/2022.
//

#include "Texture.h"


///@todo: texture should not allocate memory based on texture base level.

Texture::Texture(MemLabelId label, ObjectCreationMode mode) :
        Super(label, mode) //,
//        m_TexData(NULL),
//        m_TexelSizeX(1.0f),
//        m_TexelSizeY(1.0f),
//        m_MipCount(1),
//        m_StreamData(),
//        m_UpdateCount(0u)
#if SUPPORT_THREADS
, m_AsyncCreateMainThreadFence(0)
    , m_AsyncUploadMainThreadFence(0)
#endif
{
//    // We use unchecked version since we may not be on the main thread
//    m_TexID = GetUncheckedRealGfxDevice().CreateTextureID();
//    m_UsageMode = kTexUsageNone;
//    m_ColorSpace = kTexColorSpaceLinear;
//    m_RefCount = 0;
//    m_DownscaleFallback = false;
//    m_IsAlphaChannelOptional = false;
//    m_ForcedFallbackFormat = kTexFormatRGBA32;
}

void Texture::ThreadedCleanup()
{
//    // Additional cleanup needed in case MainThreadCleanup() was not called, which can happen when:
//    // - initialization fails, see InitTextureInternal()
//    // - the texture was created on a worker thread and never got integrated (didn't get a valid InstanceID and AwakeFromLoad() was not invoked)
//    {
//        if (m_TexID.IsValid())
//        {
//            GetUncheckedRealGfxDevice().FreeTextureID(m_TexID);
//            m_TexID = TextureID();
//        }
//    }
}

template<class TransferFunction>
void Texture::Transfer(TransferFunction& transfer)
{
    Super::Transfer(transfer);
//    TRANSFER_EDITOR_ONLY(m_ImageContentsHash);
//    TRANSFER_ENUM(m_ForcedFallbackFormat);
//    TRANSFER(m_DownscaleFallback);
//    TRANSFER(m_IsAlphaChannelOptional);

    transfer.Align();
}

// REGISTER_TYPE_ATTRIBUTES(Texture, (HasNoReferences, ()));
IMPLEMENT_REGISTER_CLASS(Texture, 27);
IMPLEMENT_OBJECT_SERIALIZE(Texture);
INSTANTIATE_TEMPLATE_TRANSFER(Texture);
