//
// Created by VincentZhang on 5/16/2022.
//

#ifndef HUAHUOENGINE_GFXDEVICEGLES_H
#define HUAHUOENGINE_GFXDEVICEGLES_H
#include "GfxDevice/GfxDevice.h"
#include "GfxContextGLES.h"
#include "ApiGLES.h"
#include "DeviceStateGLES.h"

class GfxDeviceGLES : public GfxThreadableDevice{
public:
    GfxDeviceGLES(MemLabelRef label);
    virtual         ~GfxDeviceGLES();

    virtual void    BeginFrame();
    virtual void    EndFrame();

    virtual void    SetSRGBWrite(const bool);
    virtual bool    GetSRGBWrite();

    // It needs to be called before any use of an GfxDeviceGLES instance
    // It should be in GfxDeviceGLES constructor but we create GfxDeviceGLES with UNITY_NEW_AS_ROOT which doesn't allow arguments
    virtual bool    Init(GfxDeviceLevelGL deviceLevel);
    virtual void    OnDeviceCreated(bool callingFromRenderThread);

    virtual void    SetViewport(const RectInt& rect);
    virtual RectInt GetViewport() const;

    virtual void    SetActiveContext(void* context);

    // Machinery for generating mipmaps for rendersurfaces. Previously we used to call glGenMipmaps in GfxFramebuffer::Prepare() for the previous
    // rendersurfaces as needed, but that doesn't work when we switch GL context (and therefore do ApiGLES::Invalidate) after rendering to the
    // texture but before rendering anything else to another rendertarget (so Prepare never gets called for that one).
    // Therefore we keep a list of surfaces that need mipmap generation, and call ProcessPendingMipGens() at both Prepare and right after
    // switching GL context. We'll also have to handle the case where the render texture gets deleted while the mipmaps are still in the pending list (that's what CancelPendingMipGen is for).
    void ProcessPendingMipGens(); // Generate mipmaps for all pending render textures
    virtual size_t  RenderSurfaceStructMemorySize(bool colorSurface);

    virtual void    DestroyRenderSurfacePlatform(RenderSurfaceBase* rs);

    virtual int GetActiveRenderTargetCount() const;

    virtual RenderSurfaceHandle GetActiveRenderColorSurface(int index) const;
    virtual RenderSurfaceHandle GetActiveRenderDepthSurface() const;

    virtual void                ResolveColorSurface(RenderSurfaceHandle srcHandle, RenderSurfaceHandle dstHandle);
    void MemoryBarrierImmediate(BarrierTime previousWriteTime, gl::MemoryBarrierType type);
protected:
    // Platform-dependent part of SetRenderTargets
    virtual void SetRenderTargetsImpl(const GfxRenderTargetSetup& rt);
    void UpdateSRGBWrite();
    void SetViewportInternal(const RectInt& rect);

protected:
    bool                        m_sRGBWrite;
    GfxContextGLES*             m_Context;
    ApiGLES                     m_Api;
    DeviceStateGLES             m_State;

    std::vector<RenderSurfaceBase *> m_PendingMipGens; // List of rendersurfaces that need automatic mipmap generation.
};


#endif //HUAHUOENGINE_GFXDEVICEGLES_H
