#pragma once

#if PLATFORM_WIN && !PLATFORM_XBOXONE
#include <mfidl.h>
#include <stdint.h>
#include "PlatformDependent/Win/ComPtr.h"
#endif

#include "Runtime/Graphics/Texture2D.h"
#include "Runtime/Math/Matrix4x4.h"
#include "Runtime/Scripting/BindingsDefs.h"
#include "Runtime/Scripting/ScriptingExportUtility.h"
#include "CameraParameters.h"

namespace Unity
{
    class PhotoCaptureFrame
    {
#if PLATFORM_WIN && !PLATFORM_XBOXONE
    public:
        inline static PhotoCaptureFrame* Create(const Matrix4x4f& cameraToWorld, const Matrix4x4f& projection, IMFSample* sample, IMFMediaBuffer* mfBuffer, int width, int height, CapturePixelFormat format, bool hasLocationData)
        {
            return UNITY_NEW(PhotoCaptureFrame, kMemWebCam)(cameraToWorld, projection, sample, mfBuffer, width, height, format, hasLocationData);
        }

        inline void Dispose() { PhotoCaptureFrame* me = this; UNITY_DELETE(me, kMemWebCam); }

        inline Matrix4x4f GetCameraToWorld() const { return m_CameraToWorld; }
        inline Matrix4x4f GetProjection() const { return m_Projection; }
        inline bool GetHasLocationData() const { return m_HasLocationData; }

        inline int GetWidth() const { return m_Width; }
        inline int GetHeight() const { return m_Height; }
        inline CapturePixelFormat GetFrameFormat() const { return m_Format; }

        inline int GetDataLength() const { return m_Length; }
        inline void* GetUnsafePointerToBuffer() const { m_MFBuffer->AddRef(); return m_MFBuffer; }
        inline CapturePixelFormat GetCapturePixelFormat() const { return m_Format; }

        void CopyDataIntoBuffer(void* targetBuffer) const;
        int CopyRawImageDataIntoBuffer(dynamic_array<UInt8>& byteArray);
        void UploadImageDataToTexture(Texture2D* targetTexture);

    private:
        Matrix4x4f m_CameraToWorld;
        Matrix4x4f m_Projection;
        win::ComPtr<IMFSample> m_MFSample;
        win::ComPtr<IMFMediaBuffer> m_MFBuffer;
        uint32_t m_Width;
        uint32_t m_Height;
        CapturePixelFormat m_Format;
        int m_Length;
        bool m_HasLocationData;

        PhotoCaptureFrame(const Matrix4x4f& cameraToWorld, const Matrix4x4f& projection, IMFSample* mfSample, IMFMediaBuffer* mfBuffer, int width, int height, CapturePixelFormat format, bool hasLocationData);
#endif
    };
}

BIND_MANAGED_TYPE_NAME(::Unity::PhotoCaptureFrame, UnityEngine_Windows_WebCam_PhotoCaptureFrame);
