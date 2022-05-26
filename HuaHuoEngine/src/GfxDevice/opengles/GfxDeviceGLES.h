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
protected:
    void UpdateSRGBWrite();

protected:
    bool                        m_sRGBWrite;
    GfxContextGLES*             m_Context;
    ApiGLES                     m_Api;
    DeviceStateGLES             m_State;
};


#endif //HUAHUOENGINE_GFXDEVICEGLES_H
