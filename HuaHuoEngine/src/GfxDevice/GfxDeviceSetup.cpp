//
// Created by VincentZhang on 5/16/2022.
//

#include "GfxDeviceSetup.h"
#include "GfxDevice.h"
#include "opengles/GfxDeviceGLES.h"
#include "Memory/MemoryMacros.h"

GfxDevice* InitializeGfxDevice(){
    // A simpler version of creating the device ...
    GfxDevice* device = HUAHUO_NEW_AS_ROOT(GfxDeviceGLES, kMemGfxDevice, "Rendering", "GfxDeviceGLES") ();
    SetGfxDevice(device);
    return device;
}