#pragma once

#include "Runtime/Utilities/Singleton.h"
#include "Runtime/Scripting/BindingsDefs.h"

class WebCam : public Singleton<WebCam>
{
public:
    enum WebCamMode
    {
        None = 0,
        PhotoMode = 1,
        VideoMode = 2
    };

    WebCam();

    WebCamMode GetWebCamMode();
    void SetWebCamMode(WebCamMode mode);

private:
    WebCamMode m_WebCamMode;
};

BIND_MANAGED_TYPE_NAME(WebCam::WebCamMode, UnityEngine_Windows_WebCam_WebCamMode);
BIND_MANAGED_TYPE_NAME(WebCam, UnityEngine_Windows_WebCam);
