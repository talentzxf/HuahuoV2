//
// Created by VincentZhang on 5/13/2022.
//

#ifndef HUAHUOENGINE_GFXDEVICE_H
#define HUAHUOENGINE_GFXDEVICE_H


class GfxDevice {
public:
    explicit GfxDevice();
    virtual ~GfxDevice();

    GfxDevice(GfxDevice&) = delete;

    virtual void BeginFrame() = 0;
    virtual void EndFrame() = 0;
};


#endif //HUAHUOENGINE_GFXDEVICE_H
