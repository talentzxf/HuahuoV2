//
// Created by VincentZhang on 5/24/2022.
//

#ifndef HUAHUOENGINE_GFXDEVICEOBJECTS_H
#define HUAHUOENGINE_GFXDEVICEOBJECTS_H

// --------------------------------------------------------------------------

#include "Graphics/RenderSurface.h"

// A type safe opaque pointer to something.
// ID type is used just so that you can have handles to different types, all type safe.
template<typename ID, typename ObjectPtrType = void*>
struct ObjectHandle
{
    explicit ObjectHandle(ObjectPtrType obj = 0) : object(obj) {}
    bool IsValid() const { return object != 0; }
    void Reset() { object = 0; }
    bool operator==(const ObjectHandle<ID, ObjectPtrType>& o) const { return object == o.object; }
    bool operator!=(const ObjectHandle<ID, ObjectPtrType>& o) const { return object != o.object; }

    ObjectPtrType object;
};

// TODO: should we pack better?
struct GfxRenderTarget
{
    RenderSurfaceBase*  color[kMaxSupportedRenderTargets];
    RenderSurfaceBase*  depth;

    unsigned            colorCount;
    unsigned            mipLevel;
    CubemapFace         cubemapFace;
    int                 depthSlice;
};

struct GfxRenderTargetSetup : GfxRenderTarget
{
    UInt32  flags;                                          // enum GfxDevice::RenderTargetFlags

    UInt8   colorLoadAction[kMaxSupportedRenderTargets];    // GfxRTLoadAction
    UInt8   colorStoreAction[kMaxSupportedRenderTargets];   // GfxRTStoreAction
    UInt8   depthLoadAction;                                // GfxRTLoadAction
    UInt8   depthStoreAction;                               // GfxRTStoreAction
};


// --------------------------------------------------------------------------

struct RenderSurfaceBase;
struct RenderSurface_Tag;
typedef ObjectHandle<RenderSurface_Tag, RenderSurfaceBase*> RenderSurfaceHandle;

#endif //HUAHUOENGINE_GFXDEVICEOBJECTS_H
