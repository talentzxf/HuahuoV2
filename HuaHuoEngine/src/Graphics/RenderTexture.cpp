//
// Created by VincentZhang on 5/22/2022.
//

#include "RenderTexture.h"
#include "GfxDevice/GfxDevice.h"
#include "Shaders/GraphicsCaps.h"
#include "Utilities/Utility.h"
#include "Graphics/GraphicsHelper.h"
#include "Graphics/RenderTextureMap.h"
#include "Camera/CameraUtil.h"

static void RenderTexture_DestroySurfaceHelper(GfxDevice& device, RenderSurfaceHandle& rs, RenderTexture* originRT)
{
    if (rs.IsValid())
    {
        // Camera::OnRenderSurfaceDestroyed(rs, originRT);
        device.DestroyRenderSurface(rs);
        rs.Reset();
    }
}

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

bool RenderTexture::Create(CreateFlags flags)
{
    // __FAKEABLE_METHOD_OVERLOADED__(RenderTexture, Create, (flags), bool(CreateFlags));

    RenderSurfaceHandle emptyHandle;
    return Create(emptyHandle, emptyHandle, flags);
}

void RenderTexture::DestroySurfaces() {
    if (!IsCreated())
        return;

//    RenderTextureMap::Remove(m_ColorHandle.object);
//    RenderTextureMap::Remove(m_DepthHandle.object);

    GfxDevice& device = GetGfxDevice();
//  device.GetFrameStats().ChangeRenderTextureBytes(-(SInt64)GetRuntimeMemorySize());

//    if (m_StencilTexID.IsValid())
//    {
//        device.DestroyStencilViewPlatform(m_StencilTexID, m_DepthHandle.object);
//    }

    RenderTexture_DestroySurfaceHelper(device, m_ColorHandle, this);
    RenderTexture_DestroySurfaceHelper(device, m_ResolvedColorHandle, this);
    RenderTexture_DestroySurfaceHelper(device, m_DepthHandle, this);
}


GfxRenderTargetSetup RenderTexture::MakeRenderTargetSetup(RenderTexture* texture, int mipLevel, CubemapFace face, int depthSlice, SetActiveFlags flags)
{
    texture = EnsureRenderTextureIsCreated(texture);

    RenderSurfaceHandle newColorSurface = texture ? texture->m_ColorHandle : GetGfxDevice().GetBackBufferColorSurface();
    RenderSurfaceHandle newDepthSurface = texture ? texture->m_DepthHandle : GetGfxDevice().GetBackBufferDepthSurface();
    int mips = 0;
    if (texture && texture->HasMipMap())
        mips = clamp(mipLevel, 0, texture->GetMipmapCount() - 1);
    return MakeRenderTargetSetup(1, &newColorSurface, newDepthSurface, mips, face, depthSlice, flags);
}

GfxRenderTargetSetup RenderTexture::MakeRenderTargetSetup(const int count, RenderSurfaceHandle* const newColorSurfaces, const RenderSurfaceHandle newDepthSurface, const int mipLevel, const CubemapFace face, const int depthSlice, const SetActiveFlags flags)
{
    // If the user actually wanted to bind a cube face, then we cannot bind all the slices of the texture.
    int localDepthSlice = (depthSlice == -1 && face != kCubeFaceUnknown) ? 0 : depthSlice;

    GfxRenderTargetSetup setup;
    ::memset(&setup, 0x00, sizeof(GfxRenderTargetSetup));

    setup.colorCount = count;
    for (int i = 0, n = setup.colorCount; i < n; ++i)
    {
        setup.color[i] = newColorSurfaces[i].object ? newColorSurfaces[i].object : GetGfxDevice().GetBackBufferColorSurface().object;

        setup.colorLoadAction[i] = (flags & kFlagDontRestoreColor) ? kGfxRTLoadActionDontCare : setup.color[i]->loadAction;
        setup.colorStoreAction[i] = setup.color[i]->storeAction;
        // preserve old discard behaviour: render surface flags are applied only on first activation
        if (!HasFlag(setup.color[i]->flags, kSurfaceCreateMemoryless))
            setup.color[i]->loadAction = setup.color[i]->storeAction = 0;
    }
    {
        setup.depth = newDepthSurface.object ? newDepthSurface.object : GetGfxDevice().GetBackBufferDepthSurface().object;

        setup.depthLoadAction = (flags & kFlagDontRestoreDepth) ? kGfxRTLoadActionDontCare : setup.depth->loadAction;
        setup.depthStoreAction = setup.depth->storeAction;
        // preserve old discard behaviour: render surface flags are applied only on first activation
        if (!HasFlag(setup.depth->flags, kSurfaceCreateMemoryless))
            setup.depth->loadAction = setup.depth->storeAction = 0;
    }

    setup.cubemapFace = face;
    setup.depthSlice = localDepthSlice;
    const int mipCount = setup.color[0]->backBuffer ? 1 : CalculateMipMapCount3D(setup.color[0]->width, setup.color[0]->height, 1);
    setup.mipLevel = clamp(mipLevel, 0, mipCount - 1);


    // TODO: That's kinda ridiculous
    UInt32 rtFlags = 0;
    rtFlags |= (flags & kFlagDontRestoreColor) ? GfxDevice::kFlagDontRestoreColor : 0;
    rtFlags |= (flags & kFlagDontRestoreDepth) ? GfxDevice::kFlagDontRestoreDepth : 0;
    rtFlags |= (flags & kFlagForceResolve) ? GfxDevice::kFlagForceResolve : 0;
    rtFlags |= (flags & kFlagForceSetRT) ? GfxDevice::kFlagForceSetRT : 0;
    rtFlags |= (flags & kFlagDontResolve) ? GfxDevice::kFlagDontResolve : 0;
    rtFlags |= (flags & kFlagReadOnlyDepth) ? GfxDevice::kFlagReadOnlyDepth : 0;
    rtFlags |= (flags & kFlagReadOnlyStencil) ? GfxDevice::kFlagReadOnlyStencil : 0;
    setup.flags = rtFlags;
    GraphicsHelper::ValidateMemoryless(setup);

    return setup;
}

void RenderTexture::SetActive(RenderTexture* newActive, int mipLevel, CubemapFace face, int depthSlice, SetActiveFlags flags)
{
    SetActive(MakeRenderTargetSetup(newActive, mipLevel, face, depthSlice, flags), &newActive, flags);
}

bool RenderTexture::SetActive(int count, RenderSurfaceHandle* newColorSurfaces, RenderSurfaceHandle newDepthSurface, RenderTexture** rt, int mipLevel, CubemapFace face, int depthSlice, SetActiveFlags flags)
{
    return SetActive(MakeRenderTargetSetup(count, newColorSurfaces, newDepthSurface, mipLevel, face, depthSlice, flags), rt, flags);
}

static inline bool IsRenderSurfacePresentInSetup(const RenderSurfaceBase* rs, const GfxRenderTarget& setup)
{
    for (unsigned i = 0, n = setup.colorCount; i < n; ++i)
    {
        if (setup.color[i] == rs)
            return true;
    }
    return false;
}

RenderTexture* RenderTexture::GetActive(int index)
{
    RenderSurfaceHandle handle = GetGfxDevice().GetActiveRenderColorSurface(index);
    if (handle.IsValid())
        return RenderTextureMap::Query(handle.object);

    handle = GetGfxDevice().GetActiveRenderDepthSurface();
    if (handle.IsValid())
        return RenderTextureMap::Query(handle.object);

    return NULL;
}

void RenderTexture::ResolveAntiAliasedSurface()
{
    if (!m_ResolvedColorHandle.IsValid())
        return;

    // PROFILER_AUTO_GFX(gResolveAA, this);
    GfxDevice& device = GetGfxDevice();
    device.ResolveColorSurface(m_ColorHandle, m_ResolvedColorHandle);
    /// GPU_TIMESTAMP();
}

static bool CheckRenderTargetSetup(const GfxRenderTargetSetup& setup)
{
    // Only do checking & nice logging of common RT setup errors in editor & development builds
#   if UNITY_DEVELOPER_BUILD

    // Currently we can't render into whole cubemap (depth slice -1) if it has a depth buffer
    if (setup.depthSlice < 0 && setup.cubemapFace == kCubeFaceUnknown)
    {
        const bool hasDepthBuffer = setup.depth && (setup.depth->flags & kSurfaceCreateNoDepth) == 0;
        if (hasDepthBuffer)
        {
            for (int rt = 0; rt < setup.colorCount; ++rt)
            {
                if (setup.color[rt] && setup.color[rt]->dim == kTexDimCUBE)
                {
                    WarningString("Setting active render target that is a Cubemap with a depth buffer, and active slice -1 is not currently supported.");
                    return false;
                }
            }
        }
    }

#   endif // if UNITY_DEVELOPER_BUILD

    return true;
}

bool RenderTexture::SetActive(const GfxRenderTargetSetup& setup, RenderTexture** texture, SetActiveFlags flags)
{
    DebugAssert(texture);
    const bool  backbuffer  = setup.color[0]->backBuffer;

    GfxDevice& device = GetGfxDevice();

    // we want to AA-resolve current targets if not present in new RT setup (or if forced)
    if (!(setup.flags & GfxDevice::kFlagDontResolve))
    {
        for (unsigned i = 0, n = device.GetActiveRenderTargetCount(); i < n; ++i)
        {
            RenderSurfaceBase* oldColor = device.GetActiveRenderColorSurface(i).object;
            if (!IsRenderSurfacePresentInSetup(oldColor, setup) || (setup.flags & GfxDevice::kFlagForceResolve))
            {
                RenderTexture* oldRt = RenderTexture::GetActive(i);
                if (oldRt && oldRt->IsAntiAliased() && !HasAnyFlags(oldRt->m_Desc.flags, kRTFlagNoResolvedColorSurface | kRTFlagBindMS))
                    oldRt->ResolveAntiAliasedSurface();
            }
        }
    }

    // error checking

    if (!CheckRenderTargetSetup(setup))
        return false;

    // Pass HDR Settings handle of the currently active display into render surface if not set,
    // might be used by GfxDevice and Hidden/BlitCopyHDRTonemap when using the built-in
    // tonemapping

//    if (backbuffer)
//    {
//        RenderSurfaceBase *color = setup.color[0];
//        if (!color->hdrSettings)
//            color->hdrSettings = GetScreenManager().GetHDROutputSettings();
//    }
//
//    // set render targets
//    if (texture && texture[0])
//        PROFILER_AUTO(gSetRenderTargets, texture[0]);

//    device.SetRenderTargets(setup);
//    GPU_TIMESTAMP();
//    Camera* curCam = GetRenderManager().GetCurrentCameraPtr();
//    int width = std::max<unsigned>(setup.color[0]->width >> setup.mipLevel, 1);
//    int height = std::max<unsigned>(setup.color[0]->height >> setup.mipLevel, 1);
//    bool isRenderingInStereo = GetRenderBufferManager().GetTextures().GetActiveVRUsage() != kVRTextureUsageNone;
//
//    if (setup.color[0]->flags & kSurfaceCreateDynamicScale)
//    {
//        width = ceil(width * ScalableBufferManager::GetInstance().GetWidthScaleFactor());
//        height = ceil(height * ScalableBufferManager::GetInstance().GetHeightScaleFactor());
//    }
//
//    // Setup the viewport for the render texture
//    if (!HasFlag(flags, kFlagDontSetViewport))
//    {
//        if ((backbuffer || isRenderingInStereo)
//
//                )
//        {
//            // When switching to main window, restore the viewport to the one of the current camera
//            // The corner case would be blitting from script out of normal Camera's rendering (e.g. end of frame)
//            // RenderManager always have "current camera" set, so we check if it is rendering now
//            RectInt cameraViewport;
//            if (curCam == NULL || curCam->IsCurrentlyRendering() == false)
//            {
//                if (isRenderingInStereo && texture && texture[0])
//                {
//                    cameraViewport.Set(0, 0, texture[0]->GetWidth(), texture[0]->GetHeight());
//                }
//                else
//                {
//#if SUPPORT_MULTIPLE_DISPLAYS
//                    const Rectf screenRect = GetScreenManager().GetRect(device.GetDisplayTarget());
//#else
//                    const Rectf screenRect = GetScreenManager().GetRect();
//#endif
//                    cameraViewport = RectfToRectInt(screenRect);
//                }
//            }
//            else
//            {
//                if (isRenderingInStereo && !backbuffer && texture)
//                {
//                    Rectf normalizedViewport = curCam->GetNormalizedViewportRect(texture[0], true);
//                    normalizedViewport.Scale(width, height);
//                    cameraViewport = RectfToRectInt(normalizedViewport);
//                }
//                else
//                {
//                    Rectf viewport = curCam->GetPhysicalViewportRect();
//                    cameraViewport = RectfToRectInt(viewport);
//                }
//            }
//            // Flip viewport just before applying to device, but don't store it flipped in the render manager
//            device.SetViewport(cameraViewport);
//        }
//        else
//        {
//            device.SetViewport(RectInt(0, 0, width, height));
//        }
//    }
//
//#if GFX_SUPPORTS_SINGLE_PASS_STEREO
//    // Stereo viewports always need to be set at the moment if relevant
//    {
//        SinglePassStereo singlePassStereo = device.GetSinglePassStereo();
//        if (singlePassStereo == kSinglePassStereoSideBySide)
//        {
//            Rectf origViewport;
//            if (isRenderingInStereo && curCam)
//            {
//                origViewport = curCam->GetNormalizedViewportRect();
//            }
//            else
//            {
//                origViewport.Set(0.0f, 0.0f, 1.0f, 1.0f);
//            }
//            for (int eyeIndex = kStereoscopicEyeDefault; eyeIndex < kStereoscopicEyeCount; ++eyeIndex)
//            {
//                Rectf normalizedViewport;
//                StereoscopicEye eye = (StereoscopicEye)eyeIndex;
//
//                Matrix4x4f stereoProjMatrix;
//                if (curCam)
//                    stereoProjMatrix = curCam->GetStereoProjectionMatrix(eye);
//                else
//                    device.GetStereoMatrix((MonoOrStereoscopicEye)eye, kShaderMatProj, stereoProjMatrix);
//
//                if (GetIVRDevice() && GetIVRDevice()->GetViewportForEye(eye, singlePassStereo, origViewport, normalizedViewport))
//                {
//                    normalizedViewport.Scale(width, height);
//
//                    RectInt viewportToRender(RoundfToInt(normalizedViewport.x), RoundfToInt(normalizedViewport.y), RoundfToInt(normalizedViewport.width), RoundfToInt(normalizedViewport.height));
//                    device.SetStereoViewport(eye, viewportToRender);
//                }
//            }
//        }
//    }
//#endif
//
//    // If texture coordinates go from top to bottom (D3D-style), then we need to flip projection matrix vertically when rendering
//    // into a texture.
//    RenderTexture* rt = texture[0];
//    bool flipProjection = !backbuffer && rt && rt->CalculateNeedsInvertedProjection();
//    device.SetInvertProjectionMatrix(flipProjection);
//
//    if (rt != NULL)
//        rt->SetUVYIsFromTopToBottom(!flipProjection);

    return true;
}

// This method is used to get the sRGB flag just before the actual creation of the RT (contrary to how it was done before, ie: in the sRGB flag setter)
// This allows us to not having to temper with the stored flag and thus make sure that everything works fine when switching gamma/linear. This also avoid problems with modifying the flag via SerializedProperties that don't go through get/set.
// In the end, it's still a pretty bad way to do it. The flag should be passed to the gfx device as is and then ignored when needed by gfx device depending on capabilities/player settings.
bool IsSRGBReadWriteRequired(const RenderTextureDesc& desc)
{
    return (HasFlag(desc.flags, kRTFlagSRGB) ); //&& GetActiveColorSpace() == kLinearColorSpace && !IsIEEE754Format(desc.colorFormat));
}

GraphicsFormat GetCompatibleFormat(const RenderTextureDesc& descriptor, bool& outEnableFallbackFormat)
{
    outEnableFallbackFormat = !(descriptor.flags & kRTFlagDisableCompatibleFormat);

    GraphicsCaps& caps = GetGraphicsCaps();
    GraphicsFormat compatibleFormat = outEnableFallbackFormat ? caps.GetCompatibleFormat(descriptor.colorFormat, kUsageRender) : descriptor.colorFormat;

    return compatibleFormat;
}

GraphicsFormat RenderTexture::GetColorFormat() const
{
    GraphicsCaps& caps = GetGraphicsCaps();

    GraphicsFormat format = m_Desc.colorFormat;

    bool enableFallbackFormat = false;
    const GraphicsFormat compatibleFormat = GetCompatibleFormat(m_Desc, enableFallbackFormat);
    if (compatibleFormat == kFormatNone)
    {
        ErrorStringObject(
                Format("Failed to create RenderTexture with %s (%d) format. The platform doesn't support that format, and it doesn't have a compatible format.",
                       GetFormatString(format).c_str(), format), this);
        return kFormatNone;
    }

    if (enableFallbackFormat)
    {
        if (GetActiveColorSpace() == kGammaColorSpace && IsSRGBFormat(format))
        {
            WarningStringObject(
                    Format("Requested RenderTexture with sRGB format. sRGB formats are not supported in gamma mode, fallback to a UNorm format. Use a UNorm format instead of sRGB to silence this warning."), this);
        }
        else if (compatibleFormat != format)
        {
            WarningStringObject(
                    Format("Requested RenderTexture format %s (%d) is not supported on this platform, using %s (%d) fallback format",
                           GetFormatString(format).c_str(), format, GetFormatString(compatibleFormat).c_str(), compatibleFormat), this);
        }
        return compatibleFormat;
    }
    else
    {
        if (GetActiveColorSpace() == kGammaColorSpace && IsSRGBFormat(format))
        {
            ErrorStringObject(
                    Format("Failed to create RenderTexture with an sRGB format. sRGB formats are not supported in gamma mode."), this);
        }
        else if (!caps.IsFormatSupported(format, kUsageRender))
        {
            if (compatibleFormat != kFormatNone)
            {
                ErrorStringObject(
                        Format("Failed to create RenderTexture with %s (%d) format. That format isn't supported. You can use %s (%d) format instead.",
                               GetFormatString(format).c_str(), format,
                               GetFormatString(compatibleFormat).c_str(), compatibleFormat), this);
            }
            else
            {
                ErrorStringObject(
                        Format("Failed to create RenderTexture with %s (%d) format. The format is not supported on this platform",
                               GetFormatString(format).c_str(), format), this);
            }
        }
        return format;
    }
}

bool RenderTexture::AdjustDescForGraphicsCaps(RenderTextureDesc& desc, GfxDevice& device, const GraphicsCaps& caps, RenderTexture* texture)
{
    // __FAKEABLE_STATIC_METHOD__(RenderTexture, AdjustDescForGraphicsCaps, (desc, device, caps, texture));

    if (desc.width <= 0 || desc.height <= 0)
    {
        ErrorStringObject("RenderTexture.Create failed: width & height must be larger than 0", texture);
        return false;
    }

    if (desc.dimension == kTexDimCUBE && (!IsDescPowerOfTwo(desc) || desc.width != desc.height))
    {
        ErrorStringObject("RenderTexture.Create failed: cube maps must be power of two and width must match height", texture);
        return false;
    }

    if (desc.width > caps.maxRenderTextureSize || desc.height > caps.maxRenderTextureSize)
    {
        if (IsDescPowerOfTwo(desc))
        {
            if (caps.maxRenderTextureSize < 4)
            {
                ErrorStringObject(Format("RenderTexture.Create failed: maxRenderTextureSize(%d) is too small", caps.maxRenderTextureSize), texture);
                return false;
            }

            // Decrease too large render textures while maintaining aspect ratio
            do
            {
                desc.width = std::max(desc.width / 2, 4);
                desc.height = std::max(desc.height / 2, 4);
            }
            while (desc.width > caps.maxRenderTextureSize || desc.height > caps.maxRenderTextureSize);
        }
        else
        {
            ErrorStringObject("RenderTexture.Create failed: requested size is too large.", texture);
            return false;
        }
    }

    const GraphicsFormat format = texture ? texture->GetColorFormat() : desc.colorFormat;


    if (!caps.IsFormatSupported(format, kUsageRender))
    {
        ErrorStringObject(Format("RenderTexture.Create failed: format unsupported - %s (%d).", GetFormatString(format).c_str(), desc.colorFormat), texture);
        return false;
    }
//
//    const GraphicsFormat stencilFormat = texture ? texture->GetStencilFormat() : desc.stencilFormat;
//    if (stencilFormat != kFormatNone && !caps.IsStencilFormatSupported(stencilFormat))
//    {
//        ErrorStringObject(Format("RenderTexture.Create failed: stencil texture format unsupported - %s (%d).", GetFormatString(stencilFormat).c_str(), desc.stencilFormat), texture);
//        return false;
//    }
//
//    if (HasFlag(desc.flags, kRTFlagRandomWrite) && !caps.IsFormatSupported(GetLinearFormat(format), kUsageLoadStore)) // Due to hardware design, UAV don't support sRGB formats so we create UNorm format views
//    {
//        ErrorStringObject(Format("RenderTexture.Create failed: format unsupported for random writes - %s (%d).", GetFormatString(format).c_str(), desc.colorFormat), texture);
//        return false;
//    }

    if (desc.dimension == kTexDimCUBE && (!caps.supportsDepthCubeTexture && IsDepthFormat(desc.colorFormat)))
    {
        ErrorStringObject("RenderTexture.Create failed: depth cubemap not supported.", texture);
        return false;
    }

    // Vulkan doesn't have render-to-3d but can write to it in compute shader, so allow that.
    if (desc.dimension == kTexDim3D && (!caps.has3DTexture || !(caps.hasRenderTo3D || HasFlag(desc.flags, kRTFlagRandomWrite))))
    {
        ErrorStringObject("RenderTexture.Create failed: volume texture not supported.", texture);
        return false;
    }

    if (desc.dimension == kTexDim2DArray && !HasFlag(caps.shaderCaps, kShaderRequire2DArray))
    {
        ErrorStringObject("RenderTexture.Create failed: 2DArray textures are not supported.", texture);
        return false;
    }

    if (desc.dimension == kTexDimCubeArray && !HasFlag(caps.shaderCaps, kShaderRequireCubeArray))
    {
        ErrorStringObject("RenderTexture.Create failed: CubeArray textures are not supported.", texture);
        return false;
    }

    // TODO: Add caps for placement resources, required if m_UseDynamicScale is set?
    if (HasFlag(desc.flags, kRTFlagRandomWrite) && HasFlag(desc.memoryless, kMemorylessColor))
    {
        WarningStringObject("RenderTexture.Create: enableRandomWrite and RenderTextureMemoryless.Color are not compatible, excluding RenderTextureMemoryles.Color.", texture);
        ClearFlags(desc.memoryless, kMemorylessColor);
    }
////
////    const bool isDepthOnlyRT = IsRenderTextureDepthOnly(desc.colorFormat);
//
//    if (desc.dimension == kTexDim3D && (isDepthOnlyRT || desc.depthFormat != kDepthFormatNone))
//    {
//        ErrorStringObject("RenderTexture.Create failed: 3D textures with depth are not supported.", texture);
//        return false;
//    }
//
//    if (isDepthOnlyRT && desc.depthFormat == kDepthFormatNone)
//    {
//        WarningStringObject("RenderTexture.Create: Depth|ShadowMap RenderTexture requested without a depth buffer. Changing to a 16 bit depth buffer.", texture);
//        desc.depthFormat = kDepthFormatMin16bits_NoStencil;
//    }

    if (desc.colorFormat == kFormatShadowAuto)
    {
        desc.shadowSamplingMode = kShadowSamplingCompareDepths;
    }
//
//    if (HasFlag(desc.flags, kRTFlagBindMS))
//    {
//        const MSAALevel msaaLevel = GetMSAALevel(desc.antiAliasing);
//        if (msaaLevel == kMSAALevelNone)
//        {
//            if (desc.antiAliasing > 1)
//                ErrorStringObject(Format("RenderTexture.Create with bindMS failed: antiAliasing %d unsupported.", desc.antiAliasing), texture);
//            return false;
//        }
//        if (!GetGraphicsCaps().IsFormatSupported(format, GetMSAALevelDesc(msaaLevel).usage))
//        {
//            ErrorStringObject(Format("RenderTexture.Create with bindMS failed: antiAliasing %d with unsupported format - %s.", desc.antiAliasing, GetFormatString(format).c_str()), texture);
//            return false;
//        }
//    }

    return true;
}

bool RenderTexture::Create(const RenderSurfaceHandle& colorHandle, const RenderSurfaceHandle& depthHandle, CreateFlags flags)
{
    // __FAKEABLE_METHOD_OVERLOADED__(RenderTexture, Create, (colorHandle, depthHandle, flags), bool(const RenderSurfaceHandle&, const RenderSurfaceHandle&, CreateFlags));

    // Create should be a no-op if RT is already created
    if (IsCreated())
        return true;

    DestroySurfaces();

    GfxDevice& device = GetGfxDevice();
    const GraphicsCaps& caps = GetGraphicsCaps();

    const bool IsSRGBReadWrite = IsSRGBReadWriteRequired(m_Desc);

    if (!HasFlag(flags, kCreateFlagAlreadyAdjustedDesc) &&
        !AdjustDescForGraphicsCaps(m_Desc, device, caps, this))
        return false;

    GraphicsFormat colorFormat;
    int samples = m_Desc.antiAliasing;
//    GetSupportedMSAASampleCountAndColorFormat(m_Desc, this, samples, colorFormat);
//
//    const bool isDepthOnlyRT = IsRenderTextureDepthOnly(colorFormat);
//
//    bool mipMap = HasFlag(m_Desc.flags, kRTFlagMipMap);
//    if (!GetIsPowerOfTwo() && caps.npot != kNPOTFull)
//        mipMap = false;
//
//    bool usesMSAA = samples > 1;
//    bool bindMS = HasFlag(m_Desc.flags, kRTFlagBindMS);
//    bool hasMultiSampleAutoResolve = false;
//    if (usesMSAA)
//    {
//        hasMultiSampleAutoResolve = (m_Desc.dimension == kTexDim2DArray) ? caps.hasMultiSampleTexture2DArrayAutoResolve : caps.hasMultiSampleAutoResolve;
//        hasMultiSampleAutoResolve |= bindMS;
//        mipMap = false;
//    }
//
//    // If we have native depth textures, we should use our regular textureID for the depth
//    // surface.
//    TextureID colorTID, resolvedColorTID, depthTID;
//    if (isDepthOnlyRT)
//    {
//        // we can use texture id ONLY if we have non-AA depth, unless it's a special render surface used in checkerboard rendering
//        if (!usesMSAA || bindMS || HasFlag(m_Desc.flags, kRTFlagSampleMSDepth))
//            depthTID = m_TexID;
//        m_SecondaryTexIDUsed = false;
//    }
//    else
//    {
//        // Use resolved surface as texture if MSAA enabled
//        if (usesMSAA && !hasMultiSampleAutoResolve)
//            resolvedColorTID = m_TexID;
//        else
//            colorTID = m_TexID;
//
//        bool useTextureForDepth = false;
//        if ((m_Desc.depthFormat != kDepthFormatNone) && caps.hasStencilInDepthTexture)
//        {
//            if (m_Desc.dimension == kTexDim2D)
//            {
//                useTextureForDepth = !usesMSAA;
//            }
//            else if (m_Desc.dimension == kTexDim2DArray)
//            {
//                useTextureForDepth = ((m_Desc.vrUsage != kVRTextureUsageNone) && hasMultiSampleAutoResolve) ? true : !usesMSAA;
//            }
//        }
//
//        if (useTextureForDepth || HasFlag(m_Desc.flags, kRTFlagsAssignTextureForDepth))
//        {
//            depthTID = m_SecondaryTexID;
//            m_SecondaryTexIDUsed = true;
//        }
//        else
//            m_SecondaryTexIDUsed = false;
//    }
//
//    SurfaceCreateFlags colorFlags = kSurfaceCreateFlagNone;
//    if (mipMap)
//        colorFlags |= kSurfaceCreateMipmap;
//    if (HasFlag(m_Desc.flags, kRTFlagAutoGenerateMips))
//        colorFlags |= kSurfaceCreateAutoGenMips;
//    if (IsSRGBReadWrite)
//        colorFlags |= kSurfaceCreateSRGB;
//    if (HasFlag(m_Desc.flags, kRTFlagRandomWrite))
//        colorFlags |= kSurfaceCreateRandomWrite;
//    if (!HasFlag(m_Desc.flags, kRTFlagAllowVerticalFlip))
//        colorFlags |= kSurfaceCreateNotFlipped;
//    if ((HasFlag(m_Desc.memoryless, kMemorylessColor) && !usesMSAA) || (HasFlag(m_Desc.memoryless, kMemorylessMSAA) && usesMSAA))
//        colorFlags |= kSurfaceCreateMemoryless;
//    if (GetUseDynamicScale())
//        colorFlags |= kSurfaceCreateDynamicScale;
//
//#if !PLATFORM_WEBGL
//    // WebGL always requires a color attachment.
//    if ((colorTID == TextureID() && !usesMSAA) || isDepthOnlyRT)
//        colorFlags |= kSurfaceCreateNeverUsed;                                                             // never sampled or resolved
//#endif
//    if (m_AllocateColorWithVRDevice)
//        colorFlags |= kSurfaceCreateWithVRDevice;
//    if (m_Desc.vrUsage != kVRTextureUsageNone)
//        colorFlags |= kSurfaceCreateVRUsage;
//    if (bindMS)
//        colorFlags |= kSurfaceCreateBindMS;
//
//    const GraphicsFormat colorSurfaceFormat = isDepthOnlyRT ? GetHelperColorFormatForDepthSurface() : colorFormat;
//
//    if (colorHandle.IsValid() && !HasFlag(colorHandle.object->flags, kSurfaceCreateNeverUsed))
//    {
//        m_ColorHandle = RenderSurfaceHandle(device.AliasRenderSurface(colorTID, colorHandle.object));
//    }
//    else
//    {
//        m_ColorHandle = device.CreateRenderColorSurface(colorTID,
//                                                        m_Desc.width, m_Desc.height, samples, GetVolumeDepthForUpload(this), m_Desc.mipCount, m_Desc.dimension, colorSurfaceFormat, colorFlags);
//    }
//
//    if (usesMSAA && !hasMultiSampleAutoResolve && !HasFlag(m_Desc.flags, kRTFlagNoResolvedColorSurface) && !bindMS)
//    {
//        m_ResolvedColorHandle = device.CreateResolveRenderColorSurface(resolvedColorTID, m_ColorHandle,
//                                                                       m_Desc.width, m_Desc.height, GetVolumeDepthForUpload(this), m_Desc.mipCount, m_Desc.dimension, colorSurfaceFormat, colorFlags);
//    }
//
//    if (usesMSAA && !caps.usesOpenGLTextureCoords)
//        SetUVYIsFromTopToBottom(true);
//
//    SurfaceCreateFlags depthFlags = kSurfaceCreateFlagNone;
//    if (m_Desc.colorFormat == kFormatShadowAuto)
//        depthFlags |= kSurfaceCreateShadowmap;
//    if (HasFlag(m_Desc.flags, kRTFlagSampleOnlyDepth))
//        depthFlags |= kSurfaceCreateSampleOnly;
//    if (!HasFlag(m_Desc.flags, kRTFlagAllowVerticalFlip))
//        depthFlags |= kSurfaceCreateNotFlipped;
//    if (HasFlag(m_Desc.memoryless, kMemorylessDepth))
//        depthFlags |= kSurfaceCreateMemoryless;
//    if (GetUseDynamicScale())
//        depthFlags |= kSurfaceCreateDynamicScale;
//    if (depthTID == TextureID() && m_Desc.depthFormat == kDepthFormatNone)
//        depthFlags |= kSurfaceCreateNeverUsed;                                                                    // we dont want depth on this one
//    if (m_Desc.depthFormat == kDepthFormatNone)
//        depthFlags |= kSurfaceCreateNoDepth;                                         // depth is not really used
//    if (m_Desc.vrUsage != kVRTextureUsageNone)
//        depthFlags |= kSurfaceCreateVRUsage;
//    if (m_Desc.stencilFormat != kFormatNone)
//        depthFlags |= kSurfaceCreateStencilTexture;
//
//
//    if (m_AllocateDepthWithVRDevice && m_Desc.vrUsage != kVRTextureUsageNone)
//        depthFlags |= kSurfaceCreateWithVRDevice;
//
//    // There is the case the color texture will have bindMS == false, but depth (secondary) texture will be sampled under MSAA
//    m_BindMultisampledOnSecondaryTex = false;
//    if (bindMS || (HasFlag(m_Desc.flags, kRTFlagsAssignTextureForDepth) && samples > 1))
//    {
//        depthFlags |= kSurfaceCreateBindMS;
//        m_BindMultisampledOnSecondaryTex = true;
//    }
//
//    if (depthHandle.IsValid() && !HasFlag(depthHandle.object->flags, kSurfaceCreateNeverUsed))
//    {
//        m_DepthHandle = RenderSurfaceHandle(device.AliasRenderSurface(depthTID, depthHandle.object));
//    }
//    else
//    {
//        m_DepthHandle = device.CreateRenderDepthSurface(depthTID,
//                                                        m_Desc.width, m_Desc.height, samples, GetVolumeDepthForUpload(this), m_Desc.dimension, m_Desc.depthFormat, depthFlags);
//    }
//
//    if (m_Desc.depthFormat == kDepthFormatMin24bits_Stencil && m_DepthHandle.IsValid() && m_Desc.stencilFormat != kFormatNone)
//    {
//        m_StencilTexID = GetUncheckedRealGfxDevice().CreateTextureID();
//
//        if (!device.CreateStencilTexture(m_StencilTexID, m_DepthHandle, m_Desc.stencilFormat))
//        {
//            GetUncheckedRealGfxDevice().FreeTextureID(m_StencilTexID);
//            m_StencilTexID = TextureID();
//        }
//    }
//
//    if (!m_ColorHandle.IsValid() || !m_DepthHandle.IsValid())
//    {
//        ErrorStringObject("RenderTexture.Create failed", this);
//        DestroySurfaces();
//        return false;
//    }
//
//    if (IsCreated())
//    {
//        device.GetFrameStats().ChangeRenderTextureBytes(GetRuntimeMemorySize());
//
//        Texture::TextureIDMapInsert(std::make_pair(m_TexID, this));
//        if (m_SecondaryTexIDUsed)
//            Texture::TextureIDMapInsert(std::make_pair(m_SecondaryTexID, this));
//
//        if (m_StencilTexID.IsValid())
//        {
//            Texture::TextureIDMapInsert(std::make_pair(m_StencilTexID, this));
//        }
//    }
//
//#if GFX_HAS_OBJECT_LABEL
//    device.SetRenderSurfaceName(m_ColorHandle.object, GetName());
//    device.SetRenderSurfaceName(m_DepthHandle.object, GetName());
//#endif
//
//    RenderTextureMap::Update(m_ColorHandle.object, this);
//    RenderTextureMap::Update(m_DepthHandle.object, this);
//
//    SetStoredColorSpaceNoDirtyNoApply(IsSRGBReadWrite ? kTexColorSpaceSRGB : kTexColorSpaceLinear);
//    OnUpdateExtents(mipMap);
//    ApplySettings();
//
//
//#if UNITY_EDITOR
//    if (!HasFlag(colorFlags, kSurfaceCreateNeverUsed))
//    {
//        ForceClear();  // Force-clear new RTs so that preview doesn't contain garbage. Case 498769
//
//        // Then ignore the warnings caused by the clear as it will not actually occur in standalone
//        device.IgnoreNextUnresolveOnRS(m_ColorHandle);
//        device.IgnoreNextUnresolveOnRS(m_DepthHandle);
//    }
//#endif

    return true;
}

RenderTexture* EnsureRenderTextureIsCreated(RenderTexture* tex)
{
    RenderTexture* ret = tex;
    if (ret)
        ret->Create();

    // check if we could create
    if (ret && !ret->IsCreated())
        ret = 0;

    return ret;
}
