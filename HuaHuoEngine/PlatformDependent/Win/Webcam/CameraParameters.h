#pragma once


#include "Runtime/Scripting/BindingsDefs.h"
#include "Runtime/Utilities/ReflectableEnum.h"

namespace Unity
{
    REFLECTABLE_ENUM(CapturePixelFormat,
        BGRA32,
        NV12,
        JPEG,
        PNG
    );

    struct CameraParameters
    {
        float m_HologramOpacity;
        float m_FrameRate;
        int m_CameraResolutionWidth;
        int m_CameraResolutionHeight;
        int m_PixelFormat;

        CameraParameters() :
            m_HologramOpacity(false),
            m_FrameRate(30.0f),
            m_CameraResolutionWidth(0),
            m_CameraResolutionHeight(0),
            m_PixelFormat(CapturePixelFormat::BGRA32)
        {
        }
    };
}

BIND_MANAGED_TYPE_NAME(::Unity::CapturePixelFormat::ActualEnumType, UnityEngine_Windows_WebCam_CapturePixelFormat);
BIND_MANAGED_TYPE_NAME(::Unity::CameraParameters, UnityEngine_Windows_WebCam_CameraParameters);
