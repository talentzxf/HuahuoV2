//
// Created by VincentZhang on 5/22/2022.
//

#ifndef HUAHUOENGINE_RENDERTEXTURE_H
#define HUAHUOENGINE_RENDERTEXTURE_H
#include "Texture.h"
#include "GfxDevice/GfxDeviceObjects.h"
#include "RenderTextureDesc.h"
#include "GfxDevice/GfxDevice.h"
#include "Shaders/GraphicsCaps.h"

class EXPORT_COREMODULE RenderTexture : public Texture{
    REGISTER_CLASS(RenderTexture);
    DECLARE_OBJECT_SERIALIZE();
public:
    enum SetActiveFlags
    {
        kFlagNone               = 0,
        kFlagDontSetViewport    = (1 << 0),
        kFlagForceResolve       = (1 << 1),    // Force resolve to texture: Used by MSAA targets and Xbox 360
        kFlagDontRestoreColor   = (1 << 2),    // Xbox 360 specific: do not restore old contents to EDRAM
        kFlagDontRestoreDepth   = (1 << 3),    // Xbox 360 specific: do not restore old contents to EDRAM
        kFlagDontRestore        = kFlagDontRestoreColor | kFlagDontRestoreDepth,
        kFlagForceSetRT         = (1 << 4),   // When used in SetActive call will enforce setting RT disregarding any caching involved
        kFlagDontResolve        = (1 << 5),   // Skip AA resolve even when the previous RT was AA. Used when doing shenanigans with default backbuffers.
        kFlagReadOnlyDepth      = (1 << 6),   // Bind the depth surface as read-only
        kFlagReadOnlyStencil    = (1 << 7),   // Bind the stencil surface as read-only
    };
    ENUM_FLAGS_AS_MEMBER(SetActiveFlags);

    RenderTexture(MemLabelId label, ObjectCreationMode mode);
    // virtual ~RenderTexture (); declared-by-macro

    enum CreateFlags
    {
        kCreateFlagNone = 0,
        kCreateFlagAlreadyAdjustedDesc = 1 << 0,
    };
    ENUM_FLAGS_AS_MEMBER(CreateFlags);

    // Creates the render texture.
    // Create is automatically called inside Activate the first time.
    bool Create(CreateFlags flags = kCreateFlagNone);

    // Similar to the default Create(). The only difference is that this version allows reuse of an already
    // created color and/or depth surface. This can lead to memory savings if those surfaces are just intermediates
    // that can be shared across multiple render textures.
    bool Create(const RenderSurfaceHandle& colorHandle, const RenderSurfaceHandle& depthHandle, CreateFlags flags = kCreateFlagNone);

    // Is the render texture created?
    bool IsCreated() const { return m_ColorHandle.IsValid() || m_DepthHandle.IsValid(); }

    RenderSurfaceHandle GetColorSurfaceHandle()         { return m_ColorHandle; }
    RenderSurfaceHandle GetResolvedColorSurfaceHandle() { return m_ResolvedColorHandle; }
    RenderSurfaceHandle GetDepthSurfaceHandle()         { return m_DepthHandle; }
    // RenderSurfaceHandle GetColorSurfaceHandleNoAA()     { return IsAntiAliased() ? m_ResolvedColorHandle : m_ColorHandle; }

    // Makes the render texture the current render target. If texture is NULL the back buffer is activated.
    static void SetActive(RenderTexture* texture, int mipLevel, CubemapFace face, int depthSlice, SetActiveFlags flags);
    static bool SetActive(int count, RenderSurfaceHandle* colors, RenderSurfaceHandle depth, RenderTexture** texture, int mipLevel, CubemapFace face, int depthSlice, SetActiveFlags flags);
    static bool SetActive(const GfxRenderTargetSetup& rt, RenderTexture** texture, SetActiveFlags flags);
    static void SetBackbufferActive() { SetActive(NULL, 0, kCubeFaceUnknown, 0, kFlagNone); }

    static GfxRenderTargetSetup MakeRenderTargetSetup(RenderTexture* texture, int mipLevel, CubemapFace face, int depthSlice, SetActiveFlags flags);
    static GfxRenderTargetSetup MakeRenderTargetSetup(int count, RenderSurfaceHandle* colors, RenderSurfaceHandle depth, int mipLevel, CubemapFace face, int depthSlice, SetActiveFlags flags);

    void SetWidth(int width);
    int GetWidth() const { return m_Desc.width; }
    int GetScaledWidth() const;

    void SetHeight(int height);
    int GetHeight() const { return m_Desc.height; }
    int GetScaledHeight() const;

    static bool AdjustDescForGraphicsCaps(RenderTextureDesc& desc, GfxDevice& device, const GraphicsCaps& caps, RenderTexture* texture);

    GraphicsFormat GetColorFormat() const;
    void SetColorFormat(GraphicsFormat format);
private:
    static bool IsDescPowerOfTwo(const RenderTextureDesc& desc) { return IsPowerOfTwo(desc.width) && IsPowerOfTwo(desc.height); }
    void DestroySurfaces();

private:
    RenderTextureDesc m_Desc;
    RenderSurfaceHandle m_ColorHandle;
    RenderSurfaceHandle m_ResolvedColorHandle;
    RenderSurfaceHandle m_DepthHandle;
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

// return or passed tex (but created if needed) or NULL if RT cannot be created by any reason
RenderTexture* EnsureRenderTextureIsCreated(RenderTexture* tex);

#endif //HUAHUOENGINE_RENDERTEXTURE_H
