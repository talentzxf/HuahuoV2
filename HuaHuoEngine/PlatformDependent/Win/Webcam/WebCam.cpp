#include "UnityPrefix.h"

#include "WebCam.h"
#include "Runtime/Utilities/RegisterRuntimeInitializeAndCleanup.h"

DEFINE_SINGLETON_INSTANCE(WebCam);

WebCam::WebCam() : m_WebCamMode(WebCam::None)
{
}

WebCam::WebCamMode WebCam::GetWebCamMode()
{
    return m_WebCamMode;
}

void WebCam::SetWebCamMode(WebCam::WebCamMode mode)
{
    m_WebCamMode = mode;
}

static void StaticInitializeWebCamInterface(void*)
{
    if (!WebCam::GetInstancePtr())
        WebCam::Create(kMemPermanent);
}

static void StaticCleanupWebCamInterface(void*)
{
    if (WebCam::GetInstancePtr())
        WebCam::Destroy();
}

static RegisterRuntimeInitializeAndCleanup s_WebCamCallbacks(StaticInitializeWebCamInterface, StaticCleanupWebCamInterface, 0);
