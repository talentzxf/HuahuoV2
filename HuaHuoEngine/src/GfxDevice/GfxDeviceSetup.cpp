//
// Created by VincentZhang on 5/16/2022.
//

#include "GfxDeviceSetup.h"
#include "GfxDevice.h"
#include "opengles/GfxDeviceGLES.h"
#include "Memory/MemoryMacros.h"

GfxDevice* InitializeGfxDevice(){
    GfxDevice* device = NEW(GfxDeviceGLES)();
    SetGfxDevice(device);
}